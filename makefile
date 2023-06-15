cc = gcc
cflags = 
ldflags = 
libs =  

dll:
	$(cc) -fPIC -shared md5.c -o md5.dll