/*	rules.h
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "cgic.h"


#define	VERSION		20120226
#define CDR_EXEC	"fawncalc.cgi"
#define CDR_EMAIL	"william@a9group.net"
#define LOGO_URL	"http://beta.a9group.net/fawncalc/fawncalc.png"
#define PARAM_LIFETIME	(180*24*60*60)   /* parameter sets live for 180 days */
#define	MAXOPTS		32		/* How many options for each part? */
#define	MAXDESIGNS	(32 * 1024)	/* How many designs to keep? */
#define	MAXHTML		32		/* How many options detailed in HTML output? */
#define	MAX_SECS	30		/* Maximum seconds searching */
#define DEFAULT_PARAMS  0		/* default parameter set */

#define	NET_NONE	0		/* Network types... */
#define	NET_DIRECT	1
#define	NET_RING	(NET_DIRECT<<1)
#define	NET_2DMESH	(NET_RING<<1)
#define	NET_3DMESH	(NET_2DMESH<<1)
#define	NET_SWITCH	(NET_3DMESH<<1)
#define	NET_FABRIC	(NET_SWITCH<<1)
#define	NET_CB		(NET_FABRIC<<1)	/* Channel Bonded */
#define	NET_FNN		(NET_CB<<1)	/* Flat Neighborhood Network */
#define	NET_AFN		(NET_FNN<<1)	/* Aggregate Function Network */

#define	NET_FORWARD_US	100		/* Typical time to forward through a node */

#define	MAXLEN		4096		/* Maximum length of a string */
#define MAX_LIMIT_STR	MAXLEN

#define LIMIT_ERR_OOM	-1		/* ran out of memory */
#define LIMIT_ANY	0
#define LIMIT_ALL	1

#define QUANTITY_UNLIMITED	-1     /* an unlimited quantity of this part is available */

/* if you add a SA_xxx below be sure to update NUM_SA */
#define	SA_EVAL0	0
#define	SA_EVAL1	(SA_EVAL0+1)
#define	SA_EVAL2	(SA_EVAL1+1)
#define	SA_EVAL3	(SA_EVAL2+1)
#define	SA_NET0		(SA_EVAL3+1)
#define	SA_NET1		(SA_NET0+1)
#define	SA_CAB0		(SA_NET1+1)
#define	SA_CAB1		(SA_CAB0+1)
#define	SA_PROC0	(SA_CAB1+1)
#define	SA_PROC1	(SA_PROC0+1)
#define	SA_PROC2	(SA_PROC1+1)
#define	SA_PROC3	(SA_PROC2+1)
#define	SA_PROC4	(SA_PROC3+1)
#define	SA_MEM0		(SA_PROC4+1)
#define	SA_MEM1		(SA_MEM0+1)
#define	SA_MEM2		(SA_MEM1+1)
#define	SA_MEM3		(SA_MEM2+1)
#define	SA_MEM4		(SA_MEM3+1)
#define	SA_KASE0	(SA_MEM4+1)
#define	SA_AMPS0	(SA_KASE0+1)
#define	SA_TONS0	(SA_AMPS0+1)
#define	SA_RACK0	(SA_TONS0+1)
#define	SA_RACK1	(SA_RACK0+1)
#define	SA_RACK2	(SA_RACK1+1)
#define	SA_DISK0	(SA_RACK2+1)
#define	SA_DISK1	(SA_DISK0+1)
#define	SA_DISK2	(SA_DISK1+1)
#define	SA_COST0	(SA_DISK2+1)

#define NUM_SA		(SA_COST0+1)

#define err_exit(fmt...) { \
	fprintf(cgiOut, "<H2>Internal Error</H2>\n<p>\n"); \
	fprintf(cgiOut, fmt); \
	exit(-1); \
}

