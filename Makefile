CC := /usr/bin/gcc

task1:
	$(CC) -ggdb -O0 -o build/search src/main.c -lpthread
