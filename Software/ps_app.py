#!/usr/bin/env python3

import time
import serial
import serial.tools.list_ports
import threading
import time
import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk

class VoltageCurrent_thread(threading.Thread):
    	
	def __init__(self,ser,voltage_label,current_label):
		threading.Thread.__init__(self)
		self.t = threading.currentThread()
		self.ser = ser
		self.voltage_label = voltage_label
		self.current_label = current_label
		self.run()

	def run(self):
		try:
			while getattr(self.t, "do_run", True) and self.ser != None:
				voltage = self.ser.send_serial("R0")
				current_voltage = self.ser.send_serial("R1")
				if voltage != None and current_voltage != None:
					voltage = ((10000.0 + 1600.0)/ 1600.0)*float(voltage) #Vs = ((R1+R2)/R2)*Vout
					self.voltage_label.set_label("{0:.6f} V".format(voltage))
					current_voltage = float(current_voltage) / 4.9 #We divide with 4.9 because thats the factor the amplifier amplifies the voltage
					current = current_voltage / 1000.0 		#We divide the real voltage drop we measure from amplifier with the resistor
					current = current*5000.0				#The real current at load is the current we find above in the resistor*5000 as the datasheet of LT3081 says
					self.current_label.set_label("{0:.6f} A".format(current))
				time.sleep(1)
		except:
			print("Error: Cannot receive proper format values")

class SerialPort:

	def find_ports(self):
		results = serial.tools.list_ports.comports()
		if len(results) == 0:
			return []
		ports = []
		if type(results[0]) == tuple:
			for result in results:
				ports.append(result[0])
		else:
			for result in results:
				ports.append(result.device)
		return ports

	def open_serial(self,s_port):
		try:
			self.ser = serial.Serial( port=s_port, baudrate=19200, bytesize=serial.EIGHTBITS, 	parity=serial.PARITY_NONE,stopbits=serial.STOPBITS_ONE, timeout=1, xonxoff=False, rtscts=False, write_timeout=None, dsrdtr=False, inter_byte_timeout=None, exclusive=None)
			self.ser.isOpen()
			self.serialOpen = True
		except:
			print("Cannot connect with the device.")

	def close_serial(self):
		try:
			self.serialOpen = False
			self.ser.close()
		except:
			pass

	def send_serial(self,command):
		try:
			self.ser.write(str.encode(command + "\r\n"))
			if command == "R2" :
				respond = self.ser.readline().decode("utf-8") + self.ser.readline().decode("utf-8") + self.ser.readline().decode("utf-8")
			else:
				respond = self.ser.readline().decode("utf-8")
			return respond
		except:
			print("Cannot connect with the device.")
			self.info_label.set_label("Cannot connect!\nTry to change the port\nor replug the device.")
			return "!"

	def __init__(self,info_label):
		self.serialOpen = False
		self.ser = None
		self.send_status = False
		self.info_label = info_label


