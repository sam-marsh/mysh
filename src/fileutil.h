/**
 * CITS2002 Project 2 2015
 * Names:           Samuel Marsh,   Liam Reeves
 * Student numbers: 21324325,       21329882
 * Date:            30/10/2015
 */

#ifndef __FILEUTIL_H_
#define __FILEUTIL_H_

#include "mysh.h"

extern char *locate_file(char *, char *);
extern void set_redirection(CMDTREE *);
extern void redirect_io_stream(int, char *, char *);

#endif
