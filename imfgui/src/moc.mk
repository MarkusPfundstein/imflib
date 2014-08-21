
# would be great to figure out how to "automate" this the make way
moc_imfpackageview.cpp:  
	$(MOC) -i -o $@ views/imfpackageview.h

moc_cplsequenceview.cpp:
	$(MOC) -i -o $@ views/cplsequenceview.h

moc_cplsequencecontrolview.cpp:
	$(MOC) -i -o $@ views/cplsequencecontrolview.h

moc_packagetableview.cpp:
	$(MOC) -i -o $@ views/packagetableview.h

moc_cplresourcerect.cpp:
	$(MOC) -i -o $@ views/cplresourcerect.h

moc_cplsegmentrect.cpp:
	$(MOC) -i -o $@ views/cplsegmentrect.h
