TMPFILES = lex.c parse.c
MODULES = main parse
OBJECTS = $(MODULES:=.o)
CC = gcc -g -Wall
LINK = $(CC)
LIBS = -lreadline

shell: $(OBJECTS)
	$(LINK) $(OBJECTS) -o $@ $(LIBS)

%.o: %.c
	$(CC) -c $<

parse.o: parse.c lex.c
parse.c: parse.y global.h
	bison parse.y -o $@
lex.c: lex.l
	flex -o$@ lex.l

clean: 
	rm -f shell $(OBJECTS) $(TMPFILES) *.o
