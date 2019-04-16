all: code 

lib: watch_functions.c file_functions.c helpers.c
	gcc watch_functions.c file_functions.c helpers.c -Wall -Wextra -Werror -g -pedantic -fPIC -shared -o libdast.so 

code: main.c lib
	export LD_LIBRARY_PATH=/home/matej/Programing/C/dast/:$LD_LIBRARY_PATH
	gcc -g -Wall -o main main.c -L/home/matej/Programing/C/dast/C/ -ldast

clean:
	rm libdast.so main
