#G221210092 ASIM BINGOL - https://github.com/ASIMBINGOL
#Y245060003 MESUTCAN OZKAN - https://github.com/CaniReese
#B231210061 METEHAN OZTURK - https://github.com/metepukkada
#G221210008 YUSUF TAHA EZGIN - https://github.com/yusuftahaezgin
#G221210014 AHMET SEVLI - https://github.com/Ahmet-Sevli1

all: shell.o
	gcc -o shell shell.o
	./shell

shell.o: shell.c shell.h
	gcc -c shell.c

clean:
	rm -f*.o shell