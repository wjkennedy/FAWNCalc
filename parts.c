/*	parts.c

	Get parts list and other input info...
*/

#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "rules.h"


int	version;
int	timeout;

cdrop_t cdr_op;
int     paramset;
char   *email;
char   *password;
char   *password2;
int     g_uid;
int     oread;
int     oeval;
char   *evalcomment;
char   *viewcomment;

/* requirements from the form */
double	ossize;
double	codesize;
double	datasize;
double	minmemgbs;
double	disksize;
int	swapsize;
int	useraid;
int	usebackup;
double	maxlatency;
double	afnlatency;
double	minbandwidth;
int	coordinality;
int	nodecon0;
int	nodecon1;
int	nodecon2;
int	sparen0;
int	sparen1;
int	racks;
double	amps;
double	tons;			/* tons of air conditioning */
int	video;
double	budget;
double	gflops;
int	howmany;

/* metric weights form the form */
double	metmem;
double	metmemgbs;
double	metdisk;
double	metlat;
double	metbw;
double	metcost;
double	metgflops;
double	metamps;
double	mettons;

int     limit;
int	include_logic;
char   	include_buf[MAX_LIMIT_STR];
char  **include_str;
int     include_strs;
int	exclude_logic;
char   	exclude_buf[MAX_LIMIT_STR];
char  **exclude_str;
int     exclude_strs;

char    *dbname;
char    *dbemail;

nict	nic_opt[MAXOPTS];
int	nic_opts = 0;

cabt	cab_opt[MAXOPTS];
int	cab_opts = 0;

swt	sw_opt[MAXOPTS];
int	sw_opts = 0;

proct	proc_opt[MAXOPTS];
int	proc_opts = 0;

momt	mom_opt[MAXOPTS];
int	mom_opts = 0;

memt	mem_opt[MAXOPTS];
int	mem_opts = 0;

diskt	disk_opt[MAXOPTS];
int	disk_opts = 0;

kaset	kase_opt[MAXOPTS];
int	kase_opts = 0;

rackt	rack_opt[MAXOPTS];
int	rack_opts = 0;

bundlet  bundle_opt[MAXOPTS];
int      bundle_opts;

int     mom_bund;
int     sw_bund;
int     nic_bund;
int     cab_bund;
int     proc_bund;
int     mem_bund;
int	kase_bund;
int	rack_bund;
int	disk_bund;

#define FIND_BUND(part,prev) \
for(part##_bund= prev##_bund ; (part##_bund < bundle_opts) && (bundle_opt[part##_bund]. prev##_qty > 0) ; part##_bund++);

#define	MAXHASH	2048		/* a power of 2 */
static	char *hashtab[MAXHASH] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};


char *
hash(register char *s)
{
	register char *p = s;
	register int index = *p;

	/* A lame hash function... */
	while (*p) {
		index = ((index * 13) + *p);
		++p;
	}
	index &= (MAXHASH - 1);

	while (hashtab[index]) {
		if (strcmp(hashtab[index], s) == 0) return(hashtab[index]);
		index = ((index + 1) & (MAXHASH - 1));
	}

	return(hashtab[index] = strdup(s));
}

#define getbigsval(format, field) \
	sprintf(namebuf, format, i); \
	valbuf[0] = 0; \
	cgiFormString(namebuf, valbuf, MAXLEN); \
	field = hash(valbuf);

#define	getsval(format, field) \
	sprintf(namebuf, format, i); \
	valbuf[0] = 0; \
	cgiFormStringNoNewlines(namebuf, valbuf, MAXLEN); \
	field = hash(valbuf);

#define	getival(format, field) \
	sprintf(namebuf, format, i); \
	valbuf[0] = 0; \
	cgiFormInteger(namebuf, &(field), 0);

#define	getdval(format, field) \
	sprintf(namebuf, format, i); \
	valbuf[0] = 0; \
	cgiFormDouble(namebuf, &(field), 0);

