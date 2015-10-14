#include "mysh.h"

/*
   CITS2002 Project 2 2015
   Name(s):             Samuel Marsh, Liam Reeves	
   Student number(s):   21324325, 21329882
   Date:                TODO		        
 */

#define READ_END    0
#define WRITE_END   1
extern char **environ;

// -------------------------------------------------------------------

//  THIS FUNCTION SHOULD TRAVERSE THE COMMAND-TREE and EXECUTE THE COMMANDS
//  THAT IT HOLDS, RETURNING THE APPROPRIATE EXIT-STATUS.
//  READ print_cmdtree0() IN globals.c TO SEE HOW TO TRAVERSE THE COMMAND-TREE

int execute_cmdtree(CMDTREE *t)
{
    int  exitstatus;
    if (t == NULL) {			// hmmmm, a that's problem
	    exitstatus	= EXIT_FAILURE;
    }
    else {				// normal, exit command
      switch (t->type)
      {
        case N_AND:
          {
            exitstatus = execute_cmdtree(t->left);
            if (exitstatus == EXIT_SUCCESS)
              exitstatus = execute_cmdtree(t->right);
            break;
          }
        case N_BACKGROUND:
          {
            int pid;
            switch (pid = fork())
            {
              case -1:
                perror("fork()");
                exitstatus = EXIT_FAILURE;
                break;
              case 0:
                execute_cmdtree(t->left);
                break;
              default:
                execute_cmdtree(t->right);
                exitstatus = EXIT_SUCCESS;
                break;
            }
            break;
          }
        case N_OR:
          {
            exitstatus = execute_cmdtree(t->left);
            if (exitstatus == EXIT_FAILURE)
                exitstatus = execute_cmdtree(t->right);
            break;
          }
        case N_SEMICOLON:
          {
            execute_cmdtree(t->left);
            exitstatus = execute_cmdtree(t->right);
            break;
          }
        case N_PIPE:
          {
            print_cmdtree0(t->left);
            print_cmdtree0(t->right);
            break;
          }
        case N_SUBSHELL:
          {
            int pid;
            switch (pid = fork())
            {
              case -1:
                perror("fork()");
                exitstatus = EXIT_FAILURE;
                break;
              case 0:
                execute_cmdtree(t->left);
                break;
              default:
                wait(&exitstatus);
                break;
            }
            break;
          }
        case N_COMMAND:
          {
            if (strcmp(t->argv[0], "exit") == 0)
            {
              if (t->argv[1] != NULL)
              {
                exit(atoi(t->argv[1]));
              }
              else
              {
                exit(EXIT_SUCCESS); //TODO
              }
            }
            else if (strcmp(t->argv[0], "cd") == 0)
            {
              if (t->argv[1] != NULL)
              {
                if (strchr(t->argv[1], '/'))
                {
                  chdir(t->argv[1]);
                }
                else
                {
                  char *token = strtok(CDPATH, ":");
                  while (token != NULL)
                  {
                    char full_path[MAXPATHLEN];
                    sprintf(full_path, "%s/%s", token, t->argv[1]);
                    struct stat s;
                    int err = stat(full_path, &s);
                    if (err != -1 && S_ISDIR(s.st_mode))
                    {
                      chdir(full_path);
                      break;
                    }
                    token = strtok(NULL, ":");
                  }
                }
              }
              else
              {
                chdir(HOME);
              }
            }
            else
            {
              bool time = strcmp(t->argv[0], "time") == 0;
              char **argv = t->argv;
              int argc = t->argc;
              if (time)
              {
                ++argv;
                --argc;
              }

              int pid;
              switch (pid = fork())
              {
                case -1:
                  perror("fork()");
                  exitstatus = EXIT_FAILURE;
                  break;
                case 0:
                  {
                    char *file_path;
                    if (strchr(argv[0], '/'))
                    {
                      file_path = strdup(argv[0]);
                    }
                    else 
                    {
                      char *token = strtok(PATH, ":");
                      while (token != NULL)
                      {
                        char full_path[MAXPATHLEN];
                        sprintf(full_path, "%s/%s", token, argv[0]);
                        if (access(full_path, F_OK) != -1)
                        {
                          file_path = strdup(full_path);
                          break;
                        }
                        token = strtok(NULL, ":");
                      }
                    }
                    execve(file_path, argv, environ);
                    exit(EXIT_FAILURE);
                    break;
                  }
                default:
                  if (time)
                  {
                    struct timeval st_start;
                    struct timeval st_end;
                    int start = gettimeofday(&st_start, NULL);
                    if (start == -1)
                    {
                      //error TODO
                    }
                    wait(&exitstatus);
                    int end = gettimeofday(&st_end, NULL);
                    if (end == -1)
                    {
                      //error TODO
                    }
                    fprintf(stderr, "%limsec\n", (st_end.tv_usec - st_start.tv_usec) / 1000);
                  }
                  else
                  {
                    wait(&exitstatus);
                  }
                  break;
              }
            }
            break;
          }
        default:
          fprintf(stderr, "unknown node type\n");
          return EXIT_FAILURE;
      }
    }
    return exitstatus;
}
