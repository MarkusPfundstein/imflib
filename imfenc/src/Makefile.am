# compiler flags

#AM_CXXFLAGS = 
AM_LDFLAGS = ${AM_LDFLAGS}

#LIBS = -lopenjp2 -lboost_fileystem -lboost_system -las02 -lasdcp -lkumu -lavcodec -lavformat -lavutil -lswresample -lswscale

# ugly hack. somehow autoconf cant figure out for the life of it if libboost_filesystem is installed
LIBS += -lboost_filesystem

bin_PROGRAMS = imfenc

imfenc_SOURCES = \
	cli.cpp \
	j2kencoder.cpp \
	pcmencoder.cpp \
	inputstreamdecoder.cpp \
	mxfwriter.cpp \
	wavmuxer.cpp

LPATH=../../third_party
INC_PATH=-I${LPATH}/openjpeg/include -I${LPATH}/asdcplib/include -I${LPATH}/ffmpeg/include
LIB_PATH=-L${LPATH}/openjpeg/lib -L${LPATH}/asdcplib/lib -L${LPATH}/ffmpeg/lib

imfenc_CXXFLAGS = --pedantic -Wall -std=c++11 -Wshadow -Wcast-align -Wswitch-default -Wswitch-enum -Wunreachable-code -D__STDC_CONSTANT_MACROS ${INC_PATH}

imfenc_LDFLAGS = ${LIB_PATH}
