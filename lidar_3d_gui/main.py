from typing import Union
from PyQt5 import QtWidgets,QtCore, QtGui, uic
from PyQt5.QtWidgets import QFileDialog
import sys, serial, platform
import serial.tools.list_ports
import LiDAR
from threading import Thread
from time import sleep

class Ui(QtWidgets.QMainWindow):
	def __init__(self):
		super().__init__() #notfalls super(Ui, self)
		uic.loadUi('GUI.ui', self)

		# additional classes and settings
		self.lidar = None

		# LABELS
		self.title =			self.findChild(QtWidgets.QLabel, 'title')
		# Spin Boxes
		self.fov =			self.findChild(QtWidgets.QSpinBox, 'fov')
		self.fov_high =			self.findChild(QtWidgets.QSpinBox, 'fov_high')
		self.fov_low =			self.findChild(QtWidgets.QSpinBox, 'fov_low')
		self.microstep_div =	self.findChild(QtWidgets.QSpinBox, 'microstep_div')
		self.stepper_angle =		self.findChild(QtWidgets.QDoubleSpinBox, 'stepper_angle')
		# Progress Bars
		self.scan_progress =		self.findChild(QtWidgets.QProgressBar, 'scan_progress')
		# Buttons
		self.file_select =		self.findChild(QtWidgets.QPushButton, 'file_select')
		self.scan_start =		self.findChild(QtWidgets.QPushButton, 'scan_start')
		self.scan_scale =		self.findChild(QtWidgets.QPushButton, 'scan_scale')
		self.scan_park =		self.findChild(QtWidgets.QPushButton, 'scan_park')
		# Actions
		self.quit =			self.findChild(QtWidgets.QAction, 'quit')
		self.about =			self.findChild(QtWidgets.QAction, 'about')
		self.ports = 			self.findChild(QtWidgets.QComboBox, 'ports')
		self.scan_mode =		self.findChild(QtWidgets.QComboBox, 'scan_mode')
		self.connect = 			self.findChild(QtWidgets.QPushButton, 'connect')

		# connect functions
		self.quit.triggered.connect(lambda: exit())
		self.scan_progress.reset()
		self.connect.clicked.connect(self.func_connect)
		self.file_select.clicked.connect(self.func_visualize)
		self.scan_start.clicked.connect(self.func_start_scan)
		self.scan_scale.clicked.connect(self.func_go_scale)
		self.scan_park.clicked.connect(self.func_park_position)
		self.about.triggered.connect(self.func_about)

		# set properties and ranges
		for port in serial.tools.list_ports.comports():
			self.ports.addItem(port.name)
		self.fov.setRange(0,180)
		self.fov.setValue(180)
		self.fov_high.setRange(0,50)
		self.fov_high.setValue(50)
		self.fov_low.setRange(0,30)
		self.fov_low.setValue(30)

		self.show()
	
	def openFileNameDialog(self) -> Union[str, None]:
		options = QFileDialog.Options()
		options |= QFileDialog.DontUseNativeDialog
		fileName, _ = QFileDialog.getOpenFileName(self,"QFileDialog.getOpenFileName()", "","FSCAN Files (*.fscan);;Python Files (*)", options=options)
		if fileName:
			return fileName
		else:
			return None
    
	def openFileNamesDialog(self) -> Union[str, None]:
		options = QFileDialog.Options()
		options |= QFileDialog.DontUseNativeDialog
		files, _ = QFileDialog.getOpenFileNames(self,"QFileDialog.getOpenFileNames()", "","FSCAN Files (*.fscan);;All Files (*)", options=options)
		if files:
			return files
		else:
			return None
    
	def saveFileDialog(self) -> Union[str, None]:
		options = QFileDialog.Options()
		options |= QFileDialog.DontUseNativeDialog
		fileName, _ = QFileDialog.getSaveFileName(self,"QFileDialog.getSaveFileName()",".fscan","FSCAN Files (*.fscan);;All Files (*)", options=options)
		if fileName:
			return fileName
		else:
			return None
	
	def message(self,text:str) -> None:
		msg = QtWidgets.QMessageBox()
		msg.setWindowTitle("INFO")
		msg.setText(text)
		msg.exec_()

	def func_connect(self) -> None:
		if(platform.system().lower() == "linux"):
			port = "/dev/" + self.ports.currentText()
		else:
			port = self.ports.currentText()

		try:
			self.lidar = LiDAR.LiDAR(port)
			self.scan_start.setEnabled(True)
			self.scan_scale.setEnabled(True)
			self.scan_park.setEnabled(True)
			self.connect.setEnabled(False)
			self.message("CONNECTED!")
		except Exception as e:
			self.message(f"ERROR: could not connect to serial port!\n{e}")
	
	def func_visualize(self) -> None:
		filename = self.openFileNameDialog()
		success = LiDAR.visualize(filename)
		if( not success ):
			self.message("An ERROR occured!")
	
	def func_start_scan(self) -> None:
		self.scan_progress.setValue(0)
		fov = self.fov.value()
		hfov = self.fov_high.value()
		dfov = self.fov_low.value()
		file_location = self.saveFileDialog()
		if( self.scan_mode.currentText() == "quad" ):
			mode = True
		else:
			mode = False

		def scan_thread():
			self.lidar.scan(float(self.stepper_angle.value()) / float(self.microstep_div.value()), fov, hfov, dfov, file_location, mode)

		if( file_location != None ):
			scan_thread = Thread(target=scan_thread)
			self.scan_progress.setMaximum(hfov+dfov)
			scan_thread.start()
			while( scan_thread.is_alive() ):
				self.scan_progress.setValue(round(self.lidar.current_angle))
				sleep(0.2)
			self.message("DONE SCAN")
		else:
			self.message("CANCELED!")



	def func_go_scale(self) -> None:
		fov = self.fov.value()
		hfov = self.fov_high.value()
		dfov = self.fov_low.value()
		self.lidar.go_scale(fov, hfov, dfov)


	def func_park_position(self) -> None:
		self.lidar.park()
		self.message("Parked. App will now exit")
		exit()
	
	def func_about(self) -> None:
		self.message("LiDAR lite Scanner by VoidLight\nmade with Qt")

if __name__ == "__main__":
	app = QtWidgets.QApplication(sys.argv)
	window = Ui()
	sys.exit(app.exec_())