/* what action the cgi should take */
typedef enum {
	ENTER_DESIGN,		/* enter a design (memgbs, etc) */
	EVAL_DESIGN,		/* evaluate a design */
	LOGIN,		 	/* enter login and password to edit params */
	EDIT_PARAMS,		/* edit internal parameters */
	VIEW_PARAMS,		/* print a parameter set on a page */
	SET_PARAMS,		/* set internal parameters */
	MORE_OPTS,		/* give me more of some kind of entry */
	NEW_USER,		/* create a new user account */
	NEW_USER_FORM,		/* create a new user account */
	PRINT_FORM		/* just print the design entry form */
} cdrop_t;

typedef struct {
	char	*name;		/* part name */
	char	*ntype;		/* network type */
	double	mbs;		/* how many Mb/s */
	double	us;		/* how many us latency */
	double	cost;		/* cost per part */
	double	estat;		/* eval stat */
	int	available;	/* is this nic type available (in stock) */
	int	include:1;	/* include this nic because of a string? */
	int	exclude:1;	/* exclude this nic because of a string? */
} nict;

typedef struct {
	char	*name;		/* part name */
	char	*ntype;		/* network type */
	double	feet;		/* how many feet long */
	double	cost;		/* cost per part */
	double	estat;		/* eval stat */
	int	available;	/* is this cable type available (in stock) */
	int	include:1;	/* include this cab because of a string? */
	int	exclude:1;	/* exclude this cab because of a string? */
} cabt;

typedef struct {
	char	*name;		/* part name */
	char	*ntype;		/* network type */
	double	mbs;		/* how many Mb/s bisect */
	double	us;		/* how many us latency */
	int	ports;		/* how many ports */
	double	cost;		/* cost per part */
	int	u;		/* how many U worth of space */
	double	amps;		/* how many amps of power */
	double	estat;		/* eval stat */
	int	available;	/* is this switch type available (in stock) */
	int	include:1;	/* include this switch because of a string? */
	int	exclude:1;	/* exclude this switch because of a string? */
} swt;

typedef struct {
	char	*name;		/* part name */
	char	*ptype;		/* processor type */
	double	gflops;		/* how many GFLOPS/part */
	double	cost;		/* cost per part */
	double	estat;		/* eval stat */
	int	available;	/* is this processor type available (in stock) */
	int	include:1;	/* include this proc because of a string? */
	int	exclude:1;	/* include this proc because of a string? */
} proct;

typedef struct {
	char	*name;		/* part name */
	char	*ptype;		/* processor type accepted */
	char	*mtype;		/* memory type accepted */
	char	*ctype;		/* kase type this fits in */
	int	n;		/* how many processors? */
	int	mem;		/* how many mem slots? */
	int	pci;		/* how many PCI slots? */
	int	ide;		/* how many IDE disks? */
	int	pxe;		/* supports PXE */
	double	cost;		/* cost per part */
	double	estat;		/* eval stat */
	int	available;	/* is this motherboard type available (in stock) */
	int	include:1;	/* include this motherboard because of a string? */
	int	exclude:1;	/* exclude this motherboard because of a string? */
} momt;

typedef struct {
	char	*name;		/* part name */
	char	*mtype;		/* memory type */
	double	mb;		/* how many MB/part */
	double	gbs;		/* how many GB/s bandwidth */
	double	cost;		/* cost per part */
	double	estat;		/* eval stat */
	int	available;	/* is this memory part type available (in stock) */
	int	include:1;	/* include this memory part because of a string? */
	int	exclude:1;	/* exclude this memory part because of a string? */
} memt;

typedef struct {
	char	*name;		/* part name */
	double	gb;		/* how many read/write GB/part */
	double	cost;		/* cost per part */
	double	estat;		/* eval stat */
	int	available;	/* is this disk type available (in stock) */
	int	include:1;	/* include this disk because of string? */
	int	exclude:1;	/* exclude this disk because of string? */
} diskt;

