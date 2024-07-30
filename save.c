/*
 * save.c
 *
 * Save database data to a file.
 *
 * Created by William R. Dieter, September 2002
 *
 * History
 * -------
 * $Log: save.c,v $
 * Revision 1.13  2004/06/22 20:59:42  wrdieter
 * Added bundle support.  Works for multiple single components (value
 * packs), but not components of different types.
 *
 * Revision 1.12  2004/06/07 20:19:49  wrdieter
 * Added I/O for bundles
 *
 * Revision 1.11  2004/06/04 16:32:50  wrdieter
 * Added bundle entry and database management, but no evaluation yet.
 *
 * Revision 1.10  2004/01/26 06:23:30  wrdieter
 * Added PXE field for motherboards.  No disk drive is required if PXE is present.
 *
 * Revision 1.9  2003/11/18 04:56:28  wrdieter
 * Added headers to work with Linux.
 * Added configure and Makefile
 *
 * Revision 1.8  2002/10/30 01:05:55  kaos
 * Added eval and view comments.
 * Fixed a bug in the network eval function that caused a hang when
 * evaluating single node designs.  When the network eval was
 * split into separate functions a return statement moved into the
 * function with e_nonet.  This error caused other types of networks
 * to be evaluated for 1 node, which caused a hang.
 *
 * Revision 1.7  2002/10/17 14:41:08  kaos
 * Sort form entries before saving them.
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
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>

#include "cgic.h"
#include "email.h"
#include "rules.h"
#include "util.h"

/* kinda big to be a macro, but the name stuff works more easily this way */
#define db_s_entry(name,idx,value) { \
	char *valbuf; \
	char *space; \
	valbuf = strdup(value); \
	while((space = index(valbuf, ' '))) *space = '+'; \
	fprintf(dbOut, "&"name"=%s", idx, valbuf); \
        free(valbuf); \
}

#define db_d_entry(name,index,value) \
	fprintf(dbOut, "&"name"=%g", index, value);

#define db_i_entry(name,index,value) \
	fprintf(dbOut, "&"name"=%d", index, value);


