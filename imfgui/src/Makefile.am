# compiler flags

include moc.mk

AM_CXXFLAGS = 
AM_LDFLAGS = 

LIBS += $(QT4_LIBS)

bin_PROGRAMS = imfgui

imfgui_MOC_FILES = moc_imfpackageview.cpp

# please somebody explain to me what this does
$(imfgui_OBJECTS) : $(imfgui_MOC_FILES)

imfgui_SOURCES = \
	main.cpp \
	application.cpp \
	applicationsettings.cpp \
	utils/mxfreader.cpp \
	qtmodels/imfpackagemodel.cpp \
	model/imfpackageitem.cpp \
	model/imftrack.cpp \
	model/imfvideotrack.cpp \
	model/imfaudiotrack.cpp \
	model/imfpackage.cpp \
	model/imfcompositionplaylist.cpp \
	model/imfoutputprofile.cpp \
	views/imfpackageview.cpp

imfgui_CXXFLAGS = --pedantic -Wall -std=c++0x -Wshadow -Wcast-align -Wunreachable-code
imfgui_CXXFLAGS += $(QT4_CFLAGS)

imfgui_LDFLAGS = 

CLEANFILES = $(imfgui_MOC_FILES)
