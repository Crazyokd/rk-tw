CFLAGS=-g -Wall -W -std=c99
LDFLAGS=-L. -Wl,-R. -lrktw -static

all: librktw.a librktw.so main

main: main.o
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

librktw.so: rk-tw.c
	$(CC) -fPIC -shared $^ -o $@ $(CFLAGS)

librktw.a: rk-tw.o
	$(AR) rcs $@ $^

.PHONY: clean

clean:
	rm -f *.o librktw.a librktw.so main