#define COMPARE_STR_FN(component,field) \
static int compare_##component##_##field(const void *c1, const void *c2) { \
	return strcmp(((const component##t *)c1)->field, ((const component##t *)c2)->field); \
} 

#define COMPARE_NUM_FN(component,field) \
static int compare_##component##_##field(const void *c1, const void *c2) { \
	return (((const component##t *)c1)->field > ((const component##t *)c2)->field) - (((const component##t *)c1)->field < ((const component##t *)c2)->field); \
} 

#define COMPARE_NUM_REV_FN(component,field) \
static int compare_##component##_##field(const void *c1, const void *c2) { \
	return (((const component##t *)c2)->field - ((const component##t *)c1)->field); \
} 

static void save_comments(FILE *dbOut);
static void save_nics(FILE *dbOut);
static void save_cabs(FILE *dbOut);
static void save_sw(FILE *dbOut);
static void save_proc(FILE *dbOut);
static void save_moms(FILE *dbOut);
static void save_mems(FILE *dbOut);
static void save_disks(FILE *dbOut);
static void save_kases(FILE *dbOut);
static void save_racks(FILE *dbOut);
static void save_bundles(FILE *dbOut);

FILE *fopen_db(int uid, char *mode)
{
	char buf[MAXLEN];
	
	snprintf(buf, MAXLEN, "db%d.cgtxt", uid);
	return fopen(buf, mode);
}

int touch_db(int uid)
{
	FILE *fp;

	if ((fp = fopen_db(uid, "w")) == NULL) {
		return -1;
	}
	return fclose(fp);
}

/* save the database */
int save_db(void)
{
	int      uid;
	FILE    *dbOut;
	char     db_html_name[MAXLEN];
	email_t *note;

	uid = get_uid(email);
	if ((dbOut = fopen_db(uid, "w")) == NULL) {
		err_exit("Error opening database for writing\n");
	}

	if (flock(fileno(dbOut), LOCK_SH) < 0) {
		err_exit("Cannot lock database for uid %d\n", uid);
	}
	strncpy(db_html_name, dbname, MAXLEN);
	strsubs(db_html_name, ' ', '+');
	fprintf(dbOut, "dbname=%s&dbemail=%s", db_html_name, email);
	if (oeval)
		fprintf(dbOut, "&oeval=%d", oeval);
	if (oread)
		fprintf(dbOut, "&oread=%d", oread);
	save_comments(dbOut);
	save_nics(dbOut);
	save_cabs(dbOut);
	save_sw(dbOut);
	save_proc(dbOut);
	save_moms(dbOut);
	save_mems(dbOut);
	save_disks(dbOut);
	save_kases(dbOut);
	save_racks(dbOut);
	save_bundles(dbOut);
	if (flock(fileno(dbOut), LOCK_UN) < 0) {
		err_exit("Cannot unlock database for uid %d\n", uid);
	}
	fclose(dbOut);
	update_expiration(email);
	if ((note = email_start(CDR_EMAIL, email, "[CDR] Configuration Saved")) == NULL) {
		err_exit("Could not open email");
	}
	email_printf(note,
		     "Changes to your parameter set were saved.\n"
		     "Your current parameter set will expire %g days\n"
		     "after the last time it is modified.  Update\n"
		     "your parameter set to prevent it from expiring.\n",
		     ((double)PARAM_LIFETIME) / (24.0 * 60.0 * 60.0));
	email_end(note);
	return 0;
}

COMPARE_STR_FN(nic,name)
COMPARE_STR_FN(nic,ntype)
COMPARE_NUM_FN(nic,us)
COMPARE_NUM_FN(nic,mbs)
COMPARE_NUM_FN(nic,cost)

static void save_comments(FILE *dbOut)
{
	fprintf(dbOut, "&evalcomment=%s", evalcomment);
	fprintf(dbOut, "&viewcomment=%s", viewcomment);
}

static void save_nics(FILE *dbOut)
{
	int  nic;

	qsort(nic_opt, nic_opts, sizeof(nict), compare_nic_name);
	mergesort(nic_opt, nic_opts, sizeof(nict), compare_nic_cost);
	mergesort(nic_opt, nic_opts, sizeof(nict), compare_nic_mbs);
	mergesort(nic_opt, nic_opts, sizeof(nict), compare_nic_us);
	mergesort(nic_opt, nic_opts, sizeof(nict), compare_nic_ntype);

	for (nic = 0 ; nic < nic_opts ; nic++) {
		db_s_entry("nic%dname", nic, nic_opt[nic].name);
		db_s_entry("nic%dntype", nic, nic_opt[nic].ntype);
		db_d_entry("nic%dmbs", nic, nic_opt[nic].mbs);
		db_d_entry("nic%dus", nic, nic_opt[nic].us);
		db_d_entry("nic%dcost", nic, nic_opt[nic].cost);
	}
}

COMPARE_STR_FN(cab,name)
COMPARE_STR_FN(cab,ntype)
COMPARE_NUM_FN(cab,feet)
COMPARE_NUM_FN(cab,cost)

static void save_cabs(FILE *dbOut)
{
	int cab;

	qsort(cab_opt, cab_opts, sizeof(cabt), compare_cab_name);
	mergesort(cab_opt, cab_opts, sizeof(cabt), compare_cab_cost);
	mergesort(cab_opt, cab_opts, sizeof(cabt), compare_cab_feet);
	mergesort(cab_opt, cab_opts, sizeof(cabt), compare_cab_ntype);

	for (cab = 0 ; cab < cab_opts ; cab++) {
		db_s_entry("cab%dname", cab, cab_opt[cab].name);
		db_s_entry("cab%dntype", cab, cab_opt[cab].ntype);
		db_d_entry("cab%dfeet", cab, cab_opt[cab].feet);
		db_d_entry("cab%dcost", cab, cab_opt[cab].cost);
	}
}

COMPARE_STR_FN(sw,name)
COMPARE_STR_FN(sw,ntype)
COMPARE_NUM_FN(sw,mbs)
COMPARE_NUM_FN(sw,us)
COMPARE_NUM_FN(sw,ports)
COMPARE_NUM_FN(sw,cost)
COMPARE_NUM_FN(sw,u)
COMPARE_NUM_FN(sw,amps)

static void save_sw(FILE *dbOut)
{
	int sw;

	qsort(sw_opt, sw_opts, sizeof(swt), compare_sw_name);
	mergesort(sw_opt, sw_opts, sizeof(swt), compare_sw_amps);
	mergesort(sw_opt, sw_opts, sizeof(swt), compare_sw_u);
	mergesort(sw_opt, sw_opts, sizeof(swt), compare_sw_cost);
	mergesort(sw_opt, sw_opts, sizeof(swt), compare_sw_mbs);
	mergesort(sw_opt, sw_opts, sizeof(swt), compare_sw_us);
	mergesort(sw_opt, sw_opts, sizeof(swt), compare_sw_ports);
	mergesort(sw_opt, sw_opts, sizeof(swt), compare_sw_ntype);

	for (sw = 0 ; sw < sw_opts ; sw++) {
		db_s_entry("sw%dname", sw, sw_opt[sw].name);
		db_s_entry("sw%dntype", sw, sw_opt[sw].ntype);
		db_d_entry("sw%dmbs", sw, sw_opt[sw].mbs);
		db_d_entry("sw%dus", sw, sw_opt[sw].us);
		db_i_entry("sw%dports", sw, sw_opt[sw].ports);
		db_d_entry("sw%dcost", sw, sw_opt[sw].cost);
		db_i_entry("sw%du", sw, sw_opt[sw].u);
		db_d_entry("sw%damps", sw, sw_opt[sw].amps);
	}
}

COMPARE_STR_FN(proc,name)
COMPARE_STR_FN(proc,ptype)
COMPARE_NUM_FN(proc,gflops)
COMPARE_NUM_FN(proc,cost)

static void save_proc(FILE *dbOut)
{
	int proc;

	qsort(proc_opt, proc_opts, sizeof(proct), compare_proc_name);
	mergesort(proc_opt, proc_opts, sizeof(proct), compare_proc_cost);
	mergesort(proc_opt, proc_opts, sizeof(proct), compare_proc_gflops);
	mergesort(proc_opt, proc_opts, sizeof(proct), compare_proc_ptype);

	for (proc = 0 ; proc < proc_opts ; proc++) {
		db_s_entry("proc%dname", proc, proc_opt[proc].name);
		db_s_entry("proc%dptype", proc, proc_opt[proc].ptype);
		db_d_entry("proc%dgflops", proc, proc_opt[proc].gflops);
		db_d_entry("proc%dcost", proc, proc_opt[proc].cost);
	}
}

COMPARE_STR_FN(mom,name)
COMPARE_STR_FN(mom,ptype)
COMPARE_STR_FN(mom,mtype)
COMPARE_STR_FN(mom,ctype)
COMPARE_NUM_FN(mom,n)
COMPARE_NUM_FN(mom,mem)
COMPARE_NUM_FN(mom,pci)
COMPARE_NUM_FN(mom,ide)
COMPARE_NUM_FN(mom,cost)

static void save_moms(FILE *dbOut)
{
	int mom;

	qsort(mom_opt, mom_opts, sizeof(momt), compare_mom_name);
	mergesort(mom_opt, mom_opts, sizeof(momt), compare_mom_cost);
	mergesort(mom_opt, mom_opts, sizeof(momt), compare_mom_ide);
	mergesort(mom_opt, mom_opts, sizeof(momt), compare_mom_pci);
	mergesort(mom_opt, mom_opts, sizeof(momt), compare_mom_mem);
	mergesort(mom_opt, mom_opts, sizeof(momt), compare_mom_n);
	mergesort(mom_opt, mom_opts, sizeof(momt), compare_mom_ctype);
	mergesort(mom_opt, mom_opts, sizeof(momt), compare_mom_mtype);
	mergesort(mom_opt, mom_opts, sizeof(momt), compare_mom_ptype);

	for (mom = 0 ; mom < mom_opts ; mom++) {
		db_s_entry("mom%dname", mom, mom_opt[mom].name);
		db_s_entry("mom%dptype", mom, mom_opt[mom].ptype);
		db_s_entry("mom%dmtype", mom, mom_opt[mom].mtype);
		db_s_entry("mom%dctype", mom, mom_opt[mom].ctype);
		db_i_entry("mom%dn", mom, mom_opt[mom].n);
		db_i_entry("mom%dmem", mom, mom_opt[mom].mem);
		db_i_entry("mom%dpci", mom, mom_opt[mom].pci);
		db_i_entry("mom%dide", mom, mom_opt[mom].ide);
		db_s_entry("mom%dpxe", mom, mom_opt[mom].pxe ? "Y" : "N");
		db_d_entry("mom%dcost", mom, mom_opt[mom].cost);
	}
}

COMPARE_STR_FN(mem,name)
COMPARE_STR_FN(mem,mtype)
COMPARE_NUM_FN(mem,mb)
COMPARE_NUM_FN(mem,gbs)
COMPARE_NUM_FN(mem,cost)

static void save_mems(FILE *dbOut)
{
	int mem;

	qsort(mem_opt, mem_opts, sizeof(memt), compare_mem_name);
	mergesort(mem_opt, mem_opts, sizeof(memt), compare_mem_cost);
	mergesort(mem_opt, mem_opts, sizeof(memt), compare_mem_mb);
	mergesort(mem_opt, mem_opts, sizeof(memt), compare_mem_gbs);
	mergesort(mem_opt, mem_opts, sizeof(memt), compare_mem_mtype);

	for (mem = 0 ; mem < mem_opts ; mem++) {
		db_s_entry("mem%dname", mem, mem_opt[mem].name);
		db_s_entry("mem%dmtype", mem, mem_opt[mem].mtype);
		db_d_entry("mem%dmb", mem, mem_opt[mem].mb);
		db_d_entry("mem%dgbs", mem, mem_opt[mem].gbs);
		db_d_entry("mem%dcost", mem, mem_opt[mem].cost);
	}
}

COMPARE_STR_FN(disk,name)
COMPARE_NUM_FN(disk,gb)
COMPARE_NUM_FN(disk,cost)

static void save_disks(FILE *dbOut)
{
	int disk;

	qsort(disk_opt, disk_opts, sizeof(diskt), compare_disk_name);
	mergesort(disk_opt, disk_opts, sizeof(diskt), compare_disk_cost);
	mergesort(disk_opt, disk_opts, sizeof(diskt), compare_disk_gb);

	for (disk = 0 ; disk < disk_opts ; disk++) {
		db_s_entry("disk%dname", disk, disk_opt[disk].name);
		db_d_entry("disk%dgb", disk, disk_opt[disk].gb);
		db_d_entry("disk%dcost", disk, disk_opt[disk].cost);
	}
}

COMPARE_STR_FN(kase,name)
COMPARE_STR_FN(kase,ctype)
COMPARE_STR_FN(kase,rtype)
COMPARE_NUM_FN(kase,ide)
COMPARE_NUM_FN(kase,cost)
COMPARE_NUM_FN(kase,u)
COMPARE_NUM_FN(kase,amps)

static void save_kases(FILE *dbOut)
{
	int kase;

	qsort(kase_opt, kase_opts, sizeof(kaset), compare_kase_name);
	mergesort(kase_opt, kase_opts, sizeof(kaset), compare_kase_amps);
	mergesort(kase_opt, kase_opts, sizeof(kaset), compare_kase_cost);
	mergesort(kase_opt, kase_opts, sizeof(kaset), compare_kase_ide);
	mergesort(kase_opt, kase_opts, sizeof(kaset), compare_kase_u);
	mergesort(kase_opt, kase_opts, sizeof(kaset), compare_kase_rtype);
	mergesort(kase_opt, kase_opts, sizeof(kaset), compare_kase_ctype);

	for (kase = 0 ; kase < kase_opts ; kase++) {
		db_s_entry("kase%dname", kase, kase_opt[kase].name);
		db_s_entry("kase%dctype", kase, kase_opt[kase].ctype);
		db_s_entry("kase%drtype", kase, kase_opt[kase].rtype);
		db_i_entry("kase%dide", kase, kase_opt[kase].ide);
		db_d_entry("kase%dcost", kase, kase_opt[kase].cost);
		db_i_entry("kase%du", kase, kase_opt[kase].u);
		db_d_entry("kase%damps", kase, kase_opt[kase].amps);
	}
}

COMPARE_STR_FN(rack,name)
COMPARE_STR_FN(rack,rtype)
COMPARE_NUM_FN(rack,cost)
COMPARE_NUM_FN(rack,u)
COMPARE_NUM_FN(rack,floor)

static void save_racks(FILE *dbOut)
{
	int rack;

	qsort(rack_opt, rack_opts, sizeof(rackt), compare_rack_name);
	mergesort(rack_opt, rack_opts, sizeof(rackt), compare_rack_floor);
	mergesort(rack_opt, rack_opts, sizeof(rackt), compare_rack_u);
	mergesort(rack_opt, rack_opts, sizeof(rackt), compare_rack_cost);
	mergesort(rack_opt, rack_opts, sizeof(rackt), compare_rack_rtype);

	for (rack = 0 ; rack < rack_opts ; rack++) {
		db_s_entry("rack%dname", rack, rack_opt[rack].name);
		db_s_entry("rack%drtype", rack, rack_opt[rack].rtype);
		db_d_entry("rack%dcost", rack, rack_opt[rack].cost);
		db_i_entry("rack%du", rack, rack_opt[rack].u);
		db_i_entry("rack%dfloor", rack, rack_opt[rack].floor);
	}
}

COMPARE_STR_FN(bundle,name)
COMPARE_NUM_REV_FN(bundle,disk_qty)
COMPARE_NUM_REV_FN(bundle,rack_qty)
COMPARE_NUM_REV_FN(bundle,kase_qty)
COMPARE_NUM_REV_FN(bundle,mem_qty)
COMPARE_NUM_REV_FN(bundle,proc_qty)
COMPARE_NUM_REV_FN(bundle,sw_qty)
COMPARE_NUM_REV_FN(bundle,cab_qty)
COMPARE_NUM_REV_FN(bundle,nic_qty)
COMPARE_NUM_REV_FN(bundle,mom_qty)

static void save_bundles(FILE *dbOut)
{
	int bundle;

	/* Sort order is important.  The quantity of the first
	 * component evaluated should be sorted on last, so to
	 * guarantee bundles are added in order during evaluation.
	 * WARNING: A change in the sort order will break the
	 *          bundle evaluation code unless corresponding
	 *          major changes are made to it.
	 */
	qsort(bundle_opt, bundle_opts, sizeof(bundlet), compare_bundle_name);
	mergesort(bundle_opt, bundle_opts, sizeof(bundlet), compare_bundle_disk_qty);
	mergesort(bundle_opt, bundle_opts, sizeof(bundlet), compare_bundle_rack_qty);
	mergesort(bundle_opt, bundle_opts, sizeof(bundlet), compare_bundle_kase_qty);
	mergesort(bundle_opt, bundle_opts, sizeof(bundlet), compare_bundle_mem_qty);
	mergesort(bundle_opt, bundle_opts, sizeof(bundlet), compare_bundle_proc_qty);
	mergesort(bundle_opt, bundle_opts, sizeof(bundlet), compare_bundle_cab_qty);
	mergesort(bundle_opt, bundle_opts, sizeof(bundlet), compare_bundle_nic_qty);
	mergesort(bundle_opt, bundle_opts, sizeof(bundlet), compare_bundle_sw_qty);
	mergesort(bundle_opt, bundle_opts, sizeof(bundlet), compare_bundle_mom_qty);

	for (bundle = 0 ; bundle < bundle_opts ; bundle++) {
		db_s_entry("bundle%dname", bundle, bundle_opt[bundle].name);
		db_i_entry("bundle%dnic", bundle, bundle_opt[bundle].nic);
		db_i_entry("bundle%dnic_qty", bundle, bundle_opt[bundle].nic_qty);
		db_i_entry("bundle%dcab", bundle, bundle_opt[bundle].cab);
		db_i_entry("bundle%dcab_qty", bundle, bundle_opt[bundle].cab_qty);
		db_i_entry("bundle%dsw", bundle, bundle_opt[bundle].sw);
		db_i_entry("bundle%dsw_qty", bundle, bundle_opt[bundle].sw_qty);
		db_i_entry("bundle%dproc", bundle, bundle_opt[bundle].proc);
		db_i_entry("bundle%dproc_qty", bundle, bundle_opt[bundle].proc_qty);
		db_i_entry("bundle%dmom", bundle, bundle_opt[bundle].mom);
		db_i_entry("bundle%dmom_qty", bundle, bundle_opt[bundle].mom_qty);
		db_i_entry("bundle%dmem", bundle, bundle_opt[bundle].mem);
		db_i_entry("bundle%dmem_qty", bundle, bundle_opt[bundle].mem_qty);
		db_i_entry("bundle%ddisk", bundle, bundle_opt[bundle].disk);
		db_i_entry("bundle%ddisk_qty", bundle, bundle_opt[bundle].disk_qty);
		db_i_entry("bundle%dkase", bundle, bundle_opt[bundle].kase);
		db_i_entry("bundle%dkase_qty", bundle, bundle_opt[bundle].kase_qty);
		db_i_entry("bundle%drack", bundle, bundle_opt[bundle].rack);
		db_i_entry("bundle%drack_qty", bundle, bundle_opt[bundle].rack_qty);
		db_d_entry("bundle%dcost", bundle, bundle_opt[bundle].cost);
	}
}
