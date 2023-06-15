cc = gcc
cflags = 
ldflags = 
libs = 	-lRpcrt4

rc:
	windres resource.rc -o resource.o

dll:
	$(cc) -fPIC -shared -c Gen.c -o Gen.o
	$(cc) -shared Gen.o resource.o -o Gen.dll $(libs)