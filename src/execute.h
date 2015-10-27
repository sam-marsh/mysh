/**
 * CITS2002 Project 2 2015
 * Names:           Samuel Marsh,   Liam Reeves
 * Student numbers: 21324325,       21329882
 * Date:            30/10/2015
 */

#ifndef __EXECUTE_H_
#define __EXECUTE_H_

#include "fileutil.h"

//external functions called in execute.c
extern int execute_and(CMDTREE *);
extern int execute_background(CMDTREE *);
extern int execute_or(CMDTREE *);
extern int execute_semicolon(CMDTREE *);
extern int execute_pipe(CMDTREE *);
extern int execute_subshell(CMDTREE *);

//in internal.c
extern bool execute_internal_command(CMDTREE *, int, char **, int *);

//in mysh.c
extern int run_mysh(void);

#endif
