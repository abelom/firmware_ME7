
cd ../../..

set PROJECT_BOARD=ME7_Ecu
set PROJECT_CPU=ARCH_STM32F4

call config/boards/common_make.bat

call config/boards/clean_env_variables.bat