#define getynval(format, field) \
	sprintf(namebuf, format, i); \
	valbuf[0] = 0; \
	cgiFormStringNoNewlines(namebuf, valbuf, MAXLEN); \
	field = ((valbuf[0] == 'Y') || (valbuf[0] == 'y'));

#define XNOR(x,y) (((x) && (y)) || (!(x) && !(y)))

void    set_availability(void);
void	parse_limit_strs(void);
char   *word_end(char *buf);
int	count_words(char *buf);
void 	parse_buf(char *buf, int wordc, char **wordv);

/* set globals for args not in the database */
void
get_cgi_args(void)
{
	char namebuf[MAXLEN];
	char valbuf[MAXLEN];
	register int i;


	/* Check version number... */
	cgiFormInteger("version", &version, 0);
	if (version != VERSION && version != 0) {
		out_version();
		exit(1);
	}

	/* deterimine what action to take */
	if (cgiFormStringNoNewlines("login", valbuf, MAXLEN)
	    != cgiFormNotFound) {
		cdr_op = LOGIN;
	} else if (cgiFormStringNoNewlines("edit_params", valbuf, MAXLEN)
	    != cgiFormNotFound) {
		cdr_op = EDIT_PARAMS;
	} else if (cgiFormStringNoNewlines("set_params", valbuf, MAXLEN)
		   != cgiFormNotFound) {
		cdr_op = SET_PARAMS;
	} else if (cgiFormStringNoNewlines("more_opts", valbuf, MAXLEN)
		   != cgiFormNotFound) {
		cdr_op = MORE_OPTS;
	} else if (cgiFormStringNoNewlines("new_user", valbuf, MAXLEN)
		   != cgiFormNotFound) {
		cdr_op = NEW_USER;
	} else if (cgiFormStringNoNewlines("new_user_form", valbuf, MAXLEN)
		   != cgiFormNotFound) {
		cdr_op = NEW_USER_FORM;
	} else if (cgiFormStringNoNewlines("eval_design", valbuf, MAXLEN)
		   != cgiFormNotFound) {
		cdr_op = EVAL_DESIGN;
	} else if (cgiFormStringNoNewlines("view_params", valbuf, MAXLEN)
		   != cgiFormNotFound) {
		cdr_op = VIEW_PARAMS;
	} else if (version == 0) {
		/* someone called the cgi directly (not through a form) */
		cdr_op = PRINT_FORM;
	} else {
		cdr_op = ENTER_DESIGN;
	}
        /* i is used in getsval for parameterized names.  There is no %d
	 * below, so sprintf will ignore i, but setting i gets rid of
	 * compiler warning
	 */
	i = 0;			
	getsval("email", email);
	getsval("password", password);
	getsval("password2", password2);
	g_uid =  (email[0] != '\0')
		? g_uid = validate_login(email, password) : -1;
	cgiFormInteger("paramset", &paramset, DEFAULT_PARAMS);
	  
	/* How long to run...? */
	cgiFormInteger("timeout", &timeout, MAX_SECS);

	/* The user parameters... */
	cgiFormDouble("ossize", &ossize, 16.0);
	cgiFormDouble("codesize", &codesize, 16.0);
	cgiFormDouble("datasize", &datasize, 512.0);
	cgiFormDouble("minmemgbs", &minmemgbs, 1.0);
	cgiFormDouble("disksize", &disksize, 1024.0);
	cgiFormInteger("swapsize", &swapsize, 0);
	cgiFormInteger("useraid", &useraid, 0);
	cgiFormInteger("usebackup", &usebackup, 0);
	cgiFormDouble("maxlatency", &maxlatency, 10000.0);
	cgiFormDouble("afnlatency", &afnlatency, 10000.0);
	cgiFormDouble("minbandwidth", &minbandwidth, 100.0);
	cgiFormInteger("coordinality", &coordinality, 1);
	cgiFormInteger("nodecon0", &nodecon0, 0);
	cgiFormInteger("nodecon1", &nodecon1, 0);
	cgiFormInteger("nodecon2", &nodecon2, 0);
	cgiFormInteger("sparen0", &sparen0, 0);
	cgiFormInteger("sparen1", &sparen1, 16);
	cgiFormInteger("racks", &racks, 4);
	cgiFormDouble("amps", &amps, 40.0);
	cgiFormDouble("tons", &tons, 5.0);
	cgiFormInteger("video", &video, 0);
	cgiFormDouble("budget", &budget, 10000.0);
	cgiFormDouble("gflops", &gflops, 16.0);
	cgiFormInteger("howmany", &howmany, 10);

	if (howmany > MAXDESIGNS) howmany = MAXDESIGNS;

	/* Weighting function... */
	cgiFormDouble("metmem", &metmem, 10000.0);
	cgiFormDouble("metmemgbs", &metmemgbs, 10000.0);
	cgiFormDouble("metdisk", &metdisk, 1000.0);
	cgiFormDouble("metlat", &metlat, 10.0);
	cgiFormDouble("metbw", &metbw, 100.0);
	cgiFormDouble("metcost", &metcost, 100000.0);
	cgiFormDouble("metgflops", &metgflops, 1000000.0);
	cgiFormDouble("metamps", &metamps, 1.0);
	cgiFormDouble("mettons", &mettons, 1.0);

	cgiFormInteger("include_logic", &include_logic, 0);
	cgiFormStringNoNewlines("include_str", include_buf, MAX_LIMIT_STR);
	cgiFormInteger("exclude_logic", &exclude_logic, 0);
	cgiFormStringNoNewlines("exclude_str", exclude_buf, MAX_LIMIT_STR);
	parse_limit_strs();
}

