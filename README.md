libpifacecad
============
A simple library for controlling PiFace Control and Display. See `example.c`
for example usage.

Use
---
Build the library:

    $ make

This creates the library `libpifacecad.a`. Include it in your project with:

    $ gcc -o example example.c -Isrc/ -L. -lpifacecad -L../libmcp23s17/ -lmcp23s17

`-Isrc/` is for including the header file. `-L. -lpifacecad` is for
including the library file (from this directory).
`-L../libmcp23s17/ -lmcp23s17` is for including the mcp23s17 library.

Todo
----
Feel free to contribute!

- Finish functions
- Sort out display control stuff
- mcp23s17_fd checking for each function. Print to stderr if not found.
- Documentation (Doxygen? Good comments?)
- Debian install
- Interrupts (using libmcp23s17 interrupts?)
