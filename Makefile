NAME=work-break
CFLAGS=-O0 -g3 -Wall -c -fmessage-length=0 -pthread
GTKFLAGS=`pkg-config --cflags --libs gtk+-2.0 gthread-2.0`
SRCS=src/core.c src/about.c src/fullscreen.c src/preferences.c src/trayicon.c
OSRCS=core.o about.o fullscreen.o preferences.o trayicon.o
CC=gcc
 
# top-level rule to create the program.
all: clean main run clean 
	./work-break
 
# compiling the source file.
main: $(SRCS)
	$(CC) $(GTKFLAGS) -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP $(SRCS)
 
run:
	$(CC) -o "$(NAME)" $(OSRCS) $(GTKFLAGS)

exec:
	./$(NAME)

# cleaning everything that can be automatically recreated with "make".
clean:
	rm -rf *.o
	rm -rf *.d