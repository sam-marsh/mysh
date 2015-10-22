#include "mysh.h"

int execute_semicolon(CMDTREE *t)
{
  //execute in order, no matter the result of the left side
  execute_cmdtree(t->left);
  return execute_cmdtree(t->right);
}

int execute_or(CMDTREE *t)
{
  //always execute left hand side
  int exit_status = execute_cmdtree(t->left);

  //short circuit upon failure
  if (exit_status != EXIT_SUCCESS)
  {
    return execute_cmdtree(t->right);
  }

  return exit_status;
}

int execute_and(CMDTREE *t)
{
  //always execute left hand side
  int exit_status = execute_cmdtree(t->left);

  //short circuit upon success
  if (exit_status == EXIT_SUCCESS)
  {
    return execute_cmdtree(t->right);
  }

  return exit_status;
}

int execute_background(CMDTREE *t)
{
  //fork process - throw away return value, don't need it
  switch (fork())
  {
    case FORK_FAILURE:
      //failed to fork
      perror("fork()");
      return EXIT_FAILURE;
    case FORK_CHILD:
      //child process - do the work in parallel
      //ignore result
      exit(execute_cmdtree(t->left));
      break;
    default:
      //parent process - continue down the right of the tree
      return execute_cmdtree(t->right);
  }

  fprintf(stderr, "failed to exit child process\n");
  return EXIT_FAILURE;
}

int execute_subshell(CMDTREE *t)
{
  int exit_status;

  switch (fork())
  {
    case FORK_FAILURE:
      perror("fork()");
      return EXIT_FAILURE;
    case FORK_CHILD:
      {
        if (t->infile != NULL)
        {
          FILE *fp = fopen(t->infile, "r");
          if (fp == NULL)
          {
            perror(t->infile);
            exit(EXIT_FAILURE);
          }
          dup2(fileno(fp), STDIN_FILENO);
          fclose(fp);
        }
        if (t->outfile != NULL)
        {
          FILE *fp = fopen(t->outfile, t->append ? "a" : "w");
          if (fp == NULL)
          {
            perror(t->outfile);
            exit(EXIT_FAILURE);
          }
          dup2(fileno(fp), STDOUT_FILENO);
          fclose(fp);
        }
        exit(execute_cmdtree(t->left));
        break;
      }
    default:
      while (wait(&exit_status) > 0);
      exit_status = WEXITSTATUS(exit_status);
      break;
  }

  return exit_status;
}
