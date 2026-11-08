CC=gcc
FEATURES += -D_TIME	# Time setting
FEATURES += -D_FLAGS	# Adds support for flags
#FEATURES += -D_FORKLESS	# If your libc doesn't have the fork() syscall.
#FEATURES += -D_CGI	# Adds support for CGI (cgi-bin)
CFLAGS= -ansi -O3 -Wall -Wextra -pedantic $(FEATURES)
LDFLAGS= -flto

all: eshttpd

eshttpd: eshttpd.c
	$(CC) $(CFLAGS) eshttpd.c -o eshttpd $(LDFLAGS)

clean:
	rm -fv eshttpd
