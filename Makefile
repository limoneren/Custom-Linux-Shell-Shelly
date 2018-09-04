all: install install_2 install_3 clean

CC = gcc

obj = shelly search whispers

search: search.c
	$(CC) -o search search.c

install: search
	cp search /usr/bin

whispers: whispers.c
	$(CC) whispers.c -o whispers -lrt

install_2: whispers
	cp whispers /usr/bin

shelly: shelly.c
	$(CC) shelly.c -o shelly

install_3: shelly
	cp shelly /usr/bin

clean:
	rm -f $(obj)

