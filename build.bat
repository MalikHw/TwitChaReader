@echo off
echo Building TwitChaReader for Windows...

if not exist build mkdir build
cd build

cmake .. -G "Visual Studio 17 2022" -A x64

if %errorlevel% equ 0 (
    echo CMake configuration successful!
    cmake --build . --config Release
    
    if %errorlevel% equ 0 (
        echo Build successful!
        echo Executable location: build\Release\TwitChaReader.exe
        
        echo Running windeployqt...
        cd Release
        windeployqt TwitChaReader.exe
        cd ..\..
    ) else (
        echo Build failed!
        exit /b 1
    )
) else (
    echo CMake configuration failed!
    exit /b 1
)

pause
