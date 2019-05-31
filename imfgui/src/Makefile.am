# compiler flags

include moc.mk

LIBS += $(QT4_LIBS)
LIBS += $(XERCES_LIBS)
LIBS += -lboost_filesystem

bin_PROGRAMS = imfgui

imfgui_MOC_FILES = \
	moc_imfpackageview.cpp \
	moc_cplsequenceview.cpp \
	moc_cplsequencecontrolview.cpp \
	moc_packagetableview.cpp \
	moc_cplresourcerect.cpp \
	moc_cplsegmentrect.cpp

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
	model/imfpackagelist.cpp \
    model/imfessencedescriptor.cpp \
	views/cplrenderrect.cpp \
	views/cplsequencerect.cpp \
	views/cplsegmentrect.cpp \
	views/cplresourcerect.cpp \
	views/cplsequencecontrolview.cpp \
	views/cplsequenceview.cpp \
	views/imfpackageview.cpp \
	views/packagetableview.cpp

LPATH=../../third_party
INC_PATH=-I${LPATH}/openjpeg/include -I${LPATH}/asdcplib/include -I${LPATH}/ffmpeg/include
LIB_PATH=-L${LPATH}/openjpeg/lib -L${LPATH}/asdcplib/lib -L${LPATH}/ffmpeg/lib

imfgui_CXXFLAGS = --pedantic -Wall -std=c++11 -Wshadow -Wcast-align -Wunreachable-code ${INC_PATH}
imfgui_CXXFLAGS += $(QT4_CFLAGS)

imfgui_LDFLAGS = ${LIB_PATH} 

CLEANFILES = $(imfgui_MOC_FILES)
