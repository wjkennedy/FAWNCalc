/*
 * login.c
 *
 * User management routines (login, create users, modify password file, etc)
 *
 * Created by William R. Dieter, August 2002
 *
 * History
 * -------
 * $Log: login.c,v $
 * Revision 1.9  2004/06/02 22:46:36  wrdieter
 * In Makefile.in, create a default cdrpasswd file if one does not exist.
 * If database cannot be read, print an error rather than just crashing.
 *
 * Revision 1.8  2004/01/30 02:32:42  wrdieter
 * Changes from Anand Kadiyala so that the CDR generates the new user form
 * dynamically rather than linking to an external file.
 *
 * Revision 1.7  2003/11/18 04:56:28  wrdieter
 * Added headers to work with Linux.
 * Added configure and Makefile
 *
 * Revision 1.6  2002/09/17 21:04:23  kaos
 * Fixed a typo in a message.
 * Added email notification for parameter save and new user creation.
 *
 * Revision 1.5  2002/09/17 17:19:44  kaos
 * Added expiration.
 *
 * Revision 1.4  2002/09/13 19:48:52  kaos
 * Fixed adding a new user.
 * Added permissions support.
 *
 * Revision 1.3  2002/09/13 02:11:04  kaos
 * Converted to new layout.
 *
 * Revision 1.2  2002/09/12 19:47:41  kaos
 * Touched up things a bit.  No major changes.
 *
 * Revision 1.1  2002/09/11 14:26:01  kaos
 * Added ability to edit parameter set and present the form dynamically showing
 * existing parameter sets.
 *
 */

#include <sys/file.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* should be in unistd.h, but it's not */
extern char *crypt(const char *key, const char *salt);

#include "cgic.h"
#include "email.h"
#include "rules.h"
#include "util.h"

#define MAX_TRIES  	5		/* Max number of times to try lookup up a host */
#define PASSWD_FILE 	"cdrpasswd"	/* name of the file in which passwords are stored */

static int valid_email(char *email);
static int valid_passwd(char *passwd);
static int add_user(char *email, char *password);
static int new_uid(void);
static void get_salt(char *salt);

void new_user_form(void)
{
	fprintf(cgiOut, "<head>"
	       "<title>New User Account</title>"
	       "</head>"
	       "<body>"
	       "<h1>New User Account</h1>"
	       "<p>To add a user account enter your email address and password below."
	       "<p>"
	       "<FORM METHOD=\"POST\" ACTION=\"cdr.cgi\">"
	       "<INPUT TYPE=\"HIDDEN\" NAME=\"version\" VALUE=\"20100119\">"
	       "<INPUT TYPE=\"HIDDEN\" NAME=\"timeout\" VALUE=\"30\">"
	       "<TABLE>"
	       "<TR>"
	       "<TD ALIGN=RIGHT>Email address:</TD>"
	       "<TD><INPUT TYPE=\"TEXT\" NAME=\"email\" VALUE=\"\" SIZE=\"40\""
	       "MAXLENGTH=4000></TD>"
	       "</TR>"
	       "<TR>"
	       "<TD ALIGN=RIGHT>Password:</TD>"
	       "<TD><INPUT TYPE=\"PASSWORD\" NAME=\"password\" VALUE=\"\" SIZE=10 MAXLENGTH=4000></TD></TR>"
	       "<TR>"
	       "<TD ALIGN=RIGHT>Verify Password:</TD>"
	       "<TD><INPUT TYPE=\"PASSWORD\" NAME=\"password2\" VALUE=\"\" SIZE=10 MAXLENGTH=4000></TD>"
	       "</TR>"
	       "</TABLE>"
	       "<INPUT TYPE=\"SUBMIT\" VALUE=\"Create User Account\" NAME=\"new_user\">"
	       "<INPUT TYPE=\"RESET\" VALUE=\"Reset Form\">"
	       "</FORM>"
	       "<p>"
	       "<p><B>WARNING:</B> The password entered in this form is sent across"
	       "the network in the clear.  Though unlikely, it is possible that"
	       "someone could intercept it.  Do not use the same password here that"
	       "you use for more secure accounts."
	       "</p>"
	       "</BODY>"
	       "</HTML>");
}

/* returns true if user successfully added, else prints error message
 * and returns false
 */
