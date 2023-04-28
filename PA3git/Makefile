CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2 -g
LDFLAGS = -lpthread
TARGETS = ttt ttts

all: $(TARGETS)

ttt: ttt.c
	$(CC) $(CFLAGS) ttt.c -o ttt $(LDFLAGS)

ttts: ttts.c
	$(CC) $(CFLAGS) ttts.c -o ttts $(LDFLAGS)

clean:
	rm -f $(TARGETS)