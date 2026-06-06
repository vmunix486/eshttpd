# User tunable variables
CC=cc
CFLAGS= -O3 -std=c89
INCLUDES=-I.
RM=rm
RMFLAGS=-f
# If capable, you can put -flto for more speed increase, or -Os if you smaller executables

all: httpd.c
	$(CC) $(CFLAGS) $(INCLUDES) httpd.c -o httpd

clean:
	$(RM) $(RMFLAGS) httpd
