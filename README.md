

# fScout Project Setup for Windows

[![C/C++ CI](https://github.com/Jakobimatrix/fScout/actions/workflows/ubuntu_build_test.yml/badge.svg)](https://github.com/Jakobimatrix/fScout/actions/workflows/ubuntu_build_test.yml)

 - OS: Ubuntu 24.04
 - compiler: clang 19, gcc 13
 - debug + release
 - tests

---

### 1. Prerequisites

Before you begin, ensure you have the following installed on your system:

1. **Git**: For cloning the repository.
2. **CMake**: Required for building the project. Download from [CMake's official website](https://cmake.org/download/). Add to System PATH!
3. **Qt**: Install the Qt dependencies using the online installer.

### 2. Qt Installation Steps

1. Download the Qt online installer from [Qt Downloads](https://www.qt.io/download).
2. Create an account and log in.
3. During installation:
   - Check the boxes for "Latest Supported Releases" AND "Archive."
   - Select **Qt 5.15.2** with **MinGW 8.1.0** for both 32-bit AND 64-bit.
   - Under **Developer and Designer Tools**, select **MinGW 13.1.0** 64-bit.

### 3. Clone the Project

Clone the project repository from GitHub:

```bash
git clone git@github.com:Jakobimatrix/fScout.git
```

#### Update Subrepositories

After cloning the project, navigate into the project folder and run the following command to initialize and update the submodules:

```bash
git submodule update --init --recursive
```

### 4. Build the Project

 - Navigate to the scripts folder within the project directory.
 - Run the build script. You have two options:  
   - For a release build 64 bit, run:  
        `build.bat`
   - For a release build 32 bit, run:  
        `build.bat --32`
   - For a debug build 64 bit, run:  
        `build.bat -d`
   - For a debug build 32 bit, run:  
        `build.bat -d  --32`

The scripts will handle the build process, including setting up the environment and compiling the code and copying the QT dlls.
At the end there will be a portable version inside a new zip folder. (currently the resources folder inside the zip is missing... I am sorry :(
You can add them manually. You can also create an installer (which will have all resources)

### 5. Build the Installer

 - Install Inno: https://jrsoftware.org/isinfo.php
 - Start the Inno Wizzard. The script is located scripts/ino.iss
 - click "compile" -> the fScoutInstaller.exe should be created. 
 
