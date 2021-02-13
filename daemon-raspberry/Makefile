upsd:	slib.o plog.o upsd.o
	gcc slib.o plog.o upsd.o -o upsd
slib.o:	slib.c slib.h
	gcc -c slib.c
plog.o:	plog.c plog.h
	gcc -c plog.c
upsd.o:	upsd.c plog.h
	gcc -c upsd.c
clean:
	rm -f upsd upsd.o plog.o slib.o
install:
	cp upsd /usr/local/bin/upsd
uninstall:
	rm -f /usr/local/bin/upsd
