#!/bin/bash
echo "Installing system dependencies..."
apt-get update
apt-get install -y gcc

echo "Building C backend..."
cd backend_c
mkdir -p build
gcc src/main.c \
    src/fileio.c \
    src/allocation.c \
    src/graph.c \
    src/hashmap.c \
    src/heap.c \
    src/report.c \
    -Iinclude \
    -o build/resq \
    -lm

chmod +x build/resq
cd ..

echo "Build complete!"
