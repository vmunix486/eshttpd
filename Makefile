##########################
# User tunable variables #
##########################
CC=cc
# If capable, you can put -Ofast and -flto for more speed increase, or -Os for a smaller executable.
CFLAGS= -std=c89
# If you want to learn all the options for compilation, look in OPTIONS.md
DEFS= -D_TIME -D_PENGER -D_FLAGS -D_NO_REDEFINITION -D_FORKLESS
INCLUDES=
RM=rm
RMFLAGS=-f
CURL=curl

##########################################################
# You shouldn't need to change anything after this point #
##########################################################

all: eshttpd.c
	$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) eshttpd.c -o eshttpd

test:
	sudo ./eshttpd
	$(CURL) -v localhost
	sudo killall eshttpd

clean:
	$(RM) $(RMFLAGS) eshttpd
