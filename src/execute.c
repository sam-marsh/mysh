#include "mysh.h"

int execute_command(CMDTREE *command, char *path, char **argv)
{
  switch (fork())
  {
    case FORK_FAILURE:
      MYSH_PERROR("execute_command");
      return EXIT_FAILURE;
    case FORK_CHILD:
      set_redirection(command);
      execv(path, argv);
      //failed to execute for whatever reason - try script
      execute_script(path);
      fprintf(stderr, "%s: %s: failed to execute as program or shell script\n", argv0, argv[0]);
      exit(EXIT_FAILURE);
      break;
    default:
    {
      int exit_status;
      while (wait(&exit_status) > 0);
      exit_status = WEXITSTATUS(exit_status);
      return exit_status;
    }
  }

  fprintf(stderr, "%s: internal error: should never get here\n", argv0);
  exit(EXIT_FAILURE);
}

int direct_command(CMDTREE *t)
{
  if (strcmp(t->argv[0], "set") == 0)
  {
    if (t->argc == 3)
    {
      return set_variable(t->argv[1], t->argv[2]);
    }
    else
    {
      fprintf(stderr, "Usage: set [PATH|CDPATH|HOME] [newval]\n");
      return EXIT_FAILURE;
    }
  }
  else if (strcmp(t->argv[0], "exit") == 0)
  {
    return execute_exit(t);
  }
  else if (strcmp(t->argv[0], "cd") == 0)
  {
    if (t->argc <= 2)
    {
      return change_dir(t->argv[1]);
    }
    else
    {
      fprintf(stderr, "Usage: cd [dir]\n");
      return EXIT_FAILURE;
    }
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
        return EXIT_FAILURE;
      }
    }
    char *file_path = locate_file(c_argv[0], PATH);

    if (file_path == NULL)
    {
      fprintf(stderr, "%s: unrecognised command\n", c_argv[0]);
      return EXIT_FAILURE;
    }

    if (time)
    {
      int result;
      print_execution_time(time_command(t, file_path, c_argv, &result));
      free(file_path);
      return result;
    }
    else
    {
      int result = execute_command(t, file_path, c_argv);
      free(file_path);
      return result;
    }
  }
}

/**
 * The 'main' execution function - traverses and interprets a command tree
 * and executes each of the nodes based on their type.
 *
 * @param  t the command tree to execute
 * @return   the exit status of the command tree after (attempted) execution
 */
int execute_cmdtree(CMDTREE *t)
{
  if (t == NULL)
  {
    fprintf(stderr, "%s: internal error: null command tree\n", argv0);
    return (last_exit_status = EXIT_FAILURE);
  }

  switch (t->type)
  {
    case N_AND:
      last_exit_status = execute_and(t);
      break;
    case N_BACKGROUND:
      last_exit_status = execute_background(t);
      break;
    case N_OR:
      last_exit_status = execute_or(t);
      break;
    case N_SEMICOLON:
      last_exit_status = execute_semicolon(t);
      break;
    case N_PIPE:
      last_exit_status = execute_pipe(t);
      break;
    case N_SUBSHELL:
      last_exit_status = execute_subshell(t);
      break;
    case N_COMMAND:
      last_exit_status = direct_command(t);
      break;
    default:
      fprintf(stderr, "%s: internal error: unknown node type\n", argv0);
      last_exit_status = EXIT_FAILURE;
      break;
  }

  return last_exit_status;
}
