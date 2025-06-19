CC = gcc

WORDLEN ?= 4

SRC = main.c address_dumper.c disable_aslr.c test.c

EXE1 = address_dumper
EXE2 = disable_aslr
EXE3 = test

all: $(EXE1) $(EXE2) $(EXE3)

$(EXE1): main.c
ifdef IS32
	$(CC) $< address_dumper.c -o $@ -DWORDLEN=$(WORDLEN) -DIS32
else
	$(CC) $< address_dumper.c -o $@ -DWORDLEN=$(WORDLEN)
endif

$(EXE2): disable_aslr.c
	$(CC) $< -o $@

$(EXE3): test.c
	$(CC) $< -o $@

run: all
	./$(EXE1) ./$(EXE2) ./$(EXE3)

clean:
	rm -f $(EXE1) $(EXE2) $(EXE3)
