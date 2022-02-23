#include "port/oc_assert.h"

#include <assert.h>
void abort_impl(void)
{
  assert(NULL);
}

void exit_impl(int status)
{
  (void)status;
  assert(NULL);
}

