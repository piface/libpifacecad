PROJECT=pifacecad
SOURCES=src/pifacecad.c
LIBRARY=static
INCPATHS=../libmcp23s17/src/
LIBPATHS=../libmcp23s17/
LDFLAGS=
CFLAGS=-c -Wall
CC=gcc

# ------------ MAGIC BEGINS HERE -------------

# Automatic generation of some important lists
OBJECTS=$(SOURCES:.c=.o)
INCFLAGS=$(foreach TMP,$(INCPATHS),-I$(TMP))
LIBFLAGS=$(foreach TMP,$(LIBPATHS),-L$(TMP))

# Set up the output file names for the different output types
ifeq "$(LIBRARY)" "shared"
    BINARY=lib$(PROJECT).so
    LDFLAGS += -shared
else ifeq "$(LIBRARY)" "static"
    BINARY=lib$(PROJECT).a
else
    BINARY=$(PROJECT)
endif

all: $(SOURCES) $(BINARY)

$(BINARY): $(OBJECTS)
    # Link the object files, or archive into a static library
    ifeq "$(LIBRARY)" "static"
	ar rcs $(BINARY) $(OBJECTS)
    else
	$(CC) $(LIBFLAGS) $(OBJECTS) $(LDFLAGS) -o $@
    endif

.c.o:
	$(CC) $(INCFLAGS) $(CFLAGS) -fPIC $< -o $@

distclean: clean
	rm -f $(BINARY)

clean:
	rm -f $(OBJECTS)

example: example.c
	gcc -o example example.c -Isrc/ -L. -lpifacecad -L../libmcp23s17/ -lmcp23s17

pifacecad: util/pifacecad-cmd.c
	gcc -o pifacecad util/pifacecad-cmd.c -Isrc/ -I../libmcp23s17/src/ -L. -lpifacecad -L../libmcp23s17/ -lmcp23s17

test: test.c
	gcc -o test test.c -Isrc/ -L. -lpifacecad -L../libmcp23s17/ -lmcp23s17
