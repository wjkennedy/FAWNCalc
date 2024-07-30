/*
 * email.c
 *
 * Send email to users
 *
 * History
 * -------
 * $Log: email.c,v $
 * Revision 1.1  2002/09/19 15:38:21  kaos
 * Added email.c and email.h which are needed for the previous checking that
 * supports email (oops).
 *
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "email.h"

struct email {
	FILE *fp;
	char *mailfile;
	char *to;
};

email_t *email_start(char *from, char *to, char *subject)
{
	email_t *note;
	int   out_fd; 

	/* Make the file and open it */
	if ((note = (email_t *)malloc(sizeof(email_t))) == NULL) {
		return NULL;
	}
	asprintf(&note->mailfile, "cdr_XXXXXX");
	if (note->mailfile == NULL) {
		goto err_free_note;
	}
	if ((note->to = strdup(to)) == NULL) {
		goto err_free_mailfile;
		return NULL;
	}
	if ((out_fd = mkstemp(note->mailfile)) == -1) goto err_free_to;
	if ((note->fp = fdopen(out_fd, "w")) == 0) goto err_close_fd;
	chmod(note->mailfile, (S_IRUSR | S_IWUSR |
			       S_IRGRP | S_IWGRP |
			       S_IROTH | S_IWOTH));

	fprintf(note->fp, "To: %s\n", to);
	fprintf(note->fp, "From: %s\n", from);
	fprintf(note->fp, "Reply-To: %s\n", from);
	fprintf(note->fp, "Subject: %s\n", subject);
	fprintf(note->fp, "\n\n");

	return note;

 err_close_fd:
	close(out_fd);
 err_free_to:
	free(note->to);
 err_free_mailfile:
	free(note->mailfile);
 err_free_note:
	free(note);
	return NULL;
}

int email_printf(email_t *note, char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	return vfprintf(note->fp, fmt, ap);
}

int email_end(email_t *note)
{
	fclose(note->fp);

	if (!fork()) {
		close(0);
		open(note->mailfile, O_RDONLY);
		close(1);
		close(2);
		execl("/usr/sbin/sendmail",
		      "/usr/sbin/sendmail",
		      note->to,
		      NULL);

		exit(0);
	}
	free(note->to);
	free(note->mailfile);
	free(note);
	wait(NULL);

	return(0);
}