class UIWindow:

	def on_window_destroy(self, object, data=None):
		if self.thread == None :
			pass
		elif self.thread.isAlive() :
			self.thread.do_run = False
			self.thread.join()
		self.serial.close_serial()
		Gtk.main_quit()

	def start_thread(self):
		self.thread = threading.Thread(target=VoltageCurrent_thread,args=(self.serial,self.voltage_label,self.current_label))
		self.thread.start()

	def destroy_thread(self):
		if self.thread == None:
			pass
		elif self.thread.isAlive() :
			self.thread.do_run = False
			self.thread.join()

	def combo_box_change(self,widget,data=None):
		try:
			if widget.get_active_text() != "" and self.refresh == False:
				print("Serial open:"+widget.get_active_text())
				self.destroy_thread()
				self.serial.close_serial()
				self.serial.open_serial(widget.get_active_text())
				ver = self.serial.send_serial("R2")
				if ver[0:7] == "Version" :
					self.info_label.set_label(ver)
					self.connection_status = True
					self.start_thread()
				else:
					self.info_label.set_label("Cannot connect!\nTry to change the port\nor replug the device.")
					self.serial.close_serial()
					self.connection_status = False
					self.voltage_label.set_label("")
					self.current_label.set_label("")
					self.message_label.set_label("")
			else:
				self.refresh = False
		except:
			print("Cannot connect with the device")
			self.info_label.set_label("Cannot connect!\nTry to change the port\nor replug the device.")

	def refresh_ports(self,widget,data=None):
		self.ports = self.serial.find_ports()
		self.refresh = True
		self.combo_box.remove_all()
		for port in self.ports:
			self.combo_box.append_text(port)

	def led_on(self,widget,data=None):
		self.destroy_thread()
		respond = self.serial.send_serial("W00001")
		self.start_thread()
		if respond != '!' :
			self.message_label.set_label("LED is on.")

	def led_off(self,widget,data=None):
		self.destroy_thread()
		respond = self.serial.send_serial("W00000")
		self.start_thread()
		if respond != '!' :
			self.message_label.set_label("LED is off.")

	def voltage_set(self,widget,data=None):
		try:
			voltage = float(self.voltage_set_val.get_text())
			if voltage >= 0.0 and voltage <= 5.0 :
				r = voltage / 0.00005 #According to LT3081 datasheet with this method we calculate the resistor
				ad5272_step = 100000.0 / 1023.0 #The AD5272 step
				r = int(round(r / ad5272_step))
				self.destroy_thread()
				if r < 10 :
					self.serial.send_serial("W1000"+str(r))
					print("Serial send:W1000"+str(r))
				elif r < 100:
					self.serial.send_serial("W100"+str(r))
					print("Serial send:W100"+str(r))
				elif r < 1000:
					self.serial.send_serial("W10"+str(r))
					print("Serial send:W10"+str(r))
				else:
					self.serial.send_serial("W1"+str(r))	
					print("Serial send:W1"+str(r))
				self.message_label.set_text("Voltage Set!")
				self.start_thread()
			else:
				self.message_label.set_text("Voltage range 0V-5V!")	
		except:
			self.message_label.set_text("Use only numbers!")

	def current_set(self,widget,data=None):	
		try:
			Ilimit = float(self.current_set_val.get_text())
			if Ilimit >= 0.0 and Ilimit <= 1.5 :
				Irm = Ilimit / 5000.0  #Ilimit is the current we set as limit,Irm is the current the amplifier read,Irm is 1/5000 of Iload(Ilimit)
				Vrm = 1000*Irm*4.9 #Vrm is the differential voltage that the mcu reads,the real is 1000*Irm,but is amplified by 4.9
				self.destroy_thread()
				self.serial.send_serial("C"+"{0:.6f}".format(Vrm))
				print("Serial send:C"+"{0:.6f}".format(Vrm))
				self.message_label.set_text("Current Set!")
				self.start_thread()
			else:
				self.message_label.set_text("Current range 0A-1.5A!")
		except:
			self.message_label.set_text("Use only numbers!")

	def __init__(self):
		self.refresh = False
		self.connection_status = False
		self.thread = None
		self.gladefile = "ui.glade" 
		self.builder = Gtk.Builder()
		self.builder.add_from_file(self.gladefile)
		self.builder.connect_signals(self)
		self.window = self.builder.get_object("GtkWindow")
		self.combo_box = self.builder.get_object("combo_box")
		self.info_label = self.builder.get_object("info_label")
		self.voltage_label = self.builder.get_object("voltage_value_label")
		self.current_label = self.builder.get_object("current_value_label")
		self.voltage_set_val = self.builder.get_object("voltage_input")
		self.current_set_val = self.builder.get_object("current_input")
		self.message_label = self.builder.get_object("message_label")
		self.serial = SerialPort(self.info_label)
		self.ports = self.serial.find_ports()
		for port in self.ports:
			self.combo_box.append_text(port)

		self.window.show()


#MAIN
if __name__ == "__main__":
	try:
		ui_window = UIWindow()
		Gtk.main()
	except KeyboardInterrupt:
		pass
