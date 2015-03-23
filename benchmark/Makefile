.PHONY: clean run

CC=gcc
CFLAGS=-Wall -pedantic -std=c99
SOURCES=queue_test.c cg_queue.c fg_queue.c
BIN=./bin/
EXECUTABLE=$(BIN)queue

default: clean run

$(EXECUTABLE): $(SOURCES) $(BIN)
	@$(CC) $(CFLAGS) -o $(EXECUTABLE) $(SOURCES) -pthread

run: $(EXECUTABLE)
	@./$(EXECUTABLE)

clean:
	@rm -rf $(BIN)

$(BIN):
	@mkdir -p bin
