CFLAGS = -O3 -Wall -std=c99
DEBUG_FLAGS = -g -O0 -Wall -std=c99

all: kupyna-reference

kupyna-reference: kupyna.c kupyna.h main.c makefile tables.c tables.h perf.h
	gcc $(CFLAGS) kupyna.c main.c tables.c -o kupyna-reference

debug: kupyna.c kupyna.h main.c makefile tables.c tables.h perf.h
	gcc $(DEBUG_FLAGS) kupyna.c main.c tables.c -o kupyna-debug

run:kupyna-reference
	./kupyna-reference
