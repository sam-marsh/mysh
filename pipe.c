/**
 * CITS2002 Project 2 2015
 * Names:           Samuel Marsh,   Liam Reeves
 * Student numbers: 21324325,       21329882
 * Date:            30/10/2015
 */

#include "mysh.h"

#define FD_READ     0
#define FD_WRITE    1

/**
 * Executes a command tree node, passing one child's stdout to the other's stdin
 * (piping the execution).
 *
 * @param  t the command tree, with root node of type N_PIPE
 * @return   the exit status of the right hand side of the tree (the process
 *           which is receiving stdin from the other side's stdout)
 */
int execute_pipe(CMDTREE *t)
{
  switch (fork())
  {
    case FORK_FAILURE:
      {
        MYSH_PERROR("execute_pipe");
        return EXIT_FAILURE;
      }
    case FORK_CHILD:
      {
        int fd[2];

        //index 0 (FD_READ) is for reading, index 1 (FD_WRITE) is for writing
        if (pipe(fd) == -1)
        {
          MYSH_PERROR("execute_pipe");
          return EXIT_FAILURE;
        }

        switch (fork())
        {
          case FORK_FAILURE:
            {
              MYSH_PERROR("execute_pipe");
              return EXIT_FAILURE;
            }
          case FORK_CHILD:
            {
              //set stdout to write to the pipe
              if (dup2(fd[FD_WRITE], fileno(stdout)) == -1)
              {
                MYSH_PERROR("execute_pipe");
                exit(EXIT_FAILURE);
              }

              //close both piped files - we have stdout now which is all we need
              close(fd[FD_READ]);
              close(fd[FD_WRITE]);

              //execute and then exit the child process
              exit(execute_cmdtree(t->left));
              break;
            }
          default:
            {
              //set stdin to read from the pipe
              if (dup2(fd[FD_READ], fileno(stdin)) == -1)
              {
                MYSH_PERROR("execute_pipe");
                exit(EXIT_FAILURE);
              }

              //close both piped files - we have stdin now which is all we need
              close(fd[FD_READ]);
              close(fd[FD_WRITE]);

              //execute our part
              int exit_status = execute_cmdtree(t->right);

              //wait for the other side to finish if not already done - our side
              //should always exit last as per what bash does
              while (wait(NULL) > 0);

              //exit with our exit status
              exit(exit_status);
              break;
            }
        }
      }
    default:
      {
        int exit_status;
        //we are in the original shell - just wait for the children to finish
        while (wait(&exit_status) > 0);
        exit_status = WEXITSTATUS(exit_status);
        return exit_status;
      }
  }

  //we are only here if one of the children somehow failed to exit, (probably) won't ever happen
  fprintf(stderr, "failed to exit process\n");
  return EXIT_FAILURE;
}
