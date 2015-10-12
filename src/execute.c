#include "mysh.h"

/*
   CITS2002 Project 2 2015
   Name(s):             Samuel Marsh, Liam Reeves	
   Student number(s):   21324325, 21329882
   Date:                TODO		        
 */

// -------------------------------------------------------------------

//  THIS FUNCTION SHOULD TRAVERSE THE COMMAND-TREE and EXECUTE THE COMMANDS
//  THAT IT HOLDS, RETURNING THE APPROPRIATE EXIT-STATUS.
//  READ print_cmdtree0() IN globals.c TO SEE HOW TO TRAVERSE THE COMMAND-TREE

int execute_cmdtree(CMDTREE *t)
{
    int  exitstatus;

    if (t == NULL) {			// hmmmm, a that's problem
	    exitstatus	= EXIT_FAILURE;
    }
    else {				// normal, exit commands
	    exitstatus	= EXIT_SUCCESS;
    }

    return exitstatus;
}
