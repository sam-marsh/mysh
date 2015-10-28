# CITS2002 Project 2 2015
# Names:						Samuel Marsh,	Liam Reeves
# Student numbers:	21324325,			21329882
# Date:							30/10/2015

PROJECT			=	mysh

# everything depends on mysh.h
HEADERS			=	$(PROJECT).h

# dynamically generate a list of the required object files
SOURCES			=	$(wildcard *.c)
OBJECTS			=	$(SOURCES:%.c=%.o)

COMPILER		=	cc
CFLAGS			=	-std=c99 -Wall -pedantic -Werror

$(PROJECT) : $(OBJECTS)
	$(COMPILER) $(CFLAGS) -o $(PROJECT) $(OBJECTS)

# note: also added %.h to support recompilation of a file if the header of the
# same name changes - e.g. execute.h changes -> update execute.o
%.o : %.c %.h $(HEADERS)
	$(COMPILER) $(CFLAGS) -c $<

clean:
	rm -f $(PROJECT) $(OBJECTS)
