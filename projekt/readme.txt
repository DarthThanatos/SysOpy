rozpakuj gtk+-3... 
w pierwszym oknie: 
	cd gtk+-3... 
w drugim oknie:
	su - 
	yum-deprecated install gtk2-devel
	yum-deprecated install cairo-gobject-devel
	yum-deprecated install gtk3-devel 
w pierwszym oknie:
	./configure
	make
	gcc -o test test.c `pkg-config --cflags --libs gtk+-2.0`
						^farfocle 