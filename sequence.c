/**
 * Author:          Samuel Marsh
 * Date:            30/10/2015
 */

#include "mysh.h"

/**
 * Executes the left and right branches of the command tree in sequential order.
 *
 * @param  t the command tree, with root node of type N_SEMICOLON
 * @return   the exit status of the right child of the command tree
 */
int execute_semicolon(CMDTREE *t)
{
  //execute in order, no matter the result of the left side
  int exit_status = EXIT_SUCCESS;

  if (t->left != NULL)
  {
    execute_cmdtree(t->left);
  }

  if (t->right != NULL)
  {
    exit_status = execute_cmdtree(t->right);
  }

  return exit_status;
}

/**
 * Executes the left branch of the command tree, and only on failure executes
 * the right branch (i.e. a short-circuiting OR operator).
 *
 * @param  t the command tree, with root node of type N_OR
 * @return   the exit status of the most recently executed command (that is,
 *           if the left branch returns an exit status indicating success that
 *           status is returned, otherwise the right branch is executed and the
 *           exit status of that branch is returned)
 */
int execute_or(CMDTREE *t)
{
  //always execute left hand side
  int exit_status = EXIT_SUCCESS;

  if (t->left != NULL)
  {
    exit_status = execute_cmdtree(t->left);
  }

  //short circuit upon failure
  if (exit_status != EXIT_SUCCESS && t->right != NULL)
  {
    exit_status = execute_cmdtree(t->right);
  }

  return exit_status;
}

/**
 * Executes the left branch of the command tree, and only on success executes
 * the right branch (i.e. a short-circuiting AND operator)
 *
 * @param  t the command tree, with root node of type N_AND
 * @return   the exit status of the most recently executed command (that is,
 *           if the left branch returns an exit status indicating failure that
 *           status is returned otherwise the right branch is executed and the
 *           exit status of that branch is returned)
 */
int execute_and(CMDTREE *t)
{
  //always execute left hand side
  int exit_status = EXIT_SUCCESS;

  if (t->left != NULL)
  {
    exit_status = execute_cmdtree(t->left);
  }

  //short circuit upon success
  if (exit_status == EXIT_SUCCESS && t->right != NULL)
  {
    exit_status = execute_cmdtree(t->right);
  }

  return exit_status;
}
