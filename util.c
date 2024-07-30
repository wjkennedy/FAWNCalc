/*
 * util.c
 *
 * handy utility functions
 *
 * Created by William R. Dieter, September 2002
 *
 * History
 * -------
 * $Log: util.c,v $
 * Revision 1.2  2002/09/13 19:48:52  kaos
 * Fixed adding a new user.
 * Added permissions support.
 *
 * Revision 1.1  2002/09/13 02:11:04  kaos
 * Converted to new layout.
 *
 */

#include <string.h>

#include "util.h"

char *strsubs(char *str, char old, char new)
{
	char *next;

	next = str;
	while ((next = index(next, old)) != NULL) {
		*next++ = new;
	}

	return str;
}
