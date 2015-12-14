/**
 * Author:          Samuel Marsh
 * Date:            30/10/2015
 */ 

#include "execute.h"

/**
 * Executes the shell script residing at the given file path. Prints an error
 * message and exits on failure. Called from a child process.
 *
 * @param path the path to the shell script
 */
void execute_script(char *path)
{
  //set stdin to read from the file
  //the shell should no longer be interactive - calling run_mysh() below will
  //fix this by re-checking stdin (isatty)
  redirect_io_stream(fileno(stdin), path, "r");
  //re-run the program... this will reset all internal variables.
  exit(run_mysh());
}

/**
 * Attempts to execute an external program from a file in a child process.
 * If invoking the process fails, the contents of the file are interpreted as
 * commands (a shell script).
 *
 * @param  command the command tree, with root node of type N_COMMAND
 * @param  path    the full path to the file
 * @param  argv    the arguments to be passed to the program
 * @return         the exit status of the program
 */
int execute_external_command(CMDTREE *command, char *path, char **argv)
{
  int child_pid;
  switch (child_pid = fork())
  {
    case FORK_FAILURE:
      MYSH_PERROR("execute_command");
      return EXIT_FAILURE;
    case FORK_CHILD:
      //check and enable I/O redirection if required
      set_redirection(command);

      if (access(path, X_OK) == 0)
      {
        //attempt to invoke the program
        execv(path, argv);

        //failed to access for execution for whatever reason - try interpreting as script
        execute_script(path);

        //should never reach here - will have errored and exited before this
        //point anyway if something went wrong
        fprintf(stderr, "%s: %s: Failed to execute shell script\n", argv0, argv[0]);
        exit(EXIT_FAILURE);
      }
      else
      {
        //couldn't run, couldn't read
        fprintf(stderr, "%s: %s: Permission denied\n", argv0, argv[0]);
        exit(EXIT_FAILURE);
      }
      break;
    default:
    {
      //in the parent process, just wait for the child to finish execution
      int exit_status;
      waitpid(child_pid, &exit_status, 0);
      exit_status = WEXITSTATUS(exit_status);
      return exit_status;
    }
  }

  fprintf(stderr, "%s: internal error: should never get here\n", argv0);
  exit(EXIT_FAILURE);
}

/**
 * Executes a command node, which could represent an internal (cd, time) or
 * external (e.g. /bin/ls) command.
 * @param  t the command tree, with root node of type N_COMMAND
 * @return   the exit status of the command
 */
int execute_generic_command(CMDTREE *t, int c_argc, char **c_argv)
{
  int exit_status;
  if (execute_internal_command(t, c_argc, c_argv, &exit_status))
  {
    return exit_status;
  }

  //reached here - user didn't request an internal command, so search for an
  //external one

  char *file_path = locate_file(c_argv[0], PATH);

  if (file_path == NULL)
  {
    fprintf(stderr, "%s: %s: command not found...\n", argv0, c_argv[0]);
    return EXIT_FAILURE;
  }

  int result = execute_external_command(t, file_path, c_argv);
  free(file_path);
  return result;
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
      last_exit_status = execute_generic_command(t, t->argc, t->argv);
      break;
    default:
      fprintf(stderr, "%s: internal error: unknown node type\n", argv0);
      last_exit_status = EXIT_FAILURE;
      break;
  }

  return last_exit_status;
}
