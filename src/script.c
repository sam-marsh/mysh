#include "mysh.h"

/**
 * Executes the shell script residing at the given file path. Prints an error
 * message and exits on failure.
 *
 * @param path the path to the shell script
 */
void execute_script(char *path)
{
  redirect_io_stream(fileno(stdin), path, "r");
  exit(run_mysh());
}
