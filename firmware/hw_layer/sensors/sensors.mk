HW_SENSORS_DIR=$(PROJECT_DIR)/hw_layer/sensors

HW_SENSORS_INC=$(HW_SENSORS_DIR)

HW_SENSORS_SRC = \
	$(HW_SENSORS_DIR)/cj125.cpp \
	$(HW_SENSORS_DIR)/cj125_logic.cpp \
	$(HW_SENSORS_DIR)/yaw_rate_sensor.cpp \
	$(HW_SENSORS_DIR)/accelerometer.cpp \
	$(HW_SENSORS_DIR)/joystick.cpp \
	