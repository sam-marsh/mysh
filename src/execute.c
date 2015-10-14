#include "mysh.h"

/*
   CITS2002 Project 2 2015
   Name(s):             Samuel Marsh, Liam Reeves	
   Student number(s):   21324325, 21329882
   Date:                TODO		        
 */

#define FD_READ     0
#define FD_WRITE    1

extern char **environ;  //TODO - do we actually need this?
int exitstatus;

int execute_and(CMDTREE *t)
{
  int exit_status = execute_cmdtree(t->left);

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
    case -1:
      //failed to fork
      perror("fork()");
      return EXIT_FAILURE;
    case 0:
      //child process - do the work in parallel
      //ignore result
      exit(execute_cmdtree(t->left));
      break;
    default:
      //parent process - continue down the right of the tree
      return execute_cmdtree(t->right);
  }
}

int execute_or(CMDTREE *t)
{
  int exit_status = execute_cmdtree(t->left);

  if (exit_status == EXIT_FAILURE)
  {
    return execute_cmdtree(t->right);
  }

  return exit_status;
}

int execute_semicolon(CMDTREE *t)
{
  execute_cmdtree(t->left);
  return execute_cmdtree(t->right);
}

int execute_pipe(CMDTREE *t)
{
  int fd[2];
  pipe(fd);

  switch (fork())
  {
    case -1:
      {
        perror("fork()");
        return EXIT_FAILURE;
      }
    case 0:
      {
        dup2(fd[FD_WRITE], STDOUT_FILENO);
        close(fd[FD_READ]);
        close(fd[FD_WRITE]);
        exit(execute_cmdtree(t->left));
        break;
      }
    default:
      {
        switch (fork())
        {
          case -1:
            {
              perror("fork()");
              return EXIT_FAILURE;
            }
          case 0:
            {
              dup2(fd[FD_READ], STDIN_FILENO);
              close(fd[FD_READ]);
              close(fd[FD_WRITE]);
              exit(execute_cmdtree(t->right));
              break;
            }
          default:
            {
              int exit_status;
              wait(&exit_status);
              return exit_status;
            }
        }
      }
  }
}

int execute_subshell(CMDTREE *t)
{
  int exit_status;

  switch (fork())
  {
    case -1:
      perror("fork()");
      return EXIT_FAILURE;
    case 0:
      execute_cmdtree(t->left);
      break;
    default:
      wait(&exit_status);
      break;
  }
  
  return exit_status;
}

int execute_exit(CMDTREE *t)
{
  if (t->argc == 1)
  {
    exit(exitstatus);
  }
  else
  {
    exit(atoi(t->argv[1]));
  }

  return EXIT_FAILURE;
}

int set_variable(char *ident, char *val)
{
  struct {
    char *identifier;
    char **var_ptr;
  } vars[] = {
    {"PATH", &PATH},
    {"HOME", &HOME},
    {"CDPATH", &CDPATH}
  };
  int n = sizeof(vars) / sizeof(vars[0]);

  for (int i = 0; i < n; ++i)
  {
    if (strcmp(ident, vars[i].identifier) == 0)
    {
      *vars[i].var_ptr = val;
      return EXIT_SUCCESS;
    }
  }

  fprintf(stderr, "unrecognized variable: '%s'\n", ident);
  return EXIT_FAILURE;
}

int execute_command(CMDTREE *t)
{

  return -1;
}


// -------------------------------------------------------------------

//  THIS FUNCTION SHOULD TRAVERSE THE COMMAND-TREE and EXECUTE THE COMMANDS
//  THAT IT HOLDS, RETURNING THE APPROPRIATE EXIT-STATUS.
//  READ print_cmdtree0() IN globals.c TO SEE HOW TO TRAVERSE THE COMMAND-TREE

int execute_cmdtree(CMDTREE *t)
{
  if (t == NULL)
  {
    return EXIT_FAILURE;
  }

  switch (t->type)
  {
    case N_AND:
      exitstatus = execute_and(t);
      break;
    case N_BACKGROUND:
      exitstatus = execute_background(t);
      break;
    case N_OR:
      exitstatus = execute_or(t);
      break;
    case N_SEMICOLON:
      exitstatus = execute_semicolon(t);
      break;
    case N_PIPE:
      exitstatus = execute_pipe(t);
      break;
    case N_SUBSHELL:
      exitstatus = execute_subshell(t);
      break;
    case N_COMMAND:
      {
        if (strcmp(t->argv[0], "set") == 0)
        {
          //TODO - ensure correct types and numbers of arguments
          exitstatus = set_variable(t->argv[1], t->argv[2]);
        }
        else if (strcmp(t->argv[0], "exit") == 0)
        {
          exitstatus = execute_exit(t);
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
                if (t->infile != NULL)
                {
                  FILE *fp = fopen(t->infile, "r");
                  if (fp == NULL) 
                  { 
                    //error (TODO) 
                  }
                  dup2(fileno(fp), STDIN_FILENO);
                  fclose(fp);
                }
                if (t->outfile != NULL)
                {
                  FILE *fp = fopen(t->outfile, t->append ? "a" : "w");
                  if (fp == NULL)
                  {
                    //error (TODO)
                  }
                  dup2(fileno(fp), STDOUT_FILENO);
                  fclose(fp);
                }
                if (execve(file_path, argv, environ) == -1)
                {
                  FILE *fp = fopen(file_path, "r");
                  if (fp == NULL)
                  {
                    //error (TODO)
                  }

                  int pid;

                  switch (pid = fork())
                  {
                    case -1:
                      perror("fork()");
                      exitstatus = EXIT_FAILURE;
                      break;
                    case 0:
                      dup2(fileno(fp), STDIN_FILENO);
                      break;
                    default:
                      wait(&exitstatus);
                      break;
                  }

                  fclose(fp);
                }
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
  return exitstatus;
}
