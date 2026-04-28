# This is a makefile for our source files 
SRC_DIR = ../src
INCLUDE_DIR = ../include
CMSIS_DIR = ../CMSIS

# List all your source files here
SRCS = \
$(SRC_DIR)/main.c \
$(SRC_DIR)/stm32.startup.c \
$(SRC_DIR)/system_stm32f4xx.c \
$(SRC_DIR)/gpio_driver.c \
$(SRC_DIR)/led.c \
$(SRC_DIR)/button.c \
$(SRC_DIR)/tim_driver.c \
$(SRC_DIR)/timer.c \
$(SRC_DIR)/pwm.c

# List all your include directories here
INCLUDES = \
-I$(INCLUDE_DIR) \
-I$(CMSIS_DIR) \
-I$(CMSIS_DIR)/STM32F4xx