#ifndef _MYSH_H_
#define _MYSH_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>

#if defined(__linux__)
    extern	char	*strdup(const char *str);
    extern	int	fileno(const FILE *fp);
#endif

//  Written by Chris.McDonald@uwa.edu.au, October 2015

// ----------------------------------------------------------------------

#define	DEFAULT_HOME	"/tmp"
#define	DEFAULT_PATH	"/bin:/usr/bin:/usr/local/bin:."
#define	DEFAULT_CDPATH	".:.."

#define COMMENT_CHAR	'#'	// comment character
#define HOME_CHAR	'~'	// home directory character

#define FORK_FAILURE    -1
#define FORK_CHILD      0

//  ----------------------------------------------------------------------

//  AN enum IN C99 'GENERATES' A SEQUENCE OF UNIQUE, ASCENDING CONSTANTS
typedef enum {
	N_AND = 0,		// as in   cmd1 && cmd2
	N_BACKGROUND,		// as in   cmd1 &
	N_OR,			// as in   cmd1 || cmd2
	N_SEMICOLON,		// as in   cmd1 ;  cmd2
	N_PIPE,			// as in   cmd1 |  cmd2
	N_SUBSHELL,		// as in   ( cmds )
	N_COMMAND		// an actual md node itself
} NODETYPE;


typedef	struct ct {
    NODETYPE	type;		// the type of the node, &&, ||, etc

    int		argc;		// the number of args iff type == N_COMMAND
    char	**argv;		// the NULL terminated argument vector

    char	*infile;	// as in    cmd <  infile
    char	*outfile;	// as in    cmd >  outfile
    bool	append;		// true iff cmd >> outfile

    struct ct	*left, *right;	// pointers to left and right subtrees
} CMDTREE;

extern CMDTREE	*parse_cmdtree(FILE *);		// in parser.c
extern void	free_cmdtree(CMDTREE *);	// in parser.c
extern int	execute_cmdtree(CMDTREE *);	// in execute.c

//internalcmd.c
extern int change_dir(char *);
extern char *locate_file(char *);
extern void print_execution_time(int);
extern int timeval_to_millis(struct timeval * const);
extern int time_command(CMDTREE *, char *, char **, int *);
extern int set_variable(char *ident, char *val);
extern int execute_exit(CMDTREE *t);

//branchcmd.c
extern int execute_semicolon(CMDTREE *);
extern int execute_or(CMDTREE *);
extern int execute_and(CMDTREE *);
extern int execute_background(CMDTREE *);
extern int execute_subshell(CMDTREE *);

extern void execute_script(char *);

//TODO
extern void redirect_io_stream(int, char *, char *);
extern void set_redirection(CMDTREE *);

//pipecmd.c
extern int execute_pipe(CMDTREE *);

//execute.c
extern int execute_command(CMDTREE *, char *, char **);

//mysh.c
extern int run_mysh(void);

/* The global variable HOME points to a directory name stored as a
   character string. This directory name is used to indicate two things:

    The directory used when the  cd  command is requested without arguments.

    The leading pathname of:	~/filename

   The HOME variable is initialized with the value inherited from the
   invoking environment (or DEFAULT_HOME if undefined).
 */

extern	char	*HOME;

/* The global variables PATH and CDPATH point to character strings representing
   colon separated lists of directory names.

   The value of PATH is used to search for executable files when a command
   name does not contain a '/'

   Similarly, CDPATH provides a colon separated list of directory names
   that are used in an attempt to chage the current working directory.
 */

extern	char	*PATH;
extern	char	*CDPATH;

extern	char	*argv0;		// The name of the shell, typically mysh
extern	bool	interactive;	// Boolean indicating if mysh is interactive
extern int last_exit_status;


//  ----------------------------------------------------------------------

//  TWO FUNCTIONS THAT MAY HELP WITH DEBUGGING YOUR CODE.
//  check_allocation(p) ENSURES THAT A POINTER IS NOT NULL, AND
//  print_cmdtree(t)  PRINTS THE REQUESTED COMMAND-TREE

#define	check_allocation(p)	\
	check_allocation0(p, __FILE__, __func__, __LINE__)
extern	void check_allocation0(void *p, char *file, const char *func, int line);

#define	print_cmdtree(t)	\
	printf("called from %s, %s() line %i:\n", __FILE__,__func__,__LINE__); \
	print_cmdtree0(t)
extern	void	print_cmdtree0(CMDTREE *t);

#endif
