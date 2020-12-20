# About
This project is designed to provide visual feedback from a midi piano. The intended use case is to screen capture it with OBS. If say you're a keyboard player who streams on twitch it gives your audience immediate feedback on what keys were pressed and when.

# Building
Target platform is linux. The code may need to be reworked for windows use. librtmidi and qt are required to build.
* debian: sudo apt install librtmidi-dev qt5-qmake
* arch: sudo pacman -S qt5-base rtmidi

qmake .
make -j$(nproc)
cp nothesia /usr/local/bin

# Usage
Use qjackctl to connect the keyboard to the program. Set the range of your piano in main.cpp (That should and might be changed at some point)

# Licence
Lets go with GPL3