int new_user(void)
{
	char passwd_buf[MAXLEN];

	if (!valid_email(email)) {
		err_exit("Invalid email address.");
		/* new_login(); */
	}

	/* make sure email is unique */
	passwd_buf[0] = '\0';
	if (get_passwd(email, passwd_buf, MAXLEN) >= 0) {
		err_exit("A user with the address %s already exists\n", email);
	}
	/* only need to compare password address to see if passwords are the 
	 * same because if they match then they are hashed to the same address
	 */
	if (password != password2) {
		/* password mismatch */
		err_exit("password mismatch\n");
	}
	if (!valid_passwd(password)) {
		err_exit("choose a better password");
	}
	return add_user(email, password);
}

int validate_login(char *email, char *passwd)
{
	int uid;
	char  passwd_buf[MAXLEN];
	char *stored_passwd;
	char *enc;

	if ((uid = get_passwd(email, passwd_buf, MAXLEN)) >= 0) {
		if ((stored_passwd = index(passwd_buf, ':')) == NULL) {
			err_exit("The password file has been corrupted; "
				 "cannot find uid\n");
		}
		stored_passwd++;
		uid = atoi(stored_passwd);
		if ((stored_passwd = index(stored_passwd, ':')) == NULL) {
			err_exit("The password file has been corrupted; "
				 "cannot find password\n");
		}
		stored_passwd++;
		/* skip config expiration */
		if ((stored_passwd = index(stored_passwd, ':')) == NULL) {
			err_exit("The password file has been corrupted; "
				 "cannot find password\n");
		}
		/* skip the ':' and discard the newline */
		stored_passwd++;
		stored_passwd[strlen(stored_passwd)-1] = '\0';
		enc = crypt(passwd, stored_passwd);
		if (strcmp(enc, stored_passwd) != 0) {
			/* password mismatch */
			uid = -1;
		}
	}
		
	return uid;
}

void invalid_login(void)
{
	fprintf(cgiOut,
		"<head>\n"
		"<title>Invalid Email Address or Password</title>\n"
		"</head>\n"
		"<body>\n"
		"<H1>Invalid Email Address or Password</H1>\n"
		"<p>Either the email address you typed is not\n"
		"in our database, or the password you typed does\n"
		"not match the password we have recorded for you.\n");
}

/* for now your email is valid if it has exactly one '@' in it and the '@' is
 * neither first nor last.
 */
static int valid_email(char *email)
{
	char           *domain;
	struct hostent *ent;
	int             tries;

	domain = index(email, '@');
	if (domain == NULL || domain == email) {
		/* reject email addresses without a domain name or with 0 
		 * length user names
		 */
		return 0;
	}

	domain++;
	tries = 0;
	do {
		ent = gethostbyname(domain);
		tries++;
	} while (ent == NULL && h_errno == TRY_AGAIN && tries < MAX_TRIES);

	/* allow any domainname that can be looked up as a host or that fails 
	 * the hostname lookup, but may be valid in some other context (because
	 * because I'm too lazy to figure out how to determine if it's a valid
	 * mail server.
	 */
	return ent || (h_errno == NO_DATA);
}

static int valid_passwd(char *passwd)
{
	/* allow really lame, but not empty, passwords for now */
	return passwd[0] != '\0';
}

int get_passwd(char *email, char *buf, int size)
{
	FILE *fp;
	int   index;

	if ((fp = fopen(PASSWD_FILE, "r")) == NULL) {
		err_exit("cannot open password file for read\n");
	}
	if (flock(fileno(fp), LOCK_SH) < 0) {
		err_exit("cannot lock password file shared\n");
	}
	index = 0;
	while((buf = fgets(buf, size, fp))
	      && (strncmp(email, buf, strlen(email)) != 0))
		index++;
	if (flock(fileno(fp), LOCK_UN) < 0) {
		err_exit("cannot unlock password file\n");
	}
	fclose(fp);
	if (buf == NULL)
		index = -1;

	return index;
}

