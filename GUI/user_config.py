"""
@brief PyQt5 GUI Application for Configuring User Settings

This module provides a PyQt5-based graphical interface for configuring user settings. 
The application allows users to input configuration details, including Logger ID, Cell ID, 
Upload Method (WiFi or LoRa), Upload Interval, Enabled Sensors, and Calibration parameters 
for voltage and current (V/I Slope and Offset). 

Key features:
- **Save and Load**: Users can save configurations to a file or load previous configurations for easy reuse.
- **Real-time Configuration**: By pressing the "Send Configuration" button, the settings are serialized with Protobuf 
  and transmitted over UART to the STM32 for direct application.
  
@file user_config.py
@author Ahmed Hassan Falah
@date 2024-10-10
"""
from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import QInputDialog, QMessageBox
import json
import os
import sys
import serial
import serial.tools.list_ports
import re  # For validating URL input
from soil_power_sensor_protobuf.proto import encode_user_configuration, decode_user_configuration


class Ui_MainWindow(object):

    def setupUi(self, MainWindow):
        """
        @brief Sets up the user interface components.

        Initializes the main window and creates the layout for user configurations.
        """
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(600, 500)
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")

        self.layout = QtWidgets.QVBoxLayout(self.centralwidget)

        # Group boxes
        self.setupGroupBoxes()
        # Save and Load Buttons
        self.setupSaveAndLoadButtons()

        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 600, 26))
        self.menubar.setObjectName("menubar")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "User Configuration"))
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

        # Center the window initially
        screen = QtWidgets.QDesktopWidget().screenGeometry()
        screen_width = screen.width()
        screen_height = screen.height()
        window_width = MainWindow.width()
        window_height = MainWindow.height()
        x = (screen_width - window_width) // 2
        y = (screen_height - window_height) // 3
        MainWindow.setGeometry(x, y, window_width, window_height)

    def setupGroupBoxes(self):
        """
        @brief Sets up the group boxes for different configuration sections.
        """
        font = QtGui.QFont()
        font.setPointSize(10)
        font.setBold(True)

        # Upload Settings group
        self.uploadSettingsGroupBox = QtWidgets.QGroupBox("Upload Settings", self.centralwidget)
        self.uploadSettingsLayout = QtWidgets.QGridLayout(self.uploadSettingsGroupBox)

        self.Logger_ID = self.createLabel("Logger ID", font)
        self.lineEdit_Logger_ID = self.createLineEdit("Enter Logger ID (positive integer)")
        self.uploadSettingsLayout.addWidget(self.Logger_ID, 0, 0)
        self.uploadSettingsLayout.addWidget(self.lineEdit_Logger_ID, 0, 1)

        self.Cell_ID = self.createLabel("Cell ID", font)
        self.lineEdit_Cell_ID = self.createLineEdit("Enter Cell ID (positive integer)")
        self.uploadSettingsLayout.addWidget(self.Cell_ID, 1, 0)
        self.uploadSettingsLayout.addWidget(self.lineEdit_Cell_ID, 1, 1)

        self.Upload_Method = self.createLabel("Upload Method", font)
        self.comboBox_Upload_Method = QtWidgets.QComboBox(self.centralwidget)
        self.comboBox_Upload_Method.addItems(["WiFi", "LoRa"])
        self.comboBox_Upload_Method.setCurrentIndex(1)  # Set default to LoRa
        self.comboBox_Upload_Method.currentIndexChanged.connect(self.toggleUploadMethod)
        self.uploadSettingsLayout.addWidget(self.Upload_Method, 2, 0)
        self.uploadSettingsLayout.addWidget(self.comboBox_Upload_Method, 2, 1)

        self.Upload_Interval = self.createLabel("Upload Interval", font)
        self.layout_Upload_Interval = QtWidgets.QHBoxLayout()

        # Upload_Interval: Input fields for Days, Hours, Minutes, Seconds
        self.lineEdit_Days = self.createLineEdit("0")
        self.lineEdit_Days.setFixedWidth(40)
        self.layout_Upload_Interval.addWidget(self.lineEdit_Days)
        self.label_Days = QtWidgets.QLabel("days")    
        self.layout_Upload_Interval.addWidget(self.label_Days)

        self.lineEdit_Hours = self.createLineEdit("0")
        self.lineEdit_Hours.setFixedWidth(40)
        self.layout_Upload_Interval.addWidget(self.lineEdit_Hours)
        self.label_Hours = QtWidgets.QLabel("hours")
        self.layout_Upload_Interval.addWidget(self.label_Hours)

        self.lineEdit_Minutes = self.createLineEdit("0")
        self.lineEdit_Minutes.setFixedWidth(40)
        self.layout_Upload_Interval.addWidget(self.lineEdit_Minutes)
        self.label_Minutes = QtWidgets.QLabel("minutes")
        self.layout_Upload_Interval.addWidget(self.label_Minutes)

        self.lineEdit_Seconds = self.createLineEdit("0")
        self.lineEdit_Seconds.setFixedWidth(40)
        self.layout_Upload_Interval.addWidget(self.lineEdit_Seconds)
        self.label_Seconds = QtWidgets.QLabel("seconds")
        self.layout_Upload_Interval.addWidget(self.label_Seconds)

        self.uploadSettingsLayout.addWidget(self.Upload_Interval, 3, 0)
        self.uploadSettingsLayout.addLayout(self.layout_Upload_Interval, 3, 1)

        self.layout.addWidget(self.uploadSettingsGroupBox)

        # Measurement Settings group
        self.measurementSettingsGroupBox = QtWidgets.QGroupBox("Measurement Settings", self.centralwidget)
        self.measurementSettingsLayout = QtWidgets.QGridLayout(self.measurementSettingsGroupBox)

        self.Enabled_Sensors = self.createLabel("Enabled Sensors", font)
        self.checkBox_Voltage = QtWidgets.QCheckBox("Voltage")
        self.checkBox_Current = QtWidgets.QCheckBox("Current")
        self.checkBox_Teros12 = QtWidgets.QCheckBox("Teros12")
        self.checkBox_Teros21 = QtWidgets.QCheckBox("Teros21")
        self.checkBox_BME280 = QtWidgets.QCheckBox("BME280")

        self.measurementSettingsLayout.addWidget(self.Enabled_Sensors, 0, 0)
        self.measurementSettingsLayout.addWidget(self.checkBox_Voltage, 0, 1)
        self.measurementSettingsLayout.addWidget(self.checkBox_Current, 1, 1)
        self.measurementSettingsLayout.addWidget(self.checkBox_Teros12, 2, 1)
        self.measurementSettingsLayout.addWidget(self.checkBox_Teros21, 3, 1)
        self.measurementSettingsLayout.addWidget(self.checkBox_BME280, 4, 1)

        self.Calibration_V_Slope = self.createLabel("Calibration V Slope", font)
        self.lineEdit_V_Slope = self.createLineEdit("Enter Voltage Slope (floating-point)")
        self.measurementSettingsLayout.addWidget(self.Calibration_V_Slope, 5, 0)
        self.measurementSettingsLayout.addWidget(self.lineEdit_V_Slope, 5, 1)

        self.Calibration_V_Offset = self.createLabel("Calibration V Offset", font)
        self.lineEdit_V_Offset = self.createLineEdit("Enter Voltage Offset (floating-point)")
        self.measurementSettingsLayout.addWidget(self.Calibration_V_Offset, 6, 0)
        self.measurementSettingsLayout.addWidget(self.lineEdit_V_Offset, 6, 1)

        self.Calibration_I_Slope = self.createLabel("Calibration I Slope", font)
        self.lineEdit_I_Slope = self.createLineEdit("Enter Current Slope (floating-point)")
        self.measurementSettingsLayout.addWidget(self.Calibration_I_Slope, 7, 0)
        self.measurementSettingsLayout.addWidget(self.lineEdit_I_Slope, 7, 1)

        self.Calibration_I_Offset = self.createLabel("Calibration I Offset", font)
        self.lineEdit_I_Offset = self.createLineEdit("Enter Current Offset (floating-point)")
        self.measurementSettingsLayout.addWidget(self.Calibration_I_Offset, 8, 0)
        self.measurementSettingsLayout.addWidget(self.lineEdit_I_Offset, 8, 1)

        self.layout.addWidget(self.measurementSettingsGroupBox)

        # WiFi Settings group (initially hidden)
        self.wifiGroupBox = QtWidgets.QGroupBox("WiFi Configuration", self.centralwidget)
        self.wifiLayout = QtWidgets.QGridLayout(self.wifiGroupBox)

        self.WiFi_SSID = self.createLabel("WiFi SSID", font)
        self.lineEdit_WiFi_SSID = self.createLineEdit("Enter WiFi SSID")
        self.wifiLayout.addWidget(self.WiFi_SSID, 0, 0)
        self.wifiLayout.addWidget(self.lineEdit_WiFi_SSID, 0, 1)

        self.WiFi_Password = self.createLabel("WiFi Password", font)
        self.lineEdit_WiFi_Password = self.createLineEdit("Enter WiFi Password")
        self.lineEdit_WiFi_Password.setEchoMode(QtWidgets.QLineEdit.Password)
        self.wifiLayout.addWidget(self.WiFi_Password, 1, 0)
        self.wifiLayout.addWidget(self.lineEdit_WiFi_Password, 1, 1)

        self.API_Endpoint_URL = self.createLabel("API Endpoint URL", font)
        self.lineEdit_API_Endpoint_URL = self.createLineEdit("Enter API Endpoint URL (start with http:// or https://)")
        self.wifiLayout.addWidget(self.API_Endpoint_URL, 2, 0)
        self.wifiLayout.addWidget(self.lineEdit_API_Endpoint_URL, 2, 1)

        self.API_Port = self.createLabel("API Port", font)
        self.lineEdit_API_Port = self.createLineEdit("Enter API Port (integer)")
        self.wifiLayout.addWidget(self.API_Port, 3, 0)
        self.wifiLayout.addWidget(self.lineEdit_API_Port, 3, 1)
        
        # Ensure consistent size for wifiGroupBox
        self.wifiGroupBox.setFixedHeight(self.wifiGroupBox.minimumSizeHint().height())
        self.layout.addWidget(self.wifiGroupBox)

        # Show or hide WiFi settings based on upload method
        self.toggleUploadMethod()

    def toggleUploadMethod(self):
        """
        @brief Shows or hides WiFi settings based on the upload method selected.
        """
        if self.comboBox_Upload_Method.currentText() == "WiFi":
            self.showWiFiSettings()
        else:
            self.hideWiFiSettings()

    def showWiFiSettings(self):
        """
        @brief Displays the WiFi configuration settings.
        """
        self.lineEdit_WiFi_SSID.setEnabled(True)
        self.lineEdit_WiFi_Password.setEnabled(True)
        self.lineEdit_API_Endpoint_URL.setEnabled(True)
        self.lineEdit_API_Port.setEnabled(True)
        self.lineEdit_WiFi_SSID.show()
        self.lineEdit_WiFi_Password.show()
        self.lineEdit_API_Endpoint_URL.show()
        self.lineEdit_API_Port.show()
        # set default values for API URL & PORT
        self.lineEdit_API_Endpoint_URL.setText("https://dirtviz.jlab.ucsc.edu/api/")
        self.lineEdit_API_Port.setText("443")    

    def hideWiFiSettings(self):
        """
        @brief Hides the WiFi configuration settings.
        """
        self.lineEdit_WiFi_SSID.setEnabled(False)
        self.lineEdit_WiFi_Password.setEnabled(False)
        self.lineEdit_API_Endpoint_URL.setEnabled(False)
        self.lineEdit_API_Port.setEnabled(False)
        self.lineEdit_WiFi_SSID.hide()
        self.lineEdit_WiFi_Password.hide()
        self.lineEdit_API_Endpoint_URL.hide()
        self.lineEdit_API_Port.hide()
        
    def createLabel(self, text, font):
        """
        @brief Creates a label with the specified text and font.

        @param text Text for the label.
        @param font Font settings for the label.
        @return QLabel instance
        """
        label = QtWidgets.QLabel(text)
        label.setFont(font)
        return label

    def createLineEdit(self, placeholder):
        """
        @brief Creates a QLineEdit with a placeholder.

        @param placeholder Placeholder text for the QLineEdit.
        @return QLineEdit instance
        """
        lineEdit = QtWidgets.QLineEdit(self.centralwidget)
        lineEdit.setPlaceholderText(placeholder)
        return lineEdit

    def setupSaveAndLoadButtons(self):
        """
        @brief Creates and configures the save and load buttons.
        """
        # Create a horizontal layout for the buttons
        button_layout = QtWidgets.QHBoxLayout()

        # Load button (aligned left)
        self.loadButton = QtWidgets.QPushButton("Load", self.centralwidget)
        self.loadButton.setFixedSize(100, 30)
        self.loadButton.clicked.connect(self.loadConfiguration)
        button_layout.addWidget(self.loadButton)

        # Save button (in the middle)
        self.saveButton = QtWidgets.QPushButton("Save", self.centralwidget)
        self.saveButton.setFixedSize(100, 30)
        self.saveButton.clicked.connect(self.saveConfiguration)
        button_layout.addWidget(self.saveButton)

        # Save Configuration button (aligned right)
        self.saveConfigurationButton = QtWidgets.QPushButton("Send Configuration", self.centralwidget)
        self.saveConfigurationButton.setFixedSize(300, 30)
        self.saveConfigurationButton.clicked.connect(lambda: self.saveConfiguration(flag="send"))
        button_layout.addWidget(self.saveConfigurationButton)

        self.layout.addLayout(button_layout)

    def saveConfiguration(self, flag:str):
        """
        @brief Validates inputs, encodes the configuration, and sends it via UART.
        """
        try:
            logger_id = self.validateUInt(self.lineEdit_Logger_ID.text(), "Logger ID")
            cell_id = self.validateUInt(self.lineEdit_Cell_ID.text(), "Cell ID")
            upload_method = self.comboBox_Upload_Method.currentText()

            # Calculate upload interval in seconds
            days = 0 if self.lineEdit_Days.text() == "" else self.validateUInt(self.lineEdit_Days.text(), "Days")
            hours = 0 if self.lineEdit_Hours.text() == "" else self.validateUInt(self.lineEdit_Hours.text(), "Hours")
            minutes = 0 if self.lineEdit_Minutes.text() == "" else self.validateUInt(self.lineEdit_Minutes.text(), "Minutes") 
            seconds = 0 if self.lineEdit_Seconds.text() == "" else self.validateUInt(self.lineEdit_Seconds.text(), "Seconds")
            upload_interval = days * 86400 + hours * 3600 + minutes * 60 + seconds
            
            # Check if the user entered time in upload interval
            if upload_interval==0:
                raise ValueError("You must Enter preferred time in \"upload interval\".")

            # Check if the user selected at least one sensor option 
            if not (self.checkBox_Voltage.isChecked() or
                 self.checkBox_Current.isChecked() or
                   self.checkBox_Teros12.isChecked() or
                     self.checkBox_Teros21.isChecked() or
                       self.checkBox_BME280.isChecked()):
                raise ValueError("You must choose at least one sensor.")
            
            # Convert enabled sensors into a list and filter out empty strings
            enabled_sensors = [
                sensor for sensor in [
                    'Voltage' if self.checkBox_Voltage.isChecked() else "",
                    'Current' if self.checkBox_Current.isChecked() else "",
                    'Teros12' if self.checkBox_Teros12.isChecked() else "",
                    'Teros21' if self.checkBox_Teros21.isChecked() else "",
                    'BME280' if self.checkBox_BME280.isChecked() else ""
                ] if sensor
            ]

            # Checked sensors to be saved in json
            enabled_sensors_json = {
                "Voltage" : self.checkBox_Voltage.isChecked(),
                "Current" : self.checkBox_Current.isChecked(),
                "Teros12" : self.checkBox_Teros12.isChecked(),
                "Teros21" : self.checkBox_Teros21.isChecked(),
                "BME280" : self.checkBox_BME280.isChecked()
            }
            calibration_v_slope = self.validateFloat(self.lineEdit_V_Slope.text(), "Calibration V Slope")
            calibration_v_offset = self.validateFloat(self.lineEdit_V_Offset.text(), "Calibration V Offset")
            calibration_i_slope = self.validateFloat(self.lineEdit_I_Slope.text(), "Calibration I Slope")
            calibration_i_offset = self.validateFloat(self.lineEdit_I_Offset.text(), "Calibration I Offset")

            # Add WiFi settings if WiFi is selected as the upload method
            if upload_method == "WiFi":
                wifi_ssid = self.lineEdit_WiFi_SSID.text()
                wifi_password = self.lineEdit_WiFi_Password.text()
                api_endpoint_url = self.validateURL(self.lineEdit_API_Endpoint_URL.text(), "API Endpoint URL")
                api_port = self.validateUInt(self.lineEdit_API_Port.text(), "API Port")
            else:
                wifi_ssid = ""
                wifi_password = ""
                api_endpoint_url = ""
                api_port = 0

            # Validate user input on case of WiFi
            if upload_method == "WiFi":
                if not self.lineEdit_WiFi_SSID.text():
                    raise ValueError("WiFi SSID cannot be empty.")
                if not self.lineEdit_WiFi_Password.text():
                    raise ValueError("WiFi Password cannot be empty.")
            
            # Construct the configuration dictionary to be saved in json file 
            configuration = {
                "Logger ID": logger_id,
                "Cell ID": cell_id,
                "Upload Method": upload_method,
                "Days": days,
                "Hours": hours,
                "Minutes": minutes,
                "Seconds": seconds,
                "Enabled Sensors": enabled_sensors_json,
                "Calibration V Slope": calibration_v_slope,
                "Calibration V Offset": calibration_v_offset,
                "Calibration I Slope": calibration_i_slope,
                "Calibration I Offset": calibration_i_offset,
                "WiFi SSID": wifi_ssid,
                "WiFi Password": wifi_password,
                "API Endpoint URL": api_endpoint_url,
                "API Port": api_port
            }

            # Check whether the user wants to send or just to save
            if flag == "send":
                # Construct the configuration dictionary to be encoded
                encoded_data = encode_user_configuration(
                    int(logger_id),
                    int(cell_id),
                    upload_method,
                    int(upload_interval),
                    enabled_sensors,
                    float(calibration_v_slope),
                    float(calibration_v_offset),
                    float(calibration_i_slope),
                    float(calibration_i_offset),
                    wifi_ssid,
                    wifi_password,
                    api_endpoint_url,
                    int(api_port)
            )
                # Send the encoded configuration via UART
                success = self.sendToUART(encoded_data)
                if not success:
                    return  # Don't save if sending failed
                
                print("------------------------------------------")
                print(encoded_data)
                print("------------------------------------------")
            try:
                 # Ensure the 'Load' directory exists
                load_dir = os.path.join(os.path.dirname(__file__), 'Load')
                os.makedirs(load_dir, exist_ok=True)
                # Save configuration as JSON
                config_path = os.path.join(load_dir, f'cell_{cell_id}_.json')
                with open(config_path, 'w') as json_file:
                    json.dump(configuration, json_file,indent=4)

                QtWidgets.QMessageBox.information(self.centralwidget, "Success", "Configurations saved successfully.")
            except Exception as e:
                QtWidgets.QMessageBox.critical(self.centralwidget, "Error", f"Failed to save configurations: {e}")
                        
            # Print success message in case of saving the config or saving and sending the config
            if flag == "send":
                print(f"Configuration saved and sent to STM32 successfully! Backup JSON file: {'cell_'+ str(cell_id) + '_' }")
            else:
                print(f"Configuration saved successfully! Backup JSON file: {'cell_'+ str(cell_id) + '_' }")

        except ValueError as e:
            # Show error message if validation fails
            QtWidgets.QMessageBox.critical(self.centralwidget, "Error", str(e))

    def loadConfiguration(self):
        """
        @brief Loads configuration from a selected JSON file and fills the input fields.
        """
        # Open a file dialog to select the JSON file
        options = QtWidgets.QFileDialog.Options()
        file_path, _ = QtWidgets.QFileDialog.getOpenFileName(
            self.centralwidget, "Select Configuration File", "", "JSON Files (*.json)", options=options
        )

        if file_path:
            try:
                with open(file_path, 'r') as json_file:
                    config = json.load(json_file)

                # Fill the GUI fields with the loaded configuration
                self.lineEdit_Logger_ID.setText(str(config.get("Logger ID", "")))
                self.lineEdit_Cell_ID.setText(str(config.get("Cell ID", "")))
                self.comboBox_Upload_Method.setCurrentText(config.get("Upload Method", "WiFi"))

                # Fill the upload interval fields
                self.lineEdit_Days.setText(str(config.get("Days", 0)))
                self.lineEdit_Hours.setText(str(config.get("Hours", 0)))
                self.lineEdit_Minutes.setText(str(config.get("Minutes", 0)))
                self.lineEdit_Seconds.setText(str(config.get("Seconds", 0)))

                # Fill sensor checkboxes
                enabled_sensors = config.get("Enabled Sensors", {})
                self.checkBox_Voltage.setChecked(enabled_sensors.get("Voltage", False))
                self.checkBox_Current.setChecked(enabled_sensors.get("Current", False))
                self.checkBox_Teros12.setChecked(enabled_sensors.get("Teros12", False))
                self.checkBox_Teros21.setChecked(enabled_sensors.get("Teros21", False))
                self.checkBox_BME280.setChecked(enabled_sensors.get("BME280", False))

                # Fill calibration fields
                self.lineEdit_V_Slope.setText(str(config.get("Calibration V Slope", "")))
                self.lineEdit_V_Offset.setText(str(config.get("Calibration V Offset", "")))
                self.lineEdit_I_Slope.setText(str(config.get("Calibration I Slope", "")))
                self.lineEdit_I_Offset.setText(str(config.get("Calibration I Offset", "")))

                # Fill WiFi settings if upload method is WiFi
                if config.get("Upload Method") == "WiFi":
                    self.lineEdit_WiFi_SSID.setText(config.get("WiFi SSID", ""))
                    self.lineEdit_WiFi_Password.setText(config.get("WiFi Password", ""))
                    self.lineEdit_API_Endpoint_URL.setText(config.get("API Endpoint URL", ""))
                    self.lineEdit_API_Port.setText(str(config.get("API Port", "")))
                else:
                    # Clear the WiFi fields if the method is not WiFi
                    self.lineEdit_WiFi_SSID.clear()
                    self.lineEdit_WiFi_Password.clear()
                    self.lineEdit_API_Endpoint_URL.clear()
                    self.lineEdit_API_Port.clear()

                QtWidgets.QMessageBox.information(self.centralwidget, "Success", "Configuration loaded successfully.")

            except Exception as e:
                QtWidgets.QMessageBox.critical(self.centralwidget, "Error", f"Failed to load configuration: {e}")
                
    def sendToUART(self, encoded_data):
        """
        @brief Sends the encoded configuration data via UART.

        @param data Encoded configuration data.
        """
        ser = None
        try:
            # List available ports with descriptions
            ports = serial.tools.list_ports.comports()
            available_ports = [f"{port.device} - {port.description}" for port in ports]

            if not available_ports:
                QtWidgets.QMessageBox.critical(self.centralwidget, "Error", "No serial ports available.")
                return False

            # Ask the user to select a port
            selected_port, ok = QInputDialog.getItem(self.centralwidget, "Select Port", "Available Serial Ports:", available_ports, 0, False)

            if not ok or not selected_port:
                QtWidgets.QMessageBox.critical(self.centralwidget, "Error", "No port selected.")
                return False

            # Extract the port name
            port_name = selected_port.split(' ')[0]

            # Open the serial port
            ser = serial.Serial(port=port_name, baudrate=115200, timeout=2)
            # Step 1: Send the length of the encoded data (2 bytes)
            data_length = len(encoded_data)
            ser.write(data_length.to_bytes(2, byteorder='big'))  # Send length as 2-byte big-endian integer

            # Step 2: Send the encoded data
            ser.write(encoded_data)
            print("________________________________________________________________________")
            print(f"length: {data_length}")
            print(f"Sent: {encoded_data}")

            # Step 3: Wait for acknowledgment ("ACK")
            ack = ser.read(3)  # Read 3 bytes (assuming "ACK" is 3 bytes)
            print(ack)
            print("________________________________________________________________________")
            if ack == b'ACK':
                QtWidgets.QMessageBox.information(self.centralwidget, "Success", "Received ACK from STM32")
            else:
                QtWidgets.QMessageBox.critical(self.centralwidget, "UART Error", "No acknowledgment received")
                return False

            # Step 4: After ACK, read back the same data from STM32
            received_data_length = int.from_bytes(ser.read(2), byteorder='big')  # Read the length of received data
            print("________________________________________________________________________")
            print(f"length: {received_data_length}")
            print("________________________________________________________________________")
            print("________________________________________________________________________")

            received_data = ser.read(received_data_length)  # Read the received data based on the length
            print(f"Received from STM32: {received_data}")
            print("________________________________________________________________________")

            # Step 5: Display the received data to confirm it's the same
            if received_data == encoded_data:
                QtWidgets.QMessageBox.information(self.centralwidget, "Success", "Data received matches the sent data.")
            else:
                QtWidgets.QMessageBox.critical(self.centralwidget, "Error", "Received data does not match sent data.")
            
            return True

        except serial.SerialException as e:
            QtWidgets.QMessageBox.critical(self.centralwidget, "UART Error", f"Failed to send data: {e}")
            return False

        finally:
            if ser is not None:
                ser.close()

    def validateURL(self, value, field_name):
        """
        @brief Validates that the input is a valid URL.
        @param value The input value to validate.
        @param field_name The name of the field being validated.
        @return The validated URL string.
        @throws ValueError if the input is invalid.
        """
        url_pattern = re.compile(
            r'^(https?|ftp):\/\/'  # http:// or https:// or ftp://
            r'([a-zA-Z0-9_-]+(?:(?:\.[a-zA-Z0-9_-]+)+))'  # domain
            r'(:[0-9]{1,5})?'  # port (?: optional)
            r'(\/.*)?$'  # path (?: optional)
        )
        if not url_pattern.match(value):
            raise ValueError(f"Invalid {field_name}. Must be a valid URL.")
        return value

    def validateUInt(self, value, name):
        """
        @brief Validates that the input is a positive integer.

        @param value The input value to validate.
        @param name The name of the parameter (for error messages).
        @return Validated unsigned integer value.
        """
        if not value.isdigit() or int(value) < 0:
            raise ValueError(f"{name} must be a positive integer.")
        return int(value)

    def validateInt(self, value, name):
        """
        @brief Validates that the input is an integer.

        @param value The input value to validate.
        @param name The name of the parameter (for error messages).
        @return Validated integer value.
        """
        try:
            return int(value)
        except ValueError:
            raise ValueError(f"{name} must be an integer.")

    def validateFloat(self, value, name):
        """
        @brief Validates that the input is a float.

        @param value The input value to validate.
        @param name The name of the parameter (for error messages).
        @return Validated float value.
        """
        try:
            return float(value)
        except ValueError:
            raise ValueError(f"{name} must be a floating-point number.")

if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    MainWindow = QtWidgets.QMainWindow()
    ui = Ui_MainWindow()
    ui.setupUi(MainWindow)
    MainWindow.show()
    sys.exit(app.exec_())
