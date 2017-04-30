# imflib

## dependencies
sudo apt-get libboost-all-dev
sudo apt-get libqt4*
libopenjpeg from imflib_thirdparty
ffmpeg incl. shared and static libraries from imflib_thirdparty
asdcplib from imf_asdcplib_fork incl. all headers in /usr/local/include

after installing all deps, run sudo ldconfig once

## installation

after installing all dependencies

./configure 
make
sudo cp imfenc/src/imfenc /usr/local/bin
sudo cp imfgui/src/imfgui /usr/local/bin
