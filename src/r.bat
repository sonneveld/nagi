gcc   -IC:\mingw\mingw32\include\SDL -Wall -fwritable-strings -W -Winline -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual %1.c %2 -o %1.exe -lmingw32 -lSDLmain -lSDL -mwindows -mconsole
