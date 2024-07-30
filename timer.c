/*	timer.c

	The timeout support....
*/

#include "rules.h"
#include <sys/time.h>
#include <signal.h>

volatile int timeup = 0;

static void
gotsig(register int dummy)
{
        ++timeup;
	signal(SIGALRM, gotsig);
}

void
set_timeout(void)
{
	struct itimerval newt, oldt;

	signal(SIGALRM, gotsig);
        timeup = 0;

	newt.it_interval.tv_sec =
	newt.it_value.tv_sec = timeout;
	newt.it_interval.tv_usec =
	newt.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &newt, &oldt);

	timeup = 0;
}
