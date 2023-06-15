cc = gcc
cflags = -I"C:\Users\SteveChung\AppData\Local\Programs\Python\Python310\include"
ldflags = -L"C:\Users\SteveChung\AppData\Local\Programs\Python\Python310\libs"
libs = -lpython310

pyd:
	$(cc) -DMS_WIN64 -D__WIN32 -fPIC -shared $(cflags) $(ldflags) PyBinding.c -o ClassificatorModule.pyd $(libs)