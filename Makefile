all: shell

shell:  shell.o main.o
	gcc -o shell main.o shell.o

shell.o: shell.c
	gcc -o shell.o -c shell.c -W -Wall

main.o: main.c
	gcc -o main.o -c main.c -W -Wall
