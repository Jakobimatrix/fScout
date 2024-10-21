@echo off

REM Store the current directory (script folder)
set script_folder=%cd%

REM Set the path where CMakeLists.txt is located (parent folder of scripts, adjust if needed)
set source_folder=%script_folder%\..

REM Set default compiler paths
set CC=C:\Qt\Tools\mingw1310_64\bin\gcc.exe
set CXX=C:\Qt\Tools\mingw1310_64\bin\g++.exe

set build_folder_name=build_windows
set build_type=Release
set build_bit=64

REM Check for -d flag (Debug mode) or --32 flag (32-bit mode)
:parse_args
if "%1" == "-d" (
    set build_type=Debug
    shift
    goto parse_args
) else if "%1" == "--32" (
    set build_bit=32
    shift
    goto parse_args
)

REM Set the build folder and CMake command
set build_folder=%source_folder%\%build_folder_name%_%build_type%_%build_bit%
set cmake_command=cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=%build_type% %source_folder%

set CMAKE_PREFIX_PATH=C:\Qt\5.15.2\mingw81_%build_bit%

REM Create build folder if it doesn't exist
if not exist "%build_folder%" (
    mkdir "%build_folder%"
)

cd "%build_folder%"

REM Add MinGW bin to PATH temporarily
set PATH=C:\Qt\Tools\mingw1310_64\bin;%PATH%

REM Run CMake command with MinGW generator and build with MinGW
%cmake_command%

REM Run the build command (use mingw32-make instead of MSVC tools)
mingw32-make -j8

REM Check if the build was successful
if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    exit /b %ERRORLEVEL%
)

REM Set the path to windeployqt based on the build bit
set windeployqt_path=C:\Qt\5.15.2\mingw81_%build_bit%\bin\windeployqt.exe

REM Copy necessary DLLs using windeployqt
%windeployqt_path% "%build_folder%\src\executable\finder_start.exe"

REM Retrieve version from CMakeLists.txt
for /f "tokens=2" %%i in ('findstr "project(fScout VERSION" "%source_folder%\CMakeLists.txt"') do set version=%%i

REM Get the current date in year_month_day format (adjust based on your locale)
for /f "tokens=1-3 delims=." %%d in ("%date%") do set date=%%d_%%e_%%f

REM Create zip folder name
set zip_folder_name=fScout_%build_type%_%build_bit%_%version%_%date%

REM Define excluded folders and files
set excluded_folders=CMakeFiles finder_start_autogen
set excluded_files=Makefile cmake_install.cmake


REM Check if 7z is available in the PATH
where 7z >nul 2>nul
if %errorlevel% equ 0 (
    REM Use 7z to zip everything inside the executable folder, force overwrite (-y)
    7z a -y "..\%zip_folder_name%.zip" "%build_folder%\src\executable\*"
) else (
    REM Fallback to PowerShell to create the zip file, force overwrite (-Force)
    powershell -command "Compress-Archive -Path '%build_folder%\src\executable\*' -DestinationPath '..\%zip_folder_name%.zip' -Force"
)


REM Return to the script folder after building
cd "%script_folder%"

pause