/* set globals for args in the database */
void get_cgi_db_args(void)
{
	char namebuf[MAXLEN];
	char valbuf[MAXLEN];
	register int i;
	register int opt;

	/* getsval uses i, but it will be ignored here becuase there is no
	 * %d in first arg to getsval.  Set i = 0 to remove compiler warning.
	 */
	i = 0;			
	getsval("dbname", dbname);
	getsval("dbemail", dbemail);
	oread = (cgiFormCheckboxSingle("oread") == cgiFormSuccess);
	oeval = (cgiFormCheckboxSingle("oeval") == cgiFormSuccess);
	getbigsval("evalcomment", evalcomment);
	getbigsval("viewcomment", viewcomment);

	/* The usually hidden parameters... */
	for (i=0, opt=0; i<MAXOPTS; ++i) {
		getsval("nic%dname", nic_opt[opt].name);
		if (valbuf[0] == 0) continue;

		getsval("nic%dntype", nic_opt[opt].ntype);
		getdval("nic%dmbs", nic_opt[opt].mbs);
		getdval("nic%dus", nic_opt[opt].us);
		getdval("nic%dcost", nic_opt[opt].cost);
		nic_opt[opt].estat = 0;
		opt++;
	}
	nic_opts = opt;

	for (i=0, opt=0; i<MAXOPTS; ++i) {
		getsval("cab%dname", cab_opt[opt].name);
		if (valbuf[0] == 0) continue;

		getsval("cab%dntype", cab_opt[opt].ntype);
		getdval("cab%dfeet", cab_opt[opt].feet);
		getdval("cab%dcost", cab_opt[opt].cost);
		cab_opt[opt].estat = 0;
		opt++;
	}
	cab_opts = opt;

	for (i=0, opt=0; i<MAXOPTS; ++i) {
		getsval("sw%dname", sw_opt[opt].name);
		if (valbuf[0] == 0) continue;

		getsval("sw%dntype", sw_opt[opt].ntype);
		getdval("sw%dmbs", sw_opt[opt].mbs);
		getdval("sw%dus", sw_opt[opt].us);
		getival("sw%dports", sw_opt[opt].ports);
		getdval("sw%dcost", sw_opt[opt].cost);
		getival("sw%du", sw_opt[opt].u);
		getdval("sw%damps", sw_opt[opt].amps);
		sw_opt[opt].estat = 0;
		opt++;
	}
	sw_opts = opt;

	for (i=0, opt=0; i<MAXOPTS; ++i) {
		getsval("proc%dname", proc_opt[opt].name);
		if (valbuf[0] == 0) continue;

		getsval("proc%dptype", proc_opt[opt].ptype);
		getdval("proc%dgflops", proc_opt[opt].gflops);
		getdval("proc%dcost", proc_opt[opt].cost);
		proc_opt[opt].estat = 0;
		opt++;
	}
	proc_opts = opt;

	for (i=0, opt=0; i<MAXOPTS; ++i) {
		getsval("mom%dname", mom_opt[opt].name);
		if (valbuf[0] == 0) continue;

		getsval("mom%dptype", mom_opt[opt].ptype);
		getsval("mom%dmtype", mom_opt[opt].mtype);
		getsval("mom%dctype", mom_opt[opt].ctype);
		getival("mom%dn", mom_opt[opt].n);
		getival("mom%dmem", mom_opt[opt].mem);
		getival("mom%dpci", mom_opt[opt].pci);
		getival("mom%dide", mom_opt[opt].ide);
		getynval("mom%dpxe", mom_opt[opt].pxe);
		getdval("mom%dcost", mom_opt[opt].cost);
		mom_opt[opt].estat = 0;
		opt++;
	}
	mom_opts = opt;

	for (i=0, opt=0; i<MAXOPTS; ++i) {
		getsval("mem%dname", mem_opt[opt].name);
		if (valbuf[0] == 0) continue;

		getsval("mem%dmtype", mem_opt[opt].mtype);
		getdval("mem%dmb", mem_opt[opt].mb);
		getdval("mem%dgbs", mem_opt[opt].gbs);
		getdval("mem%dcost", mem_opt[opt].cost);
		mem_opt[opt].estat = 0;
		opt++;
	}
	mem_opts = opt;

	for (i=0, opt=0; i<MAXOPTS; ++i) {
		getsval("disk%dname", disk_opt[opt].name);
		if (valbuf[0] == 0) continue;

		getdval("disk%dgb", disk_opt[opt].gb);
		getdval("disk%dcost", disk_opt[opt].cost);
		disk_opt[opt].estat = 0;
		opt++;
	}
	disk_opts = opt;

	for (i=0, opt=0; i<MAXOPTS; ++i) {
		getsval("kase%dname", kase_opt[opt].name);
		if (valbuf[0] == 0) continue;

		getsval("kase%dctype", kase_opt[opt].ctype);
		getsval("kase%drtype", kase_opt[opt].rtype);
		getival("kase%dide", kase_opt[opt].ide);
		getdval("kase%dcost", kase_opt[opt].cost);
		getival("kase%du", kase_opt[opt].u);
		getdval("kase%damps", kase_opt[opt].amps);
		kase_opt[opt].estat = 0;
		opt++;
	}
	kase_opts = opt;

	for (i=0, opt=0; i<MAXOPTS; ++i) {
		getsval("rack%dname", rack_opt[opt].name);
		if (valbuf[0] == 0) continue;

		getsval("rack%drtype", rack_opt[opt].rtype);
		getdval("rack%dcost", rack_opt[opt].cost);
		getival("rack%du", rack_opt[opt].u);
		getival("rack%dfloor", rack_opt[opt].floor);
		rack_opt[opt].estat = 0;
		opt++;
	}
	rack_opts = opt;

	for (i=0, opt=0; i<MAXOPTS; ++i) {
		getsval("bundle%dname", bundle_opt[opt].name);
		if (valbuf[0] == '\0') continue;

		getival("bundle%dnic", bundle_opt[opt].nic);
		getival("bundle%dnic_qty", bundle_opt[opt].nic_qty);
		getival("bundle%dcab", bundle_opt[opt].cab);
		getival("bundle%dcab_qty", bundle_opt[opt].cab_qty);
		getival("bundle%dsw", bundle_opt[opt].sw);
		getival("bundle%dsw_qty", bundle_opt[opt].sw_qty);
		getival("bundle%dproc", bundle_opt[opt].proc);
		getival("bundle%dproc_qty", bundle_opt[opt].proc_qty);
		getival("bundle%dmom", bundle_opt[opt].mom);
		getival("bundle%dmom_qty", bundle_opt[opt].mom_qty);
		getival("bundle%dmem", bundle_opt[opt].mem);
		getival("bundle%dmem_qty", bundle_opt[opt].mem_qty);
		getival("bundle%ddisk", bundle_opt[opt].disk);
		getival("bundle%ddisk_qty", bundle_opt[opt].disk_qty);
		getival("bundle%dkase", bundle_opt[opt].kase);
		getival("bundle%dkase_qty", bundle_opt[opt].kase_qty);
		getival("bundle%drack", bundle_opt[opt].rack);
		getival("bundle%drack_qty", bundle_opt[opt].rack_qty);
		getdval("bundle%dcost", bundle_opt[opt].cost);
		opt++;
	}
	bundle_opts = opt;

	mom_bund = 0;
	FIND_BUND(sw,   mom);
	FIND_BUND(nic,  sw);
	FIND_BUND(cab,  nic);
	FIND_BUND(proc, cab);
	FIND_BUND(mem,  proc);
	FIND_BUND(kase, mem);
	FIND_BUND(rack, kase);
	FIND_BUND(disk, rack);

	set_availability();
}

