
CC=gcc
CFLAGS=-w -std=gnu99 -Werror -D _POSIX_C_SROUCE=200809L
PROG=UnixLs
OBJS= un.o

UnixLs: $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS)

un.o: un.c
	$(CC) $(CFLAGS) -c un.c

clean:
	rm *.o $(PROG)
