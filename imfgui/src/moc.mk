
# would be great to figure out how to "automate" this the make way
moc_imfpackageview.cpp:  
	$(MOC) -i -o $@ imfpackageview.hpp