typedef struct {
	char	*name;		/* part name */
	char	*ctype;		/* kase type */
	char	*rtype;		/* rack type */
	int	ide;		/* how many IDE disks fit? */
	double	cost;		/* cost per part */
	int	u;		/* how many U worth of space */
	double	amps;		/* how many amps of power */
	double	estat;		/* eval stat */
	int	available;	/* include this kase time in the results */
	int	include:1;	/* include this kase because of a string? */
	int	exclude:1;	/* include this kase because of a string? */
} kaset;

typedef struct {
	char	*name;		/* part name */
	char	*rtype;		/* rack type must include kase */
	double	cost;		/* cost per part */
	int	u;		/* how many U worth of space */
	int	floor;		/* floorspace in 2x2 units */
	double	estat;		/* eval stat */
	int	available; 	/* is this rack type available (in stock) */
	int	include:1;	/* include this rack because of a string? */
	int	exclude:1;	/* exclude this rack because of a string? */
} rackt;

/* A bundle is a set of parts and a quantity of each part in the set */ 
typedef struct {
	char *name;		/* bundle name */
	int   nic;		/* NIC type (index into nic_opt array) */
	int   nic_qty;		/* quantity of nics */
	int   cab;		/* cable type (index into cab_opt array) */
	int   cab_qty;		/* number of cables */
	int   sw;		/* Switch type  */
	int   sw_qty;		/* number of switches */
	int   proc;		/* Processor type */
	int   proc_qty;		/* number of nics */
	int   mom;		/* motherboard type */
	int   mom_qty;		
	int   mem;		/* memory type */
	int   mem_qty;		
	int   disk;		/* disk type */
	int   disk_qty;		
	int   kase;		/* case type */
	int   kase_qty;		
	int   rack;		/* rack type */
	int   rack_qty;		
	double cost;		/* bundle cost */
	int   available;
	int   include:1;
	int   exclude:1;
} bundlet;

/* bundles included in the design */
typedef struct bund {
	int n;			/* number of instances of this bundle */
	int id;			/* index into bundle_opt */
} bundt;

/* parameters of the current design */
typedef struct design {
	int	n, nodes, spares;
	int	net_type, net_x, net_y, net_z;
	int	nic, cab, sw, sw2, proc, mom, mem, disk, kase, rack;
	/* number of individual components included in the design (i.e., components
	 * in addition to those available in bundles)
	 */
	int	nics, cabs, sws, sw2s, procs, moms, mems, disks, kases, racks;
	/* number of components required by the design */
	int	dnics, dcabs, dsws, dsw2s, dprocs, dmoms, dmems, ddisks, dkases, dracks;
	int     nics_avail, cabs_avail, sws_avail, sw2s_avail, procs_avail, moms_avail,
		mems_avail, disks_avail, kases_avail, racks_avail;
	int	nicpnode, procpmom, mempmom, diskpnode;
	int     bundles;
	bundt   bundle[MAXOPTS];
#ifdef DEBUG_METRIC_DISCARD
	int     discard;
#endif
	double	usrgflops, usrmem, usrdisk, usramps, usrtons, usrmemgbs;
	double	net_latency, net_bandwidth;
	double	cost;
	double	metric;
} designt;

/* struct with all the components needed to calculate a metric */
typedef struct metric {
	double	usrgflops, usrmem, usrdisk, usramps, usrtons, usrmemgbs;
	double	net_latency, net_bandwidth;
	int     procs;		/* # procs per node */
	struct cost {
		double nic, cab, sw, sw2, proc, mom, mem, disk, kase, rack;
	} cost;
} metric_t;

typedef unsigned long long uint64;

extern designt  best_unavailable;
extern designt	design[MAXDESIGNS + 3];
extern designt	*dp;

#ifdef DEBUG_METRIC_DISCARDS
extern int      bad_discard;
extern int      discards;
#endif

extern int	designs_ok;
extern int	designs_tried;
extern int	designs_limited;
extern int	stuck_at;
extern double	cheapest;

#ifdef DEBUG_NUM_TRIES
extern int      num_filter;
extern int	num_net;
extern int	num_cab;
extern int	num_proc;
extern int	num_mem;
extern int	num_kase;
extern int	num_disk;
extern int 	num_done;
#endif /* DEBUG_NUM_TRIES */ 

