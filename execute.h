#ifndef __EXECUTE_H_
#define __EXECUTE_H_

#include "mysh.h"

//execution of command tree nodes - various files
extern int execute_and(CMDTREE *);
extern int execute_background(CMDTREE *);
extern int execute_or(CMDTREE *);
extern int execute_semicolon(CMDTREE *);
extern int execute_pipe(CMDTREE *);
extern int execute_subshell(CMDTREE *);

//internal.c
extern bool execute_internal_command(CMDTREE *, int, char **, int *);

//mysh.c
extern int run_mysh(void);

//fileutil.c
extern void redirect_io_stream(int, char *, char *);

#endif
