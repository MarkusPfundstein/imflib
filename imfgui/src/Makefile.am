# compiler flags

include moc.mk

AM_CXXFLAGS = 
AM_LDFLAGS = 

LIBS += $(QT4_LIBS)
LIBS += $(XERCES_LIBS)
LIBS += -lboost_filesystem

bin_PROGRAMS = imfgui

imfgui_MOC_FILES = \
	moc_imfpackageview.cpp \
	moc_cplsequenceview.cpp \
	moc_cplsequencecontrolview.cpp \
	moc_packagetableview.cpp \
	moc_cplresourcerect.cpp

# please somebody explain to me what this does
$(imfgui_OBJECTS) : $(imfgui_MOC_FILES)

imfgui_SOURCES = \
	main.cpp \
	application.cpp \
	applicationsettings.cpp \
	utils/mxfreader.cpp \
	utils/uuidgenerator.cpp \
	qtmodels/imfpackagemodel.cpp \
	model/genericitem.cpp \
	model/cplresource.cpp \
	model/cplsegment.cpp \
	model/cplsequence.cpp \
	model/cplvirtualtrack.cpp \
	model/imfpackageitem.cpp \
	model/imftrack.cpp \
	model/imfvideotrack.cpp \
	model/imfaudiotrack.cpp \
	model/imfpackage.cpp \
	model/imfcompositionplaylist.cpp \
	model/imfoutputprofile.cpp \
	drawing/cplresourcerect.cpp \
	views/cplsequencecontrolview.cpp \
	views/cplsequenceview.cpp \
	views/imfpackageview.cpp \
	views/packagetableview.cpp

imfgui_CXXFLAGS = --pedantic -Wall -std=c++0x -Wshadow -Wcast-align -Wunreachable-code
imfgui_CXXFLAGS += $(QT4_CFLAGS)

imfgui_LDFLAGS = 

CLEANFILES = $(imfgui_MOC_FILES)
