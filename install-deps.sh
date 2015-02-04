#!/bin/bash
set -e -x

# install glm
cd
curl -L -o glm-0.9.6.1.zip "http://sourceforge.net/projects/ogl-math/files/glm-0.9.6.1/glm-0.9.6.1.zip/download"
unzip glm-0.9.6.1.zip
cd glm
mkdir build && cd build
cmake ..
make
sudo make install

# install jansson
cd
curl -L -o jansson-2.7.tar.bz2 "http://www.digip.org/jansson/releases/jansson-2.7.tar.bz2"
tar xf jansson-2.7.tar.bz2
cd jansson-2.7
./configure --prefix=/usr
make
sudo make install

# install sdl2
cd
curl -L -o SDL2-2.0.3.tar.gz "https://www.libsdl.org/release/SDL2-2.0.3.tar.gz"
tar xf SDL2-2.0.3.tar.gz
cd SDL2-2.0.3
./configure --prefix=/usr
make
sudo make install
