#include "mysh.h"

/**
 * Executes a background command node - the left of the tree is executed in a
 * child process, while the right hand side ('rest of') the tree is executed as
 * normal in the parent process.
 *
 * @param  t the command tree, where the root node is of type BACKGROUND
 * @return   the exit status of the right child of the command tree
 */
int execute_background(CMDTREE *t)
{
  //fork process - throw away return value, don't need it
  switch (fork())
  {
    case FORK_FAILURE:
      //failed to fork
      MYSH_PERROR("execute_background");
      return EXIT_FAILURE;
    case FORK_CHILD:
    {
      //child process - do the work in parallel
      //ignore result
      int result = EXIT_SUCCESS;
      if (t->left != NULL)
      {
        result = execute_cmdtree(t->left);
      }
      exit(result);
      break;
    }
    default:
      //parent process - continue down the right of the tree
      if (t->right != NULL)
      {
        return execute_cmdtree(t->right);
      }
      return EXIT_SUCCESS;
  }

  fprintf(stderr, "%s: internal error: failed to exit child process\n", argv0);
  return EXIT_FAILURE;
}
