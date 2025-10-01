CC=gcc
CFLAGS=-Iinclude -std=c99 -O2 -Wall
SRCDIR=src
OBJ=$(SRCDIR)/main.o $(SRCDIR)/fileio.o $(SRCDIR)/heap.o $(SRCDIR)/hashmap.o $(SRCDIR)/graph.o $(SRCDIR)/allocation.o $(SRCDIR)/report.o
all:build/resq
build/resq:$(OBJ)|build
	$(CC) $(CFLAGS) -o $@ $(OBJ)
build:
	mkdir -p build
$(SRCDIR)/main.o:$(SRCDIR)/main.c
	$(CC) $(CFLAGS) -c $< -o $@
$(SRCDIR)/fileio.o:$(SRCDIR)/fileio.c
	$(CC) $(CFLAGS) -c $< -o $@
$(SRCDIR)/heap.o:$(SRCDIR)/heap.c
	$(CC) $(CFLAGS) -c $< -o $@
$(SRCDIR)/hashmap.o:$(SRCDIR)/hashmap.c
	$(CC) $(CFLAGS) -c $< -o $@
$(SRCDIR)/graph.o:$(SRCDIR)/graph.c
	$(CC) $(CFLAGS) -c $< -o $@
$(SRCDIR)/allocation.o:$(SRCDIR)/allocation.c
	$(CC) $(CFLAGS) -c $< -o $@
$(SRCDIR)/report.o:$(SRCDIR)/report.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf build $(SRCDIR)/*.o
