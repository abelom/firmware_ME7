
cd ../../..

set LDSCRIPT = config/boards/NUCLEO_F767/STM32F76xxI.ld
set PROJECT = Black_Magic_ME7
set PROJECT_BOARD=Black_Magic_ME7
set PROJECT_CPU=ARCH_STM32F7

call config/boards/common_make.bat

