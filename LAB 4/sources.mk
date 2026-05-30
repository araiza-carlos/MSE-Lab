# This is a makefile for our source files 
SRC_DIR = Sources
INCLUDE_DIR = Includes
CMSIS_DIR = ./CMSIS

# List all your source files here
SRCS = \
$(SRC_DIR)/main.c \
$(SRC_DIR)/stm32.startup.c \
$(SRC_DIR)/system_stm32f4xx.c \
$(SRC_DIR)/gpio_driver.c \
$(SRC_DIR)/tim_driver.c \
$(SRC_DIR)/adc_driver.c \
$(SRC_DIR)/serial.c \
$(SRC_DIR)/uart.c \
$(SRC_DIR)/utils.c \
$(SRC_DIR)/sensor.c

# List all your include directories here
INCLUDES = \
-I$(INCLUDE_DIR) \
-I$(CMSIS_DIR) \
-I$(CMSIS_DIR)/STM32F4xx