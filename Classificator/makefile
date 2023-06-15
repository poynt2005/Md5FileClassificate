cc = gcc
cflags = 
ldflags = 
libs = -lShlwapi

dll:
	$(cc) -fPIC -shared Classificator.c -o Classificator.dll $(libs)

test:
	$(cc)  testBuff.c -o testBuff.exe