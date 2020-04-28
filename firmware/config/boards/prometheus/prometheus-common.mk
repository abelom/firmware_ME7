# List of all the board related files.
BOARDSRC = $(CHIBIOS)/os/hal/boards/ST_STM32F469I_DISCOVERY/board.c \
    $(PROJECT_DIR)/config/boards/prometheus/board_extra.c

BOARDSRC_CPP = $(PROJECT_DIR)/config/boards/Prometheus/board_configuration.cpp

# Required include directories
BOARDINC = $(PROJECT_DIR)/config/boards/prometheus

# This board uses bootloader
USE_BOOTLOADER=yes

# include Prometheus bootloader code
BOOTLOADERINC= $(PROJECT_DIR)/bootloader/prometheus/$(PROMETHEUS_BOARD)

# Default to a release build - clear EXTRA_PARAMS from cmdline to build debug
ifeq ($(EXTRA_PARAMS),)
	EXTRA_PARAMS=-DEFI_ENABLE_ASSERTS=FALSE -DCH_DBG_ENABLE_ASSERTS=FALSE -DCH_DBG_ENABLE_STACK_CHECK=FALSE -DCH_DBG_FILL_THREADS=FALSE -DCH_DBG_THREADS_PROFILING=FALSE
endif
ifeq ($(DEBUG_LEVEL_OPT),)
	DEBUG_LEVEL_OPT = -O2
endif

ifeq ($(PROMETHEUS_BOARD),405)
LDSCRIPT= $(PROJECT_DIR)/config/boards/prometheus/STM32F405xG.ld
DDEFS += -DDEFAULT_ENGINE_TYPE=PROMETHEUS_DEFAULTS -DSTM32F405xx -DFIRMWARE_ID=\"prometeus405\"
else
# Override DEFAULT_ENGINE_TYPE
LDSCRIPT= $(PROJECT_DIR)/config/boards/prometheus/STM32F469xI.ld
DDEFS += -DDEFAULT_ENGINE_TYPE=PROMETHEUS_DEFAULTS -DSTM32F469xx -DFIRMWARE_ID=\"prometeus469\"
endif
