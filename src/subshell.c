/**
 * CITS2002 Project 2 2015
 * Names:           Samuel Marsh,   Liam Reeves
 * Student numbers: 21324325,       21329882
 * Date:            30/10/2015
 */

#include "mysh.h"
#include "fileutil.h"

/**
 * Executes the subshell command node - executes the command tree inside a
 * subshell (a duplicate copy of the mysh program).
 *
 * @param  t the command tree, with the root node being of type N_SUBSHELL.
 * @return   the exit status of the subshell execution
 */
int execute_subshell(CMDTREE *t)
{
  int exit_status;

  //fork and execute the tree inside the child process
  switch (fork())
  {
    case FORK_FAILURE:
      MYSH_PERROR("execute_subshell");
      return EXIT_FAILURE;
    case FORK_CHILD:
      {
        //redirect standard input and output for this subshell if required
        set_redirection(t);
        //a subshell node only has a left child - execute this and exit
        exit(execute_cmdtree(t->left));
        break;
      }
    default:
      //in the parent, simply wait for the subshell to finish and then return
      //the exit status of the child process
      while (wait(&exit_status) > 0);
      exit_status = WEXITSTATUS(exit_status);
      break;
  }

  return exit_status;
}
