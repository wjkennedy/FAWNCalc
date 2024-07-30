/*
 * email.c
 *
 * Send email to users
 *
 * History
 * -------
 * $Log: email.h,v $
 * Revision 1.1  2002/09/19 15:38:21  kaos
 * Added email.c and email.h which are needed for the previous checking that
 * supports email (oops).
 *
 */

#ifndef EMAIL_H
#define EMAIL_H

typedef struct email email_t;

email_t *email_start(char *from, char *to, char *subject);
int email_end(email_t *note);
int email_printf(email_t *note, char *fmt, ...);

#endif