void parse_limit_strs(void)
{

	include_strs = count_words(include_buf);
	exclude_strs = count_words(exclude_buf);
	if (include_strs == 0 && exclude_strs == 0) {
		limit = 0;
		return;
	}
	limit = 1;
	if ((include_str = (char **)malloc(sizeof(char *) * include_strs)) == NULL) {
		include_logic = LIMIT_ERR_OOM;
		limit = 0;
		return;
	}
	if ((exclude_str = (char **)malloc(sizeof(char *) * exclude_strs)) == NULL) {
		free(include_str);
		include_logic = LIMIT_ERR_OOM;
		limit = 0;
		return;
	}
	parse_buf(include_buf, include_strs, include_str);
	parse_buf(exclude_buf, exclude_strs, exclude_str);
}

inline char *skip_space(char *buf)
{
	while(isspace(*buf)) {
		buf++;
	}
	return buf;
}

char *word_end(char *buf) 
{
	int in_quote = 0;
	
	while((*buf != '\0') && (!isspace(*buf) || in_quote)) {
		/* get rid of branch, just to be perverse... */
		/* in_quote = (*buf == '"') ? !in_quote : in_quote; */
		in_quote = (in_quote != (*buf == '"'));
		buf++;
	}

	return buf;
}

int count_words(char *buf)
{
	int   words;

	words = 0;
	if (buf != NULL) {
		buf = skip_space(buf);
		while(*buf != '\0') {
			buf = word_end(buf);
			words++;
			buf = skip_space(buf);
		}
	}
	
	return words;
}

