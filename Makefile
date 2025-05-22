CC := /usr/bin/gcc

all: task1 task2

task1:
	$(CC) -ggdb -O0 -o build/search_first src/task1.c -lpthread

task2:
	$(CC) -ggdb -O0 -o build/search_all src/task2.c -lpthread
