
echo Compiler gcc version
arm-none-eabi-gcc -v

rd /s /q deliver
mkdir deliver
cd deliver
mkdir ME7_Ecu
mkdir RM_Racing
mkdir MRE_F4

cd..
call clean.bat
echo "TIMESTAMP %date% %time%"
set PROJECT_BOARD=microrusefi
set PROJECT_CPU=ARCH_STM32F4
set EXTRA_PARAMS= -DPROJECT_BOARD_MRE=TRUE
make -j4
rem mv build\rusefi.elf deliver/MRE_F4\MicroRusefi_F4.elf
mv build\rusefi.bin deliver/MRE_F4\MicroRusefi_F4.bin
rem this file is needed for DFU generation
mv build\rusefi.hex deliver/MRE_F4\MicroRusefi_F4.hex
echo Release compilation results 1/2
echo "TIMESTAMP %date% %time%"
ls -l build
if not exist deliver/MRE_F4\MicroRusefi_F4.hex echo FAILED to compile NO ASSERTS version
if not exist deliver/MRE_F4\MicroRusefi_F4.hex exit -1

rm -f deliver/MRE_F4/MicroRusefi_F4.dfu
echo %script_name%: invoking hex2dfu.exe
..\misc\encedo_hex2dfu\hex2dfu.exe -i deliver/MRE_F4/MicroRusefi_F4.hex -o deliver\MRE_F4/MicroRusefi_F4.dfu

call clean.bat
echo "TIMESTAMP %date% %time%"
set PROJECT_BOARD=proteus
set PROJECT_CPU=ARCH_STM32F4
set EXTRA_PARAMS=-DHAL_TRIGGER_USE_PAL=TRUE -DHAL_USE_ICU=FALSE -DEFI_VEHICLE_SPEED=FALSE -DEFI_LOGIC_ANALYZER=FALSE
make -j4
rem mv build\rusefi.elf deliver/RM_Racing\RM_Racing.elf
mv build\rusefi.bin deliver/RM_Racing\RM_Racing.bin
rem this file is needed for DFU generation
mv build\rusefi.hex deliver/RM_Racing\RM_Racing.hex
echo Release compilation results 1/2
echo "TIMESTAMP %date% %time%"
ls -l build
if not exist deliver/RM_Racing/RM_Racing.hex echo FAILED to compile NO ASSERTS version
if not exist deliver/RM_Racing/RM_Racing.hex exit -1

rm -f deliver/RM_Racing/RM_Racing.dfu
echo %script_name%: invoking hex2dfu.exe
..\misc\encedo_hex2dfu\hex2dfu.exe -i deliver/RM_Racing/RM_Racing.hex -o deliver\RM_Racing/RM_Racing.dfu


call clean.bat
echo "TIMESTAMP %date% %time%"
set LDSCRIPT = config/boards/NUCLEO_F767/STM32F76xxI.ld
set PROJECT = ME7_Ecu
set PROJECT_BOARD=microrusefi
set PROJECT_CPU=ARCH_STM32F7
make -j4
set EXTRA_PARAMS=

rem mv build\rusefi.elf deliver/ME7_Ecu\ME7_Ecu.elf

mv build\rusefi.hex deliver/ME7_Ecu\ME7_Ecu.hex
rem this file is needed for DFU generation
cp build\rusefi.bin deliver/ME7_Ecu\ME7_Ecu.bin
echo Debug compilation results 2/2
echo "TIMESTAMP %date% %time%"
ls -l build
if not exist deliver/ME7_Ecu/ME7_Ecu.hex echo FAILED to compile DEFAULT with DEBUG
if not exist deliver/ME7_Ecu/ME7_Ecu.hex exit -1

rm -f deliver/ME7_Ecu/ME7_Ecu.dfu
echo %script_name%: invoking hex2dfu.exe
..\misc\encedo_hex2dfu\hex2dfu.exe -i deliver/ME7_Ecu/ME7_Ecu.hex -o deliver/ME7_Ecu/ME7_Ecu.dfu
call gen_config.bat
echo %script_name%: deliver folder
ls -l deliver
cp 7za.exe deliver\7za.exe
cp Unbrick_Basetune.msq deliver/ME7_Ecu\Unbrick_Basetune.msq
cp Unbrick_Basetune.msq deliver/MRE_F4\Unbrick_Basetune.msq
cp Unbrick_Basetune.msq deliver/RM_Racing\Unbrick_Basetune.msq
cd deliver
7za a ME7_Ecu.zip ME7_Ecu\*.*
mv ME7_Ecu.zip ME7_Ecu\ME7_Ecu.zip
7za a RM_Racing.zip RM_Racing\*.*
mv RM_Racing.zip RM_Racing\RM_Racing.zip
7za a MRE_F4.zip MRE_F4\*.*
mv MRE_F4.zip MRE_F4\MRE_F4.zip
cd..
call config/boards/clean_env_variables.bat
winscp.com /ini=nul /script=ftp_script.txt

echo clean_compile_two_versions: Looks good!