void remove_quotes(char *str)
{
	char *dest;

	dest = index(str, '"');
	if (dest == NULL)
		return;

	str = dest+1;
	while (*str != '\0') {
		if (*str != '"') {
			*dest++ = *str;
		}
		str++;
	}
	*dest = '\0';
}

void parse_buf(char *buf, int wordc, char **wordv)
{
	int words;
	char *end;

	if (buf != NULL) {
		words = 0;
		buf = skip_space(buf);
		while(*buf != '\0') {
			*wordv = buf;
			wordv++;
			end = word_end(buf);
			words++;
			if (*end != '\0') {
				*end = '\0';
				end++;
				end = skip_space(end);
			}
			remove_quotes(buf);
			buf = end;
		}
		assert(words == wordc);
	}
	return;
}

#define nic_str_match(fld,str) { \
	int nic; \
	for (nic = 0 ; nic < nic_opts ; nic++) { \
		nic_opt[nic].fld = nic_opt[nic].fld || strstr(nic_opt[nic].name, (str)) || strstr(nic_opt[nic].ntype, (str)); \
	} \
}

#define cab_str_match(fld,str) {\
	int cab; \
	for (cab = 0 ; cab < cab_opts ; cab++) { \
		cab_opt[cab].fld = cab_opt[cab].fld || strstr(cab_opt[cab].name, (str)) || strstr(cab_opt[cab].ntype, (str)); \
	} \
}

