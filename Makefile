CFLAGS = -Wall -g -std=c11 -D _POSIX_C_SOURCE=200809L -Werror

all: build

build:
	gcc $(CFLAGS) main.c general.c list.c input.c sender.c receiver.c printer.c -lpthread -o s-talk

run: build
	./s-talk

valgrind: build
	valgrind --leak-check=full ./s-talk

clean:
	rm -f s-talk
