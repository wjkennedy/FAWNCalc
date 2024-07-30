#include <errno.h>

#include "rules.h"		/* for err_exit */

static int merge(void *base, size_t len1, size_t len2,
		  size_t size, int (*compar)(const void *, const void *));

int mergesort __P((void *base, size_t nmemb, size_t size,
		    int (*compar)(const void *, const void *)))
{
	size_t mid;

	if (nmemb < 2) {
		/* list is already sorted */
		return 0;
	}
	mid = nmemb/2;
	if (mergesort(base,       mid,         size, compar) < 0) {
		return -1;
	}
	if (mergesort(base + mid * size, nmemb - mid, size, compar) < 0) {
		return -1;
	}
	if (merge(base, mid, nmemb - mid, size, compar) < 0) {
		return -1;
	}
	return 0;
}


/* merge len1 things starting at base with len2 things starting at
   base+len1 storing the result at base and without allocating extra
   storage.  NOTE: this assumes that that what is at base+len1 can be
   overwritten
*/

static int merge(void *base, size_t len1, size_t len2,
		  size_t size, int (*compar)(const void *, const void *))
{
	char *l1;		/* list 1 */
	char *l2;		/* list 2 */
	char *merged, *next_merge;
	int len;

	if ((merged = malloc((len1 + len2) * size)) == NULL) {
		errno = ENOMEM;
		return -1;
	}
	
	l1 = (char *)base;
	l2 = (char *)base + len1 * size;
	len = len1 + len2;

	next_merge = merged;
	while(len1 > 0 && len2 > 0) {
		if (compar(l1, l2) <= 0) {
			memcpy(next_merge, l1, size);
			l1 += size;
			len1--;
		} else {
			memcpy(next_merge, l2, size);
			l2 += size;
			len2--;
		}
		next_merge += size;
	}

	/* copy remaining items */
	if (len1 == 0) {
		memcpy(next_merge, l2, size * len2);
	} else {
		memcpy(next_merge, l1, size * len1);
	}

	memcpy(base, merged, size * len);

	free(merged);

	return 0; 
}
