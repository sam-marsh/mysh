#include "mysh.h"

#define FD_READ     0
#define FD_WRITE    1

int execute_pipe(CMDTREE *t)
{
  switch (fork())
  {
    case FORK_FAILURE:
      {
        perror("fork()");
        return EXIT_FAILURE;
      }
    case FORK_CHILD:
      {
        int fd[2];

        //index 0 (FD_READ) is for reading, index 1 (FD_WRITE) is for writing
        if (pipe(fd) == -1)
        {
          perror("execute_pipe()");
          return EXIT_FAILURE;
        }

        switch (fork())
        {
          case FORK_FAILURE:
            {
              perror("fork()");
              return EXIT_FAILURE;
            }
          case FORK_CHILD:
            {
              //set stdout to write to the pipe
              if (dup2(fd[FD_WRITE], fileno(stdout)) == -1)
              {
                perror("dup2()");
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
                perror("dup2()");
                exit(EXIT_FAILURE);
              }

              //close both piped files - we have stdin now which is all we need
              close(fd[FD_READ]);
              close(fd[FD_WRITE]);

              //execute and then exit the child process
              int result = execute_cmdtree(t->right);

              //wait for child if it's not finished yet
              //throw away result of left command tree
              wait(NULL);

              exit(result);
              break;
            }
        }
      }
    default:
      {
        int exit_status;
        //we are in the original shell - just wait for the children to finish
        while (wait(&exit_status) > 0);
        return exit_status;
      }
  }

  //we are only here if one of the children somehow failed to exit, (probably) won't ever happen
  fprintf(stderr, "failed to exit process\n");
  return EXIT_FAILURE;
}
