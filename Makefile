# CITS2002 Project 2 2015
# Names:						Samuel Marsh,	Liam Reeves
# Student numbers:	21324325,			21329882
# Date:							30/10/2015

PROJECT		=	mysh

# at the moment, a single header file is used in all source files
HEADERS		=	$(PROJECT).h

# dynamically generate a list of the required object files
SOURCES		=	$(wildcard *.c)
OBJECTS		=	$(SOURCES:%.c=%.o)

C99			=	cc -std=c99
CFLAGS		=	-Wall -pedantic -Werror

$(PROJECT) : $(OBJECTS)
	$(C99) $(CFLAGS) -o $(PROJECT) $(OBJECTS)

%.o : %.c $(HEADERS)
	$(C99) $(CFLAGS) -c $<

clean:
	rm -f $(PROJECT) $(OBJECTS)
