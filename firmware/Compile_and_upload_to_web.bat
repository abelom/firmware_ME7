
echo Compiler gcc version
arm-none-eabi-gcc -v

rd /s /q deliver
mkdir deliver
cd deliver
mkdir ME7_Ecu
mkdir MR_Racing
mkdir BlackMagic_Mini

cd..
call clean.bat
echo "TIMESTAMP %date% %time%"
set LDSCRIPT = config/boards/NUCLEO_F767/STM32F76xxI.ld
set PROJECT = ME7_Ecu
set PROJECT_BOARD=ME7_Ecu
set PROJECT_CPU=ARCH_STM32F7
make -j4
set EXTRA_PARAMS=-DHAL_TRIGGER_USE_PAL=TRUE -DHAL_USE_ICU=FALSE -DEFI_VEHICLE_SPEED=FALSE -DEFI_LOGIC_ANALYZER=FALSE -DEFI_USE_OSC=TRUE

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


call clean.bat
echo "TIMESTAMP %date% %time%"
set PROJECT_BOARD=proteus
set PROJECT_CPU=ARCH_STM32F4
set EXTRA_PARAMS=-DHAL_TRIGGER_USE_PAL=TRUE -DHAL_USE_ICU=FALSE -DEFI_VEHICLE_SPEED=FALSE -DEFI_LOGIC_ANALYZER=FALSE
make -j4
rem mv build\rusefi.elf deliver/MR_Racing\MR_Racing.elf
mv build\rusefi.bin deliver/MR_Racing\MR_Racing.bin
rem this file is needed for DFU generation
mv build\rusefi.hex deliver/MR_Racing\MR_Racing.hex
echo Release compilation results 1/2
echo "TIMESTAMP %date% %time%"
ls -l build
if not exist deliver/MR_Racing/MR_Racing.hex echo FAILED to compile NO ASSERTS version
if not exist deliver/MR_Racing/MR_Racing.hex exit -1

rm -f deliver/MR_Racing/MR_Racing.dfu
echo %script_name%: invoking hex2dfu.exe
..\misc\encedo_hex2dfu\hex2dfu.exe -i deliver/MR_Racing/MR_Racing.hex -o deliver\MR_Racing/MR_Racing.dfu


call clean.bat


call gen_config.bat
echo "TIMESTAMP %date% %time%"
set PROJECT_BOARD=microrusefi
set PROJECT_CPU=ARCH_STM32F4
set EXTRA_PARAMS= -DPROJECT_BOARD_MRE=TRUE
make -j4
rem mv build\rusefi.elf deliver/BlackMagic_Mini\BlackMagic_Mini.elf
mv build\rusefi.bin deliver/BlackMagic_Mini\BlackMagic_Mini.bin
rem this file is needed for DFU generation
mv build\rusefi.hex deliver/BlackMagic_Mini\BlackMagic_Mini.hex
echo Release compilation results 1/2
echo "TIMESTAMP %date% %time%"
ls -l build
if not exist deliver/BlackMagic_Mini\BlackMagic_Mini.hex echo FAILED to compile NO ASSERTS version
if not exist deliver/BlackMagic_Mini\BlackMagic_Mini.hex exit -1

rm -f deliver/BlackMagic_Mini/BlackMagic_Mini.dfu
echo %script_name%: invoking hex2dfu.exe
..\misc\encedo_hex2dfu\hex2dfu.exe -i deliver/BlackMagic_Mini/BlackMagic_Mini.hex -o deliver\BlackMagic_Mini/BlackMagic_Mini.dfu
call gen_config.bat
echo %script_name%: deliver folder
ls -l deliver
cp 7za.exe deliver\7za.exe
cp Unbrick_Basetune.msq deliver/ME7_Ecu\Unbrick_Basetune.msq
cp Unbrick_Basetune.msq deliver/BlackMagic_Mini\Unbrick_Basetune.msq
cp Readme.txt deliver/BlackMagic_Mini\Readme.txt
cp Unbrick_Basetune.msq deliver/BlackMagic_Mini\Unbrick_Basetune.msq
cp Readme.txt deliver/MR_Racing\Readme.txt
cd deliver
7za a ME7_Ecu.zip ME7_Ecu\"*"
mv ME7_Ecu.zip ME7_Ecu\ME7_Ecu.zip
7za a MR_Racing.zip MR_Racing\"*"
mv MR_Racing.zip MR_Racing\MR_Racing.zip
7za a BlackMagic_Mini.zip BlackMagic_Mini\"*"
mv BlackMagic_Mini.zip BlackMagic_Mini\BlackMagic_Mini.zip
cd..
call config/boards/clean_env_variables.bat
winscp.com /ini=nul /script=ftp_script.txt

echo clean_compile_two_versions: Looks good!
