NAME=test
CFLAGS=-O0 -g3 -Wall -c -fmessage-length=0 -pthread
GTKFLAGS=`pkg-config --cflags --libs gtk+-2.0 gthread-2.0`
SRCS=src/core.c src/about.c src/fullscreen.c src/preferences.c src/trayicon.c
OSRCS=core.o about.o fullscreen.o preferences.o trayicon.o
CC=gcc
 
# top-level rule to create the program.
all: main
 
# compiling the source file.
main: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) $(GTKFLAGS)
 
run:
	$(CC) $(OSRCS) $(GTKFLAGS)

exec:
	$(CC) $(GTKFLAGS) $(GTKFLAGS)$(SRCS) -o test

# cleaning everything that can be automatically recreated with "make".
clean:
	rm -f *.o

	
demo:
	rm -rf *.o
	rm -rf *.d
	rm breaker
	gcc $(GTKFLAGS) -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP $(SRCS)
	gcc  -o "work-break" $(OSRCS)   $(GTKFLAGS)
	rm -rf *.o
	rm -rf *.d
	#./breaker > /dev/null 2>&1