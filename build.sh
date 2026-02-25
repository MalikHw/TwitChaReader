#!/bin/bash

echo "Building TwitChaReader..."

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -eq 0 ]; then
    echo "CMake configuration successful!"
    cmake --build . --config Release -j$(nproc)
    
    if [ $? -eq 0 ]; then
        echo "Build successful!"
        echo "Executable location: build/TwitChaReader"
    else
        echo "Build failed!"
        exit 1
    fi
else
    echo "CMake configuration failed!"
    exit 1
fi
