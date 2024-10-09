#!/bin/bash

timestamp=$(date +%s)


defines="-DENGINE"
libs="-luser32 -lopengl32 -lgdi32"

warnings="-Wno-writable-strings -Wno-format-security -Wno-deprecated-declarations -Wno-switch"
includes="-Ithird_party -Ithird_party/Include"

clang++ $includes -g src/main.cpp -obreakout.exe $libs $warnings $defines

rm -f game_* # remove old game files

# Compile the game.cpp source file into a shared library (.dll)
# -g: Include debugging information
# -shared: Create a shared library
# -o game_$timestamp.dll: Output file named with a timestamp
clang++ -g "src/game.cpp" -shared -o game_$timestamp.dll $warnings $defines

# Rename the newly created .dll file to game.dll
# This ensures that the game always loads the latest version
mv game_$timestamp.dll game.dll