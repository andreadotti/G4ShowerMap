CC=clang++
LINKER=$(CC)
OPTFLAGS=
CFLAGS=-DUNITTESTING

all: test

test: test.o G4ShowerMap.o
	$(LINKER) $(OPTFLAGS) -o test test.o G4ShowerMap.o


.SUFFIXES:
.SUFFIXES: .cc .o

.cc.o:
	$(CC) $(OPTFLAGS) $(CFLAGS) -c $<

clean:
	rm -f test test.o G4ShowerMap.o