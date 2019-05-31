LPATH=$(pwd)/third_party

export LD_LIBRARY_PATH="${LPATH}/openssl/lib:${LPATH}/openjpeg/lib:${LPATH}/ffmpeg/lib:${LPATH}/asdcplib/lib"

PROG=imfenc/src/imfenc
${PROG} $@