#define sw_str_match(fld,str) {\
	int sw; \
	for (sw = 0 ; sw < sw_opts ; sw++) { \
		sw_opt[sw].fld = sw_opt[sw].fld || strstr(sw_opt[sw].name, (str)) || strstr(sw_opt[sw].ntype, (str)); \
	} \
}

#define proc_str_match(fld,str) {\
	int proc; \
	for (proc = 0 ; proc < proc_opts ; proc++) { \
		proc_opt[proc].fld = proc_opt[proc].fld || strstr(proc_opt[proc].name, (str)) || strstr(proc_opt[proc].ptype, (str)); \
	} \
}

#define mom_str_match(fld,str) {\
	int mom; \
	for (mom = 0 ; mom < mom_opts ; mom++) { \
		mom_opt[mom].fld = mom_opt[mom].fld || strstr(mom_opt[mom].name, str) || strstr(mom_opt[mom].ptype, (str)) \
					|| strstr(mom_opt[mom].mtype, (str)) || strstr(mom_opt[mom].ctype, (str)); \
	} \
}

#define mem_str_match(fld,str) {\
	int mem; \
	for (mem = 0 ; mem < mem_opts ; mem++) { \
		mem_opt[mem].fld = mem_opt[mem].fld || strstr(mem_opt[mem].name, (str)) || strstr(mem_opt[mem].mtype, (str)); \
	} \
}

#define disk_str_match(fld,str) {\
	int disk; \
	for (disk = 0 ; disk < disk_opts ; disk++) { \
		disk_opt[disk].fld = disk_opt[disk].fld || strstr(disk_opt[disk].name, (str)) != NULL; \
	} \
}

#define kase_str_match(fld,str) { \
	int kase; \
	for (kase = 0 ; kase < kase_opts ; kase++) { \
		kase_opt[kase].fld = kase_opt[kase].fld || strstr(kase_opt[kase].name, (str)) || strstr(kase_opt[kase].ctype, (str)) \
			|| strstr(kase_opt[kase].rtype, (str)); \
	} \
}

#define rack_str_match(fld,str) {\
	int rack; \
	for (rack = 0 ; rack < rack_opts ; rack++) { \
		rack_opt[rack].fld = rack_opt[rack].fld || strstr(rack_opt[rack].name, (str)) || strstr(rack_opt[rack].rtype, (str)); \
	} \
}

#define bundle_str_match(fld,str) { \
	int bundle; \
	for(bundle = 0 ; bundle < bundle_opts ; bundle++) { \
		bundle_opt[bundle].fld = bundle_opt[bundle].fld \
		  || strstr(bundle_opt[bundle].name, (str)) \
		  || strstr(nic_opt[bundle_opt[bundle].nic].name, (str)) \
		  || strstr(cab_opt[bundle_opt[bundle].cab].name, (str)) \
		  || strstr(sw_opt[bundle_opt[bundle].sw].name, (str)) \
		  || strstr(proc_opt[bundle_opt[bundle].proc].name, (str)) \
		  || strstr(mom_opt[bundle_opt[bundle].mom].name, (str)) \
		  || strstr(mem_opt[bundle_opt[bundle].mem].name, (str)) \
		  || strstr(disk_opt[bundle_opt[bundle].disk].name, (str)) \
		  || strstr(kase_opt[bundle_opt[bundle].kase].name, (str)) \
		  || strstr(rack_opt[bundle_opt[bundle].rack].name, (str)) ;\
	  }\
}

