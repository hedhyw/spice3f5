#include "spice.h"

#ifndef HAS_DUP2
#include <fcntl.h>

dup2(oldd, newd)
	int	oldd, newd;
{
#ifdef HAS_FCNTL
	close(newd);
#ifdef F_DUPFD
	(void) fcntl(oldd, F_DUPFD, newd);
#endif
#endif
	return 0;
}
#else
int Dummy_Symbol;
#endif
