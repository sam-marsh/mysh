# Author:	Samuel Marsh
# Date:		30/10/2015 

PROJECT			=	mysh

# everything depends on mysh.h
HEADERS			=	$(PROJECT).h

# dynamically generate a list of the required object files
SOURCES			=	$(wildcard *.c)
OBJECTS			=	$(SOURCES:%.c=%.o)

COMPILER		=	cc -std=c99
CFLAGS			=	-Wall -pedantic -Werror

$(PROJECT) : $(OBJECTS)
	$(COMPILER) $(CFLAGS) -o $(PROJECT) $(OBJECTS)

# note: also added $(wildcard %.h) to support recompilation of a file if the
# header of the same name exists and has changed - e.g. execute.h
# changes -> update execute.o
%.o : %.c $(wildcard %.h) $(HEADERS)
	$(COMPILER) $(CFLAGS) -c $<

clean:
	rm -f $(PROJECT) $(OBJECTS)
