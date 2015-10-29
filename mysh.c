/**
 * CITS2002 Project 2 2015
 * Names:           Samuel Marsh,   Liam Reeves
 * Student numbers: 21324325,       21329882
 * Date:            30/10/2015
 */

#include "mysh.h"

/**
 * Executes the mysh 'loop', reading commands from stdin.
 *
 * @return  the exit status of the final command executed.
 */
int run_mysh(void)
{
  //initialise the three internal variables - in here and not in main() because
  //the values of internal variables should be reset if this function is run
  //again - e.g. when executing a shell script in a child process.
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

  last_exit_status = EXIT_SUCCESS;
  
  //determine if this shell is interactive
  interactive = (isatty(fileno(stdin)) && isatty(fileno(stdout)));

  while (!feof(stdin))
  {
    CMDTREE	*t = parse_cmdtree(stdin);
    if (t != NULL)
    {
      last_exit_status = execute_cmdtree(t);
      free_cmdtree(t);
    }
  }

  if(interactive)
  {
    fputc('\n', stdout);
  }

  return last_exit_status;
}

int main(int argc, char *argv[])
{
  //  REMEMBER THE PROGRAM'S NAME (TO REPORT ANY LATER ERROR MESSAGES)
  argv0	= (argv0 = strrchr(argv[0],'/')) ? argv0+1 : argv[0];
  argc--;				// skip 1st command-line argument
  argv++;
  return run_mysh();
}