/* this should probably be generalized... */
void print_options(void)
{
	FILE *fp;
	char  buf[MAXLEN];
	char *str;
	char *name;
	int   uid;
	FILE *dbfp;
	char  db_buf[MAXLEN];
	int   print_option;
	int   expired;

	fprintf(cgiOut,
		"<SELECT NAME=\"paramset\">\n"
		"<OPTION VALUE=\"0\"%s>Default Parameter Set\n",
		(g_uid <= 0) ? " selected" : "");
	if ((fp = fopen(PASSWD_FILE, "r")) == NULL) {
		err_exit("cannot open password file for read\n");
	}
	if (flock(fileno(fp), LOCK_SH) < 0) {
		err_exit("</SELECT><P>cannot lock password file shared.");
	}
	while(fgets(buf, MAXLEN, fp) != NULL) {
		expired = is_expired(buf);
		if ((str = index(buf, ':')) == NULL) {
			flock(fileno(fp), LOCK_UN);
			err_exit("password file corrupted, cannot find uid\n");
		}
		*str = '\0';
		str++;
		uid = atoi(str);
		if (expired && uid != g_uid) {
			continue;
		} 
		if ((dbfp = fopen_db(uid, "r")) == NULL) {
			flock(fileno(fp), LOCK_UN);
			err_exit("Cannot open database for user %s (uid = %d)\n",
				 buf, uid);
		}
		if (flock(fileno(dbfp), LOCK_SH) < 0) {
			err_exit("Cannot lock database for uid %d\n", uid);
		}
		/* ASSUME: a buffer of size MAXLEN is big enough to hold the
		 * database name, email, and oread or oeval
		 */
		if (fgets(db_buf, MAXLEN, dbfp)) {
			if ((name = index(db_buf, '=')) == NULL) {
				flock(fileno(dbfp), LOCK_UN);
				flock(fileno(fp), LOCK_UN);
				err_exit("Cannot find name in database %d\n", uid);
			}
			name++;
			if ((str = index(name, '&')) == NULL) {
				flock(fileno(dbfp), LOCK_UN);
				flock(fileno(fp), LOCK_UN);
				err_exit("Cannot find end of name in db %d\n", uid);
			}
			*str = '\0';
			/* look for oeval in rest of this buf */
			print_option = 0;
			while (str != NULL && !print_option) {
				str++;
				print_option = strncmp(str, "oeval", strlen("oeval")) == 0;
				str = index(str, '&');
			}
			if (print_option || uid == g_uid) {
				strsubs(name, '+', ' ');
				fprintf(cgiOut, "<OPTION VALUE=\"%d\"%s>%s - %s\n",
					uid, ((uid == g_uid) ? " selected" : ""),
					name, buf);
				if (expired) {
					fprintf(cgiOut,
						" <FONT COLOR=\"RED\"><STRONG>(expired)</STRONG></FONT>\n");
				}
			}
		}
		if (flock(fileno(dbfp), LOCK_UN) < 0) {
			err_exit("Cannot unlock database for uid %d\n", uid);
		}
		fclose(dbfp);

	}
	if (flock(fileno(fp), LOCK_UN) < 0) {
		err_exit("</SELECT><P>Cannot unlock password file\n");
	}
	fclose(fp);
	fprintf(cgiOut,
		/* "<OPTION VALUE=\"-1\">Private Custom Parameter Set\n" */
		"</SELECT>\n");
}

#define SALTBYTES (8)
#define SALTLEN   (SALTBYTES+2)

/* add to password database */
static int add_user(char *email, char *password)
{
	FILE    *fp;
	char    *enc;
	char     salt[SALTLEN];
	int      uid;
	email_t *note;

	uid = new_uid();
	if ((fp = fopen(PASSWD_FILE, "a")) == NULL) {
		err_exit("cannot open password file for append\n");
	}
	if (flock(fileno(fp), LOCK_EX) < 0) {
		err_exit("Cannot get exclusive password file lock\n");
	}
	get_salt(salt);
	enc = crypt(password, salt);
	fprintf(fp, "%s:%d:%016x:%s\n", email, uid, 0, enc);
	if (flock(fileno(fp), LOCK_UN) < 0) {
		err_exit("Cannot unlock exclusive password file\n");
	}
	fclose(fp);

	if ((note = email_start(CDR_EMAIL, email, "[CDR] User Account Created")) == NULL) {
		err_exit("Could not open email");
	}
	email_printf(note,
		     "Welcome to aggregate.org.\n\n"
		     "A Cluster Design Rules user account has been created for\n"
		     "%s.  If you did not create this account,\n"
		     "please notify the Cluster Design Rules administrator at:\n\n"
		     "	"CDR_EMAIL,
		     email);
	email_end(note);

	return uid;
}