extern int	version;
extern int	timeout;

extern int      paramset;
extern cdrop_t  cdr_op;
extern char    *email;
extern char    *password;
extern char    *password2;
extern int	g_uid;
extern int      oread;
extern int      oeval;

extern double	ossize;
extern double	codesize;
extern double	datasize;
extern double	minmemgbs;
extern double	disksize;
extern int	swapsize;
extern int	useraid;
extern int	usebackup;
extern double	maxlatency;
extern double	afnlatency;
extern double	minbandwidth;
extern int	coordinality;
extern int	nodecon0;
extern int	nodecon1;
extern int	nodecon2;
extern int	sparen0;
extern int	sparen1;
extern int	racks;
extern double	amps;
extern double	tons;
extern int	video;
extern double	budget;
extern double	gflops;
extern int	howmany;

extern double	metmem;
extern double	metmemgbs;
extern double	metdisk;
extern double	metlat;
extern double	metbw;
extern double	metcost;
extern double	metgflops;
extern double	metamps;
extern double	mettons;

extern int	limit;
extern int	include_logic;
extern char   **include_str;
extern int      include_strs;
extern int	exclude_logic;
extern char   **exclude_str;
extern int      exclude_strs;

extern char    *dbname;
extern char    *dbemail;

extern char    *evalcomment;
extern char    *viewcomment;

extern nict	nic_opt[MAXOPTS];
extern int	nic_opts;

extern cabt	cab_opt[MAXOPTS];
extern int	cab_opts;

extern swt	sw_opt[MAXOPTS];
extern int	sw_opts;

extern proct	proc_opt[MAXOPTS];
extern int	proc_opts;

extern momt	mom_opt[MAXOPTS];
extern int	mom_opts;

extern memt	mem_opt[MAXOPTS];
extern int	mem_opts;

extern diskt	disk_opt[MAXOPTS];
extern int	disk_opts;

extern kaset	kase_opt[MAXOPTS];
extern int	kase_opts;

extern rackt	rack_opt[MAXOPTS];
extern int	rack_opts;

extern bundlet  bundle_opt[MAXOPTS];
extern int      bundle_opts;

extern int     	mom_bund;	/* first bundle with mom as first part */
extern int     	sw_bund;	/* first bundle with switch as first part */
extern int     	nic_bund;	/* first bundle with nic as first part */
extern int     	cab_bund;	/* ... */
extern int     	proc_bund;
extern int     	mem_bund;
extern int	kase_bund;
extern int	rack_bund;
extern int	disk_bund;	/* first bundle with disk as first part */


extern char	*plot(void);

extern int	searchsecs;
extern int	searchdone;

extern volatile int	timeup;
extern void	set_timeout(void);

extern int	max_search_nodes;
extern int	min_search_nodes;

void	get_cgi_args(void);
void    get_cgi_db_args(void);

/*********************************************************************************/
/*                              Design Evaluation                                */
/*********************************************************************************/
void	eval(void);
int	nics4fnn(register int nodes, register int ports);

/*********************************************************************************/
/*                              HTML Output                                      */
/*********************************************************************************/
void	out_html(void);
void    out_version(void);
void 	print_eval_form(void);
void	print_options(void);
void	print_params(void);

void 	edit_params(void);

/*********************************************************************************/
/*                              User Management                                  */
/*********************************************************************************/
int     new_user(void);
void	new_user_form(void);
int	get_passwd(char *email, char *buf, int size);
int	validate_login(char *email, char *passwd);
void	invalid_login(void);
int     get_uid(char *email);
void 	eval_denied(void);
int     update_expiration(char *email);
int 	is_expired(char *pw_entry);

/*********************************************************************************/
/*                              File Management                                  */
/*********************************************************************************/
int	save_db(void);
FILE   *fopen_db(int uid, char *mode);
int 	touch_db(int uid);
