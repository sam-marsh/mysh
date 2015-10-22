#include "mysh.h"

/*
   CITS2002 Project 2 2015
   Name(s):             student-name1 (, student-name2)
   Student number(s):   student-number-1 (, student-number-2)
   Date:                date-of-submission
 */

int run_shell(FILE *in)
{
  interactive = (isatty(fileno(in)) && isatty(fileno(stdout)));
  int exit_status = EXIT_SUCCESS;

  while (!feof(in))
  {
    CMDTREE *t = parse_cmdtree(in);
    
    if (t != NULL)
    {
      exit_status = execute_cmdtree(t);
      free_cmdtree(t);
    }

  }

  if (interactive)
  {
    fputc('\n', stdout);
  }

  return exit_status;
}

int main(int argc, char *argv[])
{
//  REMEMBER THE PROGRAM'S NAME (TO REPORT ANY LATER ERROR MESSAGES)
    argv0	= (argv0 = strrchr(argv[0],'/')) ? argv0+1 : argv[0];
    argc--;				// skip 1st command-line argument
    argv++;

//  INITIALIZE THE THREE INTERNAL VARIABLES
    char	*p;

    p		= getenv("HOME");
    HOME	= strdup(p == NULL ? DEFAULT_HOME : p);
    check_allocation(HOME);

    p		= getenv("PATH");
    PATH	= strdup(p == NULL ? DEFAULT_PATH : p);
    check_allocation(PATH);

    p		= getenv("CDPATH");
    CDPATH	= strdup(p == NULL ? DEFAULT_CDPATH : p);
    check_allocation(CDPATH);

    int exit_status = EXIT_SUCCESS;

    if (argc >= 1)
    {
      FILE *fp = fopen(argv[0], "r");

      if (fp == NULL)
      {
        perror(argv[0]);
        exit_status = EXIT_FAILURE;
      }
      else
      {
        exit_status = run_shell(fp);
        fclose(fp);
      }
    }
    else
    {
      exit_status = run_shell(stdin);
    }

    return exit_status;
}
