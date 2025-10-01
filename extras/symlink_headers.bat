@echo off
rem Workaround for Arduino IDE's lackluster project file system
cd ..\color_to_sound
del /q .\common.h
mklink .\common.h ..\common.h
cd ..\motor_controller
del /q .\common.h
mklink .\common.h ..\common.h

rem New-Item -Force -ItemType SymbolicLink -Value .\common.h -Path .\color_to_sound\common.h
rem New-Item -Force -ItemType SymbolicLink -Value .\common.h -Path .\motor_controller\common.h
