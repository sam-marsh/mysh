#include "mysh.h"

/*
   CITS2002 Project 2 2015
   Name(s):             Samuel Marsh, Liam Reeves
   Student number(s):   21324325, 21329882
   Date:                TODO
 */

int exitstatus;

int execute_exit(CMDTREE *t)
{
  if (t->argc == 1)
    exit(exitstatus);
  else
    exit(atoi(t->argv[1]));

  fprintf(stderr, "failed to exit process\n");
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

extern int main(int argc, char *argv[]);

void run_script(char *path, char **argv)
{
  FILE *fp = fopen(path, "r");
  if (fp == NULL)
  {
    fprintf(stderr, "%s: not an executable or script file\n", path);
    exit(EXIT_FAILURE);
  }

  dup2(fileno(fp), STDIN_FILENO);
  fclose(fp);
  exit(main(1, &argv0));
}

void set_stream(int file_no, char *path, char *mode)
{
  FILE *fp = fopen(path, mode);
  if (fp == NULL)
  {
    perror("set_file_to_stream");
    exit(EXIT_FAILURE);
  }
  dup2(fileno(fp), file_no);
  fclose(fp);
}

int execute_command(CMDTREE *command, char *path, char **argv)
{
  switch (fork())
  {
    case FORK_FAILURE:
      perror("fork()");
      return EXIT_FAILURE;
    case FORK_CHILD:
      if (command->infile != NULL)
      {
        set_stream(STDIN_FILENO, command->infile, "r");
      }
      if (command->outfile != NULL)
      {
        set_stream(STDOUT_FILENO, command->outfile, command->append ? "a" : "w");
      }
      execv(path, argv);
      //failed to execute - try script
      run_script(path, argv);
      fprintf(stderr, "%s: unrecognised command\n", argv[0]);
      exit(EXIT_FAILURE);
      break;
    default:
    {
      int exit_status;
      while (wait(&exit_status) > 0);
      return exit_status;
    }
  }

  fprintf(stderr, "should never get here\n");
  exit(EXIT_FAILURE);
}

//TODO change all wait calls to while(wait) loops

// -------------------------------------------------------------------

//  THIS FUNCTION SHOULD TRAVERSE THE COMMAND-TREE and EXECUTE THE COMMANDS
//  THAT IT HOLDS, RETURNING THE APPROPRIATE EXIT-STATUS.
//  READ print_cmdtree0() IN globals.c TO SEE HOW TO TRAVERSE THE COMMAND-TREE

int execute_cmdtree(CMDTREE *t)
{
  if (t == NULL)
  {
    return (exitstatus = EXIT_FAILURE);
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
          if (t->argc >= 3)
          {
            exitstatus = set_variable(t->argv[1], t->argv[2]);
          }
          else
          {
            fprintf(stderr, "Usage: set [PATH|CDPATH|HOME] [newval]\n");
            exitstatus = EXIT_FAILURE;
          }
        }
        else if (strcmp(t->argv[0], "exit") == 0)
        {
          exitstatus = execute_exit(t);
        }
        else if (strcmp(t->argv[0], "cd") == 0)
        {
          exitstatus = change_dir(t->argv[1]);
        }
        else
        {
          char **c_argv = t->argv;
          bool time = strcmp(c_argv[0], "time") == 0;
          if (time)
          {
            while (*c_argv != NULL && strcmp(*c_argv, "time") == 0) c_argv++;
            if (*c_argv == NULL)
            {
              fprintf(stderr, "Usage: time [command] [args ...]\n");
              return (exitstatus = EXIT_FAILURE);
            }
          }
          char *file_path = locate_file(c_argv[0]);

          if (file_path == NULL)
          {
            fprintf(stderr, "%s: unrecognised command\n", c_argv[0]);
            return (exitstatus = EXIT_FAILURE);
          }

          if (time)
          {
            print_execution_time(time_command(t, file_path, c_argv, &exitstatus));
            free(file_path);
            return exitstatus;
          }
          else
          {
            exitstatus = execute_command(t, file_path, c_argv);
            free(file_path);
            return exitstatus;
          }
        }
        break;
      }
    default:
      fprintf(stderr, "unknown node type\n");
      return (exitstatus = EXIT_FAILURE);
  }
  return exitstatus;
}
