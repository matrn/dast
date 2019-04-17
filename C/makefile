all: main

libdast.so: watch_functions.c file_functions.c helpers.c
	gcc watch_functions.c file_functions.c helpers.c -Wall -Wextra -Werror -g -pedantic -fPIC -shared -o libdast.so 
	sudo cp libdast.so /usr/lib/libdast.so
	ldconfig -n /usr/lib/

main: main.c libdast.so
	#export LD_LIBRARY_PATH=/home/matej/Programing/C/dast/:$LD_LIBRARY_PATH
	#gcc -g -Wall -o main main.c -L/home/matej/Programing/C/dast/C/ -ldast
	gcc -g -Wall -o main main.c -ldast

clean:
	-rm libdast.so main

remove: clean
	sudo rm /usr/lib/libdast.so
