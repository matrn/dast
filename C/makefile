CC=gcc
CFLAGS=-Wall -Wextra -pedantic -g

all: main

libdast.so: lib/*.c lib/dast.h # lib/watch_functions.c lib/file_functions.c lib/helpers.c
	#compile everything to so library
	$(CC) lib/*.c -fPIC -shared -o libdast.so $(CFLAGS)

	sudo cp lib/dast.h /usr/include/dast.h   #copy header file
	sudo cp libdast.so /usr/lib/libdast.so   #copy library

	ldconfig -n /usr/lib/   #run ldconfig

main: main.c libdast.so
	#export LD_LIBRARY_PATH=/home/matej/Programing/C/dast/:$LD_LIBRARY_PATH
	#gcc -g -Wall -o main main.c -L/home/matej/Programing/C/dast/C/ -ldast
	$(CC) -o main main.c -ldast $(CFLAGS)   #compile main code - example

clean:
	-rm libdast.so main

remove: clean
	sudo rm /usr/lib/libdast.so
	sudo rm /usr/include/dast.h
