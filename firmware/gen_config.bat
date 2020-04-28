@echo off
rem This batch files reads rusefi_config.txt and produses firmware persistent configuration headers
rem the storage section of rusefi.ini is updated as well

rm gen_config.log
rm gen_config_board.log

rem lazy is broken - TS input is not considered a change
rm build/config.gen

mkdir build

java -DSystemOut.name=gen_config ^
 -Drusefi.generator.lazyfile.enabled=true ^
 -jar ../java_tools/ConfigDefinition.jar ^
 -definition integration\rusefi_config.txt ^
 -romraider integration ^
 -ts_destination tunerstudio ^
 -with_c_defines false ^
 -initialize_to_zero false ^
 -tool gen_config.bat ^
 -c_defines        controllers\generated\rusefi_generated.h ^
 -c_destination    controllers\generated\engine_configuration_generated_structures.h ^
 -c_fsio_constants controllers\generated\fsio_enums_generated.def ^
 -c_fsio_getters   controllers\generated\fsio_getters.def ^
 -c_fsio_names     controllers\generated\fsio_names.def ^
 -c_fsio_strings   controllers\generated\fsio_strings.def ^
 -java_destination ../java_console/models/src/com/rusefi/config/generated/Fields.java ^
 -romraider_destination ../java_console/rusefi.xml ^
 -skip build/config.gen
IF NOT ERRORLEVEL 0 echo ERROR generating
IF NOT ERRORLEVEL 0 EXIT /B 1


rem This would automatically copy latest file to 'dev' TS project
set ts_path="%HOMEDRIVE%%HOMEPATH%\Documents\TunerStudioProjects"
echo %ts_path%
cp tunerstudio/ME7_Ecu.ini deliver\ME7_Ecu\ME7_Ecu.ini
cp tunerstudio/proteus.ini deliver\MR_Racing\MR_Racing.ini
cp tunerstudio/microrusefi.ini deliver\BlackMagic_Mini\BlackMagic_Mini.ini

call gen_config_board ME7_Ecu
IF NOT ERRORLEVEL 0 echo ERROR generating
IF NOT ERRORLEVEL 0 EXIT /B 1



call gen_config_board proteus
IF NOT ERRORLEVEL 0 echo ERROR generating
IF NOT ERRORLEVEL 0 EXIT /B 1


cd config\boards\kinetis\config
!gen_config.bat
