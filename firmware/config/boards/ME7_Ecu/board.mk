# Combine the related files for a specific platform and MCU.


BOARDS_DIR = $(PROJECT_DIR)/config/boards

# Target ECU board design
BOARDSRC_CPP = $(BOARDS_DIR)/ME7_Ecu/board_configuration.cpp

# Target processor details
ifeq ($(PROJECT_CPU),ARCH_STM32F4)
  MCU_DEFS = -DSTM32F407xx
  BOARDSRC = $(CHIBIOS)/os/hal/boards/ST_STM32F4_DISCOVERY/board.c
  BOARDINC = $(BOARDS_DIR)/microrusefi
  BOARDINC += $(PROJECT_DIR)/config/stm32f4ems	# For board.h
  BOARDINC += $(BOARDS_DIR)/st_stm32f4
  LDSCRIPT= $(BOARDS_DIR)/prometheus/STM32F405xG.ld
else
  MCU_DEFS = -DSTM32F767xx
  BOARDSRC = $(CHIBIOS)/os/hal/boards/ST_NUCLEO144_F767ZI/board.c
  CONFDIR=config/stm32f7ems
  BOARDINC = $(BOARDS_DIR)/nucleo_f767		# For board.h
  BOARDINC += $(PROJECT_DIR)/config/stm32f7ems	# efifeatures/halconf/chconf.h
  LDSCRIPT= $(BOARDS_DIR)/nucleo_f767/STM32F76xxI.ld
endif


# Set this if you want a default engine type other than normal MRE
ifeq ($(DEFAULT_ENGINE_TYPE),)
  DEFAULT_ENGINE_TYPE = -DDEFAULT_ENGINE_TYPE=MINIMAL_PINS
endif
# Add them all together
DDEFS += $(MCU_DEFS) -DHAL_TRIGGER_USE_PAL=TRUE -DHAL_USE_ICU=FALSE -DEFI_VEHICLE_SPEED=FALSE -DEFI_LOGIC_ANALYZER=FALSE -DEFI_USE_OSC=TRUE -DEFI_FATAL_ERROR_PIN=GPIOF_0 -DFIRMWARE_ID=\"microRusEfi\" $(DEFAULT_ENGINE_TYPE)