static int new_uid(void)
{
	FILE *fp;
	int   max_uid;
	int   uid;
	char  buf[MAXLEN];
	char  *ustr;

	/* ASSUME: we will never have more than 2^31-1 users */
	/* ASSUME: lines are less than MAXLEN */
	if ((fp = fopen(PASSWD_FILE, "r")) == NULL) {
		err_exit("cannot open password file for read\n");
	}
	if (flock(fileno(fp), LOCK_SH) < 0) {
		err_exit("cannot lock password file shared.");
	}

	max_uid = 0;
	while(fgets(buf, MAXLEN, fp)) {
		ustr = index(buf, ':');
		if (ustr != NULL) {
			ustr++;
			uid = atoi(ustr);
			max_uid = (max_uid >= uid) ? max_uid : uid;
		}
	}
	if (flock(fileno(fp), LOCK_UN) < 0) {
		err_exit("Cannot unlock password file\n");
	}
	fclose(fp);

	if (max_uid + 1 < 0) {
		err_exit("Too many users.  Over 2 billion served!\n");
	}

	return max_uid+1;
}

int get_uid(char *email)
{
	char buf[MAXLEN];
	char *ustr;

	get_passwd(email, buf, MAXLEN);
	if ((ustr = index(buf, ':')) == NULL) {
		err_exit("Database is corrupt\n");
	}
	ustr++;
	return atoi(ustr);
}

/* salt must be big enough for 10 chars */
static void get_salt(char *salt)
{
	int fd;
	uint64 rand64;
	int    rand6;
	int    i;

	if ((fd = open("/dev/urandom", O_RDONLY)) < 0) {
		err_exit("cannot open /dev/urandom for reading\n");
	}
	if (read(fd, &rand64, sizeof(rand64)) < sizeof(rand64)) {
		err_exit("could not read 8 bytes from /dev/urandom\n");
	}
	close(fd);

	salt[0] = '_';
	for (i = 1 ; i <= SALTBYTES ; i++) {
		rand6 = rand64 & 0x3f;
		rand64 = rand64 >> 6;
		salt[i] = rand6 + '.';
		if(salt[i] > '9')
			rand6 += 'A' - '9';
		else if (salt[i] > 'Z')
			rand6 += 'a' - 'Z';
	}
	salt[i] = '\0';
}

int update_expiration(char *email)
{
	FILE *fp;
	char  buf[MAXLEN];
	char *line;
	int   email_len;
	long  offset;

	if ((fp = fopen(PASSWD_FILE, "r+")) == NULL) {
		err_exit("cannot open password file to update expiration\n");
	}
	if (flock(fileno(fp), LOCK_EX) < 0) {
		err_exit("cannot lock password file shared.");
	}
	/* find the line we want */
	email_len = strlen(email);
	while(((line = fgets(buf, MAXLEN, fp)) != NULL)
	      && strncmp(email, buf, email_len))
		;
	if (!line) {
		flock(fileno(fp), LOCK_UN);
		err_exit("cannot update expiration for %s: user not found\n",
			 email);
	}

	/* find the expiration field in the buffer */
	if ((line = index(line, ':')) == NULL) {
		flock(fileno(fp), LOCK_UN);
		err_exit("password file corrupted: update_expiration "
			 "cannot find end of email.\n");
	}
	if ((line = index(++line, ':')) == NULL) {
		flock(fileno(fp), LOCK_UN);
		err_exit("password file corrupted: update_expiration "
			 "cannot find end of uid.\n");
	}
	line++;
	/* put the expiration into the password file at the right place */
	offset = line - buf - strlen(buf);
	if (fseek(fp, offset, SEEK_CUR) < 0) {
		flock(fileno(fp), LOCK_UN);
		err_exit("cannot seek to expiration in password file.\n"
			 "(offset = %ld)", offset);
	}
	fprintf(fp, "%016lx", time(NULL) + PARAM_LIFETIME);
	if (flock(fileno(fp), LOCK_UN) < 0) {
		err_exit("Cannot unlock password file\n");
	}
	fclose(fp);

	return 0;
}

/* is pw_entry expired */
int is_expired(char *pw_entry)
{
	time_t exp;
	/* skip email and uid */
	if ((pw_entry = index(pw_entry, ':')) == NULL) {
		err_exit("password file corrupted: is_expired "
			 "cannot find end of email.\n");
	}
	if ((pw_entry = index(++pw_entry, ':')) == NULL) {
		err_exit("password file corrupted: is_expired "
			 "cannot find end of uid.\n");
	}
	exp = strtol(++pw_entry, NULL, 16);

	return exp < time(NULL);
} 
