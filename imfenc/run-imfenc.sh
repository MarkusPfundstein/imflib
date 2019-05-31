CUR_PATH=$(pwd)
LPATH=${CUR_PATH}/third_party

export LD_LIBRARY_PATH=${LPATH}/openssl/lib:${LPATH}/asdcplib/lib:${LPATH}/ffmpeg/lib:${LPATH}/openjpeg/lib

PROG=imfenc/src/imfenc

${PROG} $@
