@echo off

REM Store the current directory (script folder)
set script_folder=%cd%

REM Set the default build folder
set build_folder=%script_folder%\..\build_windows

REM Check for -r flag (Release mode)
if "%1" == "-r" (
    set build_folder=%script_folder%\..\build_release_windows
)

REM Check if the build folder exists
if exist "%build_folder%" (
    REM Remove all files in the build folder
    echo Cleaning build folder: %build_folder%
    rmdir /s /q "%build_folder%"
)

REM Call the existing build script
call build.bat %*

pause
