ACLOCAL_AMFLAGS = -I m4

EXTRADIST = 

AUTOMAKE_OPTIONS=subdir-objects

SUBDIRS = src

doc_DATA = AUTHORS ChangeLog COPYING INSTALL NEWS README

dist-hook:
	-rm -rf `find $(distdir) -name .git`
