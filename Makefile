CC = gcc
CFLAGS = -Wall -Werror -O0 -ggdb -g -lm
all: reformat main
reformat: 
	$(CC) reformat_with_key.c $(CFLAGS) -o ref	
	./ref a
main: 
	$(CC) main.c $(CFLAGS) -o run
clean:
	rm a run 	
