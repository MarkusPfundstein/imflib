# compiler flags

AM_CXXFLAGS = 
AM_LDFLAGS = 

bin_PROGRAMS = imfgui

imfgui_SOURCES = \
	main.cpp

imfgui_CXXFLAGS = --pedantic -Wall -std=c++0x -Wshadow -Wcast-align -Wswitch-default -Wswitch-enum -Wunreachable-code
imfgui_CXXFLAGS += $(QT4_CFLAGS)

imfgui_LDFLAGS = $(QT4_LIBS)
