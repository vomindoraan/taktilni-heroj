@echo off
rem Workaround for Arduino IDE's lackluster project file system

cd "%~dp0"

set src_dir=..
set dst_dirs=..\color_to_sound ..\motor_controller
set files=common.h

for %%d in (%dst_dirs%) do (
    for %%f in (%files%) do (
        del /q %%d\%%f
        mklink %%d\%%f %src_dir%\%%f
    )
)