#define opt_init(opt,fld,val) { \
	int i; \
	for (i = 0 ; i < opt##_opts ; i++) { \
		opt##_opt[i].fld = (val); \
	} \
}

void set_availability(void)
{
	register int i;

	/* mark everything as available in unlimited quantity */
	for (i = 0 ; i < nic_opts ; i++) {
		nic_opt[i].available = QUANTITY_UNLIMITED;
	}
	for (i = 0 ; i < cab_opts ; i++) {
		cab_opt[i].available = QUANTITY_UNLIMITED; 
	}
	for (i = 0 ; i < sw_opts ; i++) {
		sw_opt[i].available = QUANTITY_UNLIMITED;
	}
	for (i = 0 ; i < proc_opts ; i++) {
		proc_opt[i].available = QUANTITY_UNLIMITED;
	}
	for (i = 0 ; i < mom_opts ; i++) {
		mom_opt[i].available = QUANTITY_UNLIMITED;
	}
	for (i = 0 ; i < mem_opts ; i++) {
		mem_opt[i].available = QUANTITY_UNLIMITED;
	}
	for (i = 0 ; i < disk_opts ; i++) {
		disk_opt[i].available = QUANTITY_UNLIMITED;
	}
	for (i = 0 ; i < kase_opts ; i++) {
		kase_opt[i].available = QUANTITY_UNLIMITED;
	}
	for (i = 0 ; i < rack_opts ; i++) {
		rack_opt[i].available = QUANTITY_UNLIMITED;
	}
	for (i = 0 ; i < bundle_opts ; i++) {
		bundle_opt[i].available = QUANTITY_UNLIMITED;
	}

	opt_init(nic, include, 0);
	opt_init(cab, include, 0);
	opt_init(sw, include, 0);
	opt_init(proc, include, 0);
	opt_init(mom, include, 0);
	opt_init(mem, include, 0);
	opt_init(disk, include, 0);
	opt_init(kase, include, 0);
	opt_init(rack, include, 0);
	opt_init(bundle, include, 0);

	opt_init(nic, exclude, 0);
	opt_init(cab, exclude, 0);
	opt_init(sw, exclude, 0);
	opt_init(proc, exclude, 0);
	opt_init(mom, exclude, 0);
	opt_init(mem, exclude, 0);
	opt_init(disk, exclude, 0);
	opt_init(kase, exclude, 0);
	opt_init(rack, exclude, 0);
	opt_init(bundle, exclude, 0);
	if (limit) {
		for (i = 0 ; i < include_strs ; i++) {
			nic_str_match(include, include_str[i]);
			cab_str_match(include, include_str[i]);
			sw_str_match(include, include_str[i]);
			proc_str_match(include, include_str[i]);
			mom_str_match(include, include_str[i]);
			mem_str_match(include, include_str[i]);
			disk_str_match(include, include_str[i]);
			kase_str_match(include, include_str[i]);
			rack_str_match(include, include_str[i]);
			bundle_str_match(include, include_str[i]);
		}
		for (i = 0 ; i < exclude_strs ; i++) {
			nic_str_match(exclude, exclude_str[i]);
			cab_str_match(exclude, exclude_str[i]);
			sw_str_match(exclude, exclude_str[i]);
			proc_str_match(exclude, exclude_str[i]);
			mom_str_match(exclude, exclude_str[i]);
			mem_str_match(exclude, exclude_str[i]);
			disk_str_match(exclude, exclude_str[i]);
			kase_str_match(exclude, exclude_str[i]);
			rack_str_match(exclude, exclude_str[i]);
			bundle_str_match(exclude, exclude_str[i]);
		}
	}
}
