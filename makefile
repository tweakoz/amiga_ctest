CXXFLAGS = -O0 -Wall -fomit-frame-pointer

M68 = /opt/amiga/dev/
IDIR = -I $(M68)/m68k-amigaos/ndk/include/ -I $(M68)/m68k-amigaos/sys-include/
LDIR = -L $(M68)/lib/gcc-lib/m68k-amigaos/2.95.3/
LIBS = -lm020

all:
	m68k-amigaos-c++ -O4 $(IDIR) main.cpp amictx.cpp -o main.exe -noixemul -msmall-code
#	m68k-amigaos-c++ main.cpp -o main.exe -noixemul -fbaserel32 -m68020 -msmall-code	
