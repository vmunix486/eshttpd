# User tunable variables
CC=cc
CFLAGS= -O3 -std=c89
DEFS= -D_TIME -D_FLAGS -D_LINUX -D_PENGER
INCLUDES=
RM=rm
RMFLAGS=-f
CURL=curl
# If capable, you can put -flto for more speed increase, or -Os if you smaller executables

all: eshttpd.c
	$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) eshttpd.c -o eshttpd

test:
	sudo ./eshttpd
	$(CURL) -v localhost
	sudo killall eshttpd

clean:
	$(RM) $(RMFLAGS) eshttpd
