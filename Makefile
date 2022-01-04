
CC=gcc
SRC=*.c
EXE=gravity2D
STD=-std=c99
WFLAGS=-Wall -Wextra
OPT=-O2
IDIR=-I.
LDIR=lib
SLIBS=glee fract photon
DLIBS=glfw

LSTATIC=$(patsubst %,lib%.a,$(SLIBS))
LPATHS=$(patsubst %,$(LDIR)/%,$(LSTATIC))
IDIR += $(patsubst %,-I%,$(SLIBS))
LFLAGS=$(patsubst %,-L%,$(LDIR))
LFLAGS += $(patsubst %,-l%,$(SLIBS))
LFLAGS += $(patsubst %,-l%,$(DLIBS))

OS=$(shell uname)

ifeq ($(OS),Darwin)
	LFLAGS += -framework OpenGL -mmacos-version-min=10.9
else
	LFLAGS += -lm -lGL -lGLEW
endif

CFLAGS=$(STD) $(WFLAGS) $(OPT) $(IDIR) $(LFLAGS)

$(EXE): $(SRC) $(LPATHS)
	$(CC) -o $@ $^ $(CFLAGS)

$(LDIR):
	mkdir $@

$(LDIR)%.a: % $(LDIR)
	cd $< && make && mv $@ ../

$(LDIR)/$(LDIR)%.a: $(LDIR)%.a $(LDIR)
	mv $< $(LDIR)

clean:
	rm -r $(LDIR) && rm $(EXE)
