/*
 * edit.c
 *
 * Created by William R. Dieter, August 2002
 *
 * History
 * -------
 * $Log: edit.c,v $
 * Revision 1.10  2004/06/23 19:35:41  wrdieter
 * Updated text on bundling entry in edit.c.
 * Output number of parts required by design in overall report rather than
 * number of parts available from bundles and/or individual components.
 *
 * Revision 1.9  2004/06/23 14:18:54  wrdieter
 * Bug fixes to bundling code (still has problems)
 *
 * Revision 1.8  2004/06/22 20:59:42  wrdieter
 * Added bundle support.  Works for multiple single components (value
 * packs), but not components of different types.
 *
 * Revision 1.7  2004/06/04 16:32:50  wrdieter
 * Added bundle entry and database management, but no evaluation yet.
 *
 * Revision 1.6  2004/01/26 06:23:30  wrdieter
 * Added PXE field for motherboards.  No disk drive is required if PXE is present.
 *
 * Revision 1.5  2002/10/30 01:05:55  kaos
 * Added eval and view comments.
 * Fixed a bug in the network eval function that caused a hang when
 * evaluating single node designs.  When the network eval was
 * split into separate functions a return statement moved into the
 * function with e_nonet.  This error caused other types of networks
 * to be evaluated for 1 node, which caused a hang.
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

#include "cgic.h"
#include "rules.h"

#define EXTRA_OPTS  3
 
#define form_row_start() fprintf(cgiOut, "<TR>\n");
#define form_row_end()   fprintf(cgiOut, "</TR>\n");

#define form_s_entry(name,index,value) \
	fprintf(cgiOut, \
		"<TD><INPUT TYPE=\"TEXT\" NAME=\""name"\" "  \
		"VALUE=\"%s\"></TD>\n", index, value);

/* narrow string */
#define form_ns_entry(name,index,value) \
	fprintf(cgiOut, \
		"<TD><INPUT TYPE=\"TEXT\" SIZE=\"4\" NAME=\""name"\" "  \
		"VALUE=\"%s\"></TD>\n", index, value);

#define form_d_entry(name,index,value) \
	fprintf(cgiOut, \
		"<TD><INPUT TYPE=\"TEXT\" NAME=\""name"\" "  \
		"VALUE=\"%g\"></TD>\n", index, value);

#define form_i_entry(name,index,value) \
	fprintf(cgiOut, \
		"<TD><INPUT TYPE=\"TEXT\" NAME=\""name"\" "  \
		"VALUE=\"%d\"></TD>\n", index, value);

/* narrow integer */
#define form_ni_entry(name,index,value) \
	fprintf(cgiOut, \
		"<TD><INPUT TYPE=\"TEXT\" SIZE=\"4\" NAME=\""name"\" "  \
		"VALUE=\"%d\"></TD>\n", index, value);

#define form_yn_entry(name,index,value) \
	fprintf(cgiOut, \
		"<TD><INPUT TYPE=\"TEXT\" NAME=\""name"\" " \
		"VALUE=\"%s\"></TD>\n", index, value ? "Y" : "N");

#define form_sel_entry(NAME,bundle,type) {\
	int i; \
	fprintf(cgiOut, "<TD><SELECT NAME=\""NAME"\">\n", bundle); \
	for(i = 0 ; i < type##_opts ; i++) { \
		fprintf(cgiOut, "<OPTION %s VALUE=\"%d\">%s\n", \
			(bundle_opt[bundle]. type == i) ? "SELECTED" : "", \
			i, type##_opt[i].name); \
	} \
	fprintf(cgiOut, "</SELECT></TD>\n"); \
}


#define more_button(opt) \
	fprintf(cgiOut, \
		"<INPUT TYPE=\"SUBMIT\" NAME=\"more_opts\" " \
		"VALUE=\"Update Values/More Entries...\">\n");

static void print_nics(void);
static void print_cabs(void);
static void print_sw(void);
static void print_proc(void);
static void print_moms(void);
static void print_mems(void);
static void print_disks(void);
static void print_kases(void);
static void print_racks(void);
static void print_bundles(void);

void edit_params(void)
{
	if (g_uid < 0) {
		invalid_login();
		return;
	}
	fprintf(cgiOut, "<head>\n"
		"<title>CDR Costs &AMP; Internal Parameters</title>\n"
		"</head>\n"
		"<body>\n"
		"<p>\n"
		"<H1>Costs &AMP; Internal Parameters</H2>\n"
		"<P>\n"
		"\n"
		"Obviously, one of the most important cluster design constraints\n"
		"is cost...  but cost changes rapidly, and is not necessarily the\n"
		"same for everybody depending on where you live, what deals your\n"
		"institution has with vendors, etc.  To get truly accurate\n"
		"prices, the best approach is probably to first discuss the\n"
		"cluster with someone who knows your institution's purchasing\n"
		"procedure and any vendor relationships, then to get prices\n"
		"yourself (via the WWW, phone calls, etc.), and finally to write\n"
		"a document explaining precisely what you need so that people in\n"
		"purchasing will not make the common mistake of treating your\n"
		"request like purchase of generic PCs.  For the purpose of\n"
		"computing approximate price/performance for the cluster\n"
		"configurations that are otherwise viable, there are a few cost\n"
		"parameters used by our code.  The default prices used are very\n"
		"approximate; you may want to alter the values to more accurately\n"
		"reflect the prices you find.\n"
		"\n"
		"<P>\n"
		"<p>\n"
		"<FORM METHOD=\"POST\" ACTION=\""CDR_EXEC"\">\n"
		"<INPUT TYPE=\"HIDDEN\" NAME=\"version\" VALUE=\"%d\">\n"
		"<INPUT TYPE=\"HIDDEN\" NAME=\"paramset\" VALUE=\"%d\">\n"
		"<INPUT TYPE=\"HIDDEN\" NAME=\"email\" VALUE=\"%s\">\n"
		"<INPUT TYPE=\"HIDDEN\" NAME=\"password\" VALUE=\"%s\">\n"
		"<P>\n"
		"\n"
		"The full set of cost and <EM>internal</EM> parameters used \n"
		"by the CGI is given below.  You can change any of these \n"
		"parameters to better reflect your circumstances.\n"
		"\n"
		"<P>\n"
		"<INPUT TYPE=\"TEXT\" NAME=\"dbname\" VALUE=\"%s\" MAXLENGTH=400>\n"
		"<br>"
		"<INPUT TYPE=\"CHECKBOX\" NAME=\"oeval\" VALUE=1 %s>\n"
		"Allow others to evalute designs based on this database<br>\n"
		"<INPUT TYPE=\"CHECKBOX\" NAME=\"oread\" VALUE=1 %s>\n"
		"Allow others to view parameters used in this database<br>\n",
		VERSION, g_uid, email, password, dbname,
		oeval ? "CHECKED" : "",
		oread ? "CHECKED" : "");
	printf("<P>\nEnter comments you would like displayed when a user\n"
	       "evaluates a design with these parameters\n"
	       "<P>\n"
	       "<TEXTAREA ROWS=\"4\" COLS=\"60\" NAME=\"evalcomment\">"
	       "%s"
	       "</TEXTAREA>"
	       "<P>\n"
	       "<P>\nEnter comments you would like displayed when a user\n"
	       "views these parameters\n"
	       "<P>\n"
	       "<TEXTAREA ROWS=\"4\" COLS=\"60\" NAME=\"viewcomment\">"
	       "%s"
	       "</TEXTAREA>\n",
	       evalcomment, viewcomment);

	/* print form */
	print_nics();
	print_cabs();
	print_sw();
	print_proc();
	print_moms();
	print_mems();
	print_disks();
	print_kases();
	print_racks();
	print_bundles();
	fprintf(cgiOut,
		"<P><HR><P>\n"
		"<INPUT TYPE=\"SUBMIT\" VALUE=\"Save Changes\" NAME=\"set_params\">\n"
		"<INPUT TYPE=RESET VALUE=\"Reset Form\">\n"
		"</FORM>\n"
		"<p>\n"
		"<HR>\n"
		"<A HREF=\"http://aggregate.org/\">\n"
		"<IMG SRC=\""LOGO_URL"\" WIDTH=160 HEIGHT=32 "
		"BORDER=0 ALT=\"The Aggregate\">\n"
		"</A> Cluster Supercomputer Design Rules\n"
		"</BODY>\n"
		"</HTML>\n");
}

static void print_nics(void)
{
	int  nic;

	fprintf(cgiOut,
		"<H3>NICs (PCI bus network interface cards)</H3>\n"
		"<TABLE BORDER>\n"
		"<TR>\n"
		"<TH ALIGN=LEFT>Name</TH>\n"
		"<TH ALIGN=LEFT>Network Type</TH>\n"
		"<TH ALIGN=LEFT>Bandwidth (Mb/s)</TH>\n"
		"<TH ALIGN=LEFT>Latency (us)</TH>\n"
		"<TH ALIGN=LEFT>Cost</TH>\n"
		"</TR>\n");
	for (nic = 0 ; nic < nic_opts ; nic++) {
		form_row_start();
		form_s_entry("nic%dname", nic, nic_opt[nic].name);
		form_s_entry("nic%dntype", nic, nic_opt[nic].ntype);
		form_d_entry("nic%dmbs", nic, nic_opt[nic].mbs);
		form_d_entry("nic%dus", nic, nic_opt[nic].us);
		form_d_entry("nic%dcost", nic, nic_opt[nic].cost);
		form_row_end();
	}
	for ( ; nic < MAXOPTS && nic < (nic_opts + EXTRA_OPTS) ; nic++) {
		form_row_start();
		form_s_entry("nic%dname", nic, "");
		form_s_entry("nic%dntype", nic, "");
		form_s_entry("nic%dmbs", nic, "");
		form_s_entry("nic%dus", nic, "");
		form_s_entry("nic%dcost", nic, "");
		form_row_end();
	}
	fprintf(cgiOut,
		"</TR>\n"
		"\n"
		"\n"
		"</TABLE>\n"
		"<P>\n");
	
	more_button("nic");
}

static void print_cabs(void)
{
	int cab;

	fprintf(cgiOut,
		"<H3>Network Cables</H3>\n"
		"<TABLE BORDER>\n"
		"<TR>\n"
		"<TH ALIGN=LEFT>Name</TH>\n"
		"<TH ALIGN=LEFT>Network Type</TH>\n"
		"<TH ALIGN=LEFT>Length (feet)</TH>\n"
		"<TH ALIGN=LEFT>Cost</TH>\n"
		"</TR>\n");
	for (cab = 0 ; cab < cab_opts ; cab++) {
		form_row_start();
		form_s_entry("cab%dname", cab, cab_opt[cab].name);
		form_s_entry("cab%dntype", cab, cab_opt[cab].ntype);
		form_d_entry("cab%dfeet", cab, cab_opt[cab].feet);
		form_d_entry("cab%dcost", cab, cab_opt[cab].cost);
		form_row_end();
	}
	for ( ; cab < MAXOPTS && cab < (cab_opts + EXTRA_OPTS) ; cab++) {
		form_row_start();
		form_s_entry("cab%dname", cab, "");
		form_s_entry("cab%dntype", cab, "");
		form_s_entry("cab%dfeet", cab, "");
		form_s_entry("cab%dcost", cab, "");
		form_row_end();
	}

	fprintf(cgiOut,
		"</TABLE>\n"
		"<P>\n");
	more_button("cab");
}

static void print_sw(void)
{
	int sw;

	fprintf(cgiOut,
		"<H3>Switches (network switches)</H3>\n"
		"<TABLE BORDER>\n"
		"<TR>\n"
		"<TH ALIGN=LEFT>Name</TH>\n"
		"<TH ALIGN=LEFT>Network Type</TH>\n"
		"<TH ALIGN=LEFT>Total Bandwidth (Mb/s)</TH>\n"
		"<TH ALIGN=LEFT>Latency (us)</TH>\n"
		"<TH ALIGN=LEFT># Ports</TH>\n"
		"<TH ALIGN=LEFT>Cost</TH>\n"
		"<TH ALIGN=LEFT>Size (in U)</TH>\n"
		"<TH ALIGN=LEFT>Power (in Amps)</TH>\n"
		"</TR>\n");
	for (sw = 0 ; sw < sw_opts ; sw++) {
		form_row_start();
		form_s_entry("sw%dname", sw, sw_opt[sw].name);
		form_s_entry("sw%dntype", sw, sw_opt[sw].ntype);
		form_d_entry("sw%dmbs", sw, sw_opt[sw].mbs);
		form_d_entry("sw%dus", sw, sw_opt[sw].us);
		form_i_entry("sw%dports", sw, sw_opt[sw].ports);
		form_d_entry("sw%dcost", sw, sw_opt[sw].cost);
		form_i_entry("sw%du", sw, sw_opt[sw].u);
		form_d_entry("sw%damps", sw, sw_opt[sw].amps);
		form_row_end();
	}
	for ( ; sw < MAXOPTS && sw < (sw_opts + EXTRA_OPTS) ; sw++) {
		form_row_start();
		form_s_entry("sw%dname",  sw, "");
		form_s_entry("sw%dntype", sw, "");
		form_s_entry("sw%dmbs",   sw, "");
		form_s_entry("sw%dus",    sw, "");
		form_s_entry("sw%dports", sw, "");
		form_s_entry("sw%dcost",  sw, "");
		form_s_entry("sw%du",     sw, "");
		form_s_entry("sw%damps",  sw, "");
		form_row_end();
	}


	fprintf(cgiOut,
		"</TABLE>\n"
		"<P>\n");
	more_button("sw");
}

static void print_proc(void)
{
	int proc;

	fprintf(cgiOut,
		"<H3>Processors</H3>\n"
		"<TABLE BORDER>\n"
		"<TR>\n"
		"<TH ALIGN=LEFT>Name</TH>\n"
		"<TH ALIGN=LEFT>Processor Type</TH>\n"
		"<TH ALIGN=LEFT>GFLOPS</TH>\n"
		"<TH ALIGN=LEFT>Cost</TH>\n"
		"</TR>\n");

	for (proc = 0 ; proc < proc_opts ; proc++) {
		form_row_start();
		form_s_entry("proc%dname", proc, proc_opt[proc].name);
		form_s_entry("proc%dptype", proc, proc_opt[proc].ptype);
		form_d_entry("proc%dgflops", proc, proc_opt[proc].gflops);
		form_d_entry("proc%dcost", proc, proc_opt[proc].cost);
		form_row_end();
	}
	for ( ; proc < MAXOPTS && proc < (proc_opts + EXTRA_OPTS) ; proc++) {
		form_row_start();
		form_s_entry("proc%dname", proc,  "");
		form_s_entry("proc%dptype", proc, "");
		form_s_entry("proc%dgflops", proc, "");
		form_s_entry("proc%dcost", proc, "");
		form_row_end();
	}

	fprintf(cgiOut,
		"</TABLE>\n"
		"<P>\n");
	more_button("proc");
}

static void print_moms(void)
{
	int mom;

	fprintf(cgiOut,
		"<H3>Motherboards</H3>\n"
		"<TABLE BORDER>\n"
		"<TR>\n"
		"<TH ALIGN=LEFT>Name</TH>\n"
		"<TH ALIGN=LEFT>Processor Type</TH>\n"
		"<TH ALIGN=LEFT>Memory Type</TH>\n"
		"<TH ALIGN=LEFT>Case Type</TH>\n"
		"<TH ALIGN=LEFT># Processor Sockets</TH>\n"
		"<TH ALIGN=LEFT># Memory Slots</TH>\n"
		"<TH ALIGN=LEFT># PCI Slots</TH>\n"
		"<TH ALIGN=LEFT># IDE Drives</TH>\n"
		"<TH ALIGN=LEFT>Has PXE</TH>\n"
		"<TH ALIGN=LEFT>Cost</TH>\n"
		"</TR>\n");
	for (mom = 0 ; mom < mom_opts ; mom++) {
		form_row_start();
		form_s_entry("mom%dname", mom, mom_opt[mom].name);
		form_s_entry("mom%dptype", mom, mom_opt[mom].ptype);
		form_s_entry("mom%dmtype", mom, mom_opt[mom].mtype);
		form_s_entry("mom%dctype", mom, mom_opt[mom].ctype);
		form_i_entry("mom%dn", mom, mom_opt[mom].n);
		form_i_entry("mom%dmem", mom, mom_opt[mom].mem);
		form_i_entry("mom%dpci", mom, mom_opt[mom].pci);
		form_i_entry("mom%dide", mom, mom_opt[mom].ide);
		form_yn_entry("mom%dpxe", mom, mom_opt[mom].pxe);
		form_d_entry("mom%dcost", mom, mom_opt[mom].cost);
		form_row_end();
	}
	for ( ; mom < MAXOPTS && mom < (mom_opts + EXTRA_OPTS) ; mom++) {
		form_row_start();
		form_s_entry("mom%dname", mom, "");
		form_s_entry("mom%dptype", mom, "");
		form_s_entry("mom%dmtype", mom, "");
		form_s_entry("mom%dctype", mom, "");
		form_s_entry("mom%dn", mom, "");
		form_s_entry("mom%dmem", mom, "");
		form_s_entry("mom%dpci", mom, "");
		form_s_entry("mom%dide", mom, "");
		form_s_entry("mom%dpxe", mom, "");
		form_s_entry("mom%dcost", mom, "");
		form_row_end();
	}

	fprintf(cgiOut,
		"</TABLE>\n"
		"<P>\n");
	more_button("mom");
}

static void print_mems(void)
{
	int mem;

	fprintf(cgiOut,
		"<H3>Memory Parts</H3>\n"
		"<TABLE BORDER>\n"
		"<TR>\n"
		"<TH ALIGN=LEFT>Name</TH>\n"
		"<TH ALIGN=LEFT>Memory Type</TH>\n"
		"<TH ALIGN=LEFT>Size (in MB)</TH>\n"
		"<TH ALIGN=LEFT>Bandwidth (in GB/s)</TH>\n"
		"<TH ALIGN=LEFT>Cost</TH>\n"
		"</TR>\n");

	for (mem = 0 ; mem < mem_opts ; mem++) {
		form_row_start();
		form_s_entry("mem%dname", mem, mem_opt[mem].name);
		form_s_entry("mem%dmtype", mem, mem_opt[mem].mtype);
		form_d_entry("mem%dmb", mem, mem_opt[mem].mb);
		form_d_entry("mem%dgbs", mem, mem_opt[mem].gbs);
		form_d_entry("mem%dcost", mem, mem_opt[mem].cost);
		form_row_end();
	}
	for ( ; mem < MAXOPTS && mem < (mem_opts + EXTRA_OPTS) ; mem++) {
		form_row_start();
		form_s_entry("mem%dname", mem, "");
		form_s_entry("mem%dmtype", mem, "");
		form_s_entry("mem%dmb", mem, "");
		form_s_entry("mem%dgbs", mem, "");
		form_s_entry("mem%dcost", mem, "");
		form_row_end();
	}

	fprintf(cgiOut,
		"</TABLE>\n"
		"<P>\n");
	more_button("mem");
}

static void print_disks(void)
{
	int disk;

	fprintf(cgiOut,
		"<H3>Disk Drives (or other netboot devices)</H3>\n"
		"<TABLE BORDER>\n"
		"<TR>\n"
		"<TH ALIGN=LEFT>Name</TH>\n"
		"<TH ALIGN=LEFT>Size (in GB)</TH>\n"
		"<TH ALIGN=LEFT>Cost</TH>\n"
		"</TR>\n");
	for (disk = 0 ; disk < disk_opts ; disk++) {
		form_row_start();
		form_s_entry("disk%dname", disk, disk_opt[disk].name);
		form_d_entry("disk%dgb", disk, disk_opt[disk].gb);
		form_d_entry("disk%dcost", disk, disk_opt[disk].cost);
		form_row_end();
	}
	for ( ; disk < MAXOPTS && disk < (disk_opts + EXTRA_OPTS) ; disk++) {
		form_row_start();
		form_s_entry("disk%dname", disk, "");
		form_s_entry("disk%dgb", disk, "");
		form_s_entry("disk%dcost", disk, "");
		form_row_end();
	}

	fprintf(cgiOut,
		"</TABLE>\n"
		"<P>\n");
	more_button("disk");
}

static void print_kases(void)
{
	int kase;

	fprintf(cgiOut,
		"<H3>Cases (stand-alone or rack mount)</H3>\n"
		"<TABLE BORDER>\n"
		"<TR>\n"
		"<TH ALIGN=LEFT>Name</TH>\n"
		"<TH ALIGN=LEFT>Case Type</TH>\n"
		"<TH ALIGN=LEFT>Rack Type</TH>\n"
		"<TH ALIGN=LEFT># IDE Drives</TH>\n"
		"<TH ALIGN=LEFT>Cost</TH>\n"
		"<TH ALIGN=LEFT>Size (in U)</TH>\n"
		"<TH ALIGN=LEFT>Power (in Amps)</TH>\n"
		"</TR>\n");
	for (kase = 0 ; kase < kase_opts ; kase++) {
		form_row_start();
		form_s_entry("kase%dname", kase, kase_opt[kase].name);
		form_s_entry("kase%dctype", kase, kase_opt[kase].ctype);
		form_s_entry("kase%drtype", kase, kase_opt[kase].rtype);
		form_i_entry("kase%dide", kase, kase_opt[kase].ide);
		form_d_entry("kase%dcost", kase, kase_opt[kase].cost);
		form_i_entry("kase%du", kase, kase_opt[kase].u);
		form_d_entry("kase%damps", kase, kase_opt[kase].amps);
		form_row_end();
	}
	for ( ; kase < MAXOPTS && kase < (kase_opts + EXTRA_OPTS) ; kase++) {
		form_row_start();
		form_s_entry("kase%dname", kase, "");
		form_s_entry("kase%dctype", kase, "");
		form_s_entry("kase%drtype", kase, "");
		form_s_entry("kase%dide", kase, "");
		form_s_entry("kase%dcost", kase, "");
		form_s_entry("kase%du", kase, "");
		form_s_entry("kase%damps", kase, "");
		form_row_end();
	}

	fprintf(cgiOut,
		"</TABLE>\n"
		"<P>\n");
	more_button("kase");
}

static void print_racks(void)
{
	int rack;

	fprintf(cgiOut,
		"<H3>Racks (shelving units or rack cabinets)</H3>\n"
		"<TABLE BORDER>\n"
		"<TR>\n"
		"<TH ALIGN=LEFT>Name</TH>\n"
		"<TH ALIGN=LEFT>Rack Type</TH>\n"
		"<TH ALIGN=LEFT>Cost</TH>\n"
		"<TH ALIGN=LEFT>Size (in U)</TH>\n"
		"<TH ALIGN=LEFT>Floorspace (in 2x2')</TH>\n"
		"</TR>\n");
	for (rack = 0 ; rack < rack_opts ; rack++) {
		form_row_start();
		form_s_entry("rack%dname", rack, rack_opt[rack].name);
		form_s_entry("rack%drtype", rack, rack_opt[rack].rtype);
		form_d_entry("rack%dcost", rack, rack_opt[rack].cost);
		form_i_entry("rack%du", rack, rack_opt[rack].u);
		form_i_entry("rack%dfloor", rack, rack_opt[rack].floor);
		form_row_end();
	}
	for ( ; rack < MAXOPTS && rack < (rack_opts + EXTRA_OPTS) ; rack++) {
		form_row_start();
		form_s_entry("rack%dname", rack, "");
		form_s_entry("rack%drtype", rack, "");
		form_s_entry("rack%dcost", rack, "");
		form_s_entry("rack%du", rack, "");
		form_s_entry("rack%dfloor", rack, "");
		form_row_end();
	}

	fprintf(cgiOut,
		"</TABLE>\n"
		"<P>\n");
	more_button("rack");
}

static void print_bundles(void)
{
	int bundle;

	fprintf(cgiOut,
		"<H3>Bundles</H3>\n"
		"<P>Often components are available in bundles.  For \n"
		"example, Ethernet NICs come in 10-pack bundles,\n"
		"motherboards come with CPUs, and some blades\n"
		"come with intregrated hard drives.  Bundles can also\n"
		"be used to represent components that\n"
		"cannot physically be separated (e.g., motherboards with\n"
		"built-in NICs.)\n"
		"<P>To add a bundle to the database, enter a bundle name\n"
		"and select the number and type of components used in\n"
		"the bundle.  The price of the bundle will override the\n"
		"price of the individual components when the bundle is\n"
		"evaluated with the design.  Enter a higher price for the\n"
		"individual component if it can only be bought with the\n"
		"bundle.  If a component you have entered recently does not\n"
		"appear in one of the component lists, click on the \"Update"
		"Values/More Entries...\" button\n"
		"<P>\n"
		"<TABLE BORDER>\n"
		"<TR>\n"
		"<TH ALIGN=LEFT>Name</TH>\n"
		"<TH ALIGN=LEFT>NIC</TH>\n"
		"<TH ALIGN=LEFT>Qty.</TH>\n"
		"<TH ALIGN=LEFT>Network Cables</TH>\n"
		"<TH ALIGN=LEFT>Qty.</TH>\n"
		"<TH ALIGN=LEFT>Switches</TH>\n"
		"<TH ALIGN=LEFT>Qty.</TH>\n"
		"<TH ALIGN=LEFT>Processors</TH>\n"
		"<TH ALIGN=LEFT>Qty.</TH>\n"
		"<TH ALIGN=LEFT>Motherboards</TH>\n"
		"<TH ALIGN=LEFT>Qty.</TH>\n"
		"<TH ALIGN=LEFT>Memory</TH>\n"
		"<TH ALIGN=LEFT>Qty.</TH>\n"
		"<TH ALIGN=LEFT>Disks</TH>\n"
		"<TH ALIGN=LEFT>Qty.</TH>\n"
		"<TH ALIGN=LEFT>Cases</TH>\n"
		"<TH ALIGN=LEFT>Qty.</TH>\n"
		"<TH ALIGN=LEFT>Racks</TH>\n"
		"<TH ALIGN=LEFT>Qty.</TH>\n"
		"<TH ALIGN=LEFT>Bundle Cost</TH>\n"
		"</TR>\n");

	for (bundle = 0 ; bundle < bundle_opts ; bundle++) {
		form_row_start();
		form_s_entry("bundle%dname", bundle, bundle_opt[bundle].name);
		form_sel_entry("bundle%dnic", bundle, nic);
		form_ni_entry("bundle%dnic_qty", bundle, bundle_opt[bundle].nic_qty);
		form_sel_entry("bundle%dcab", bundle, cab);
		form_ni_entry("bundle%dcab_qty",  bundle, bundle_opt[bundle].cab_qty);
		form_sel_entry("bundle%dsw", bundle, sw);
		form_ni_entry("bundle%dsw_qty",   bundle, bundle_opt[bundle].sw_qty);
		form_sel_entry("bundle%dproc", bundle, proc);
		form_ni_entry("bundle%dproc_qty", bundle, bundle_opt[bundle].proc_qty);
		form_sel_entry("bundle%dmom", bundle, mom);
		form_ni_entry("bundle%dmom_qty",  bundle, bundle_opt[bundle].mom_qty);
		form_sel_entry("bundle%dmem", bundle, mem);
		form_ni_entry("bundle%dmem_qty",  bundle, bundle_opt[bundle].mem_qty);
		form_sel_entry("bundle%ddisk", bundle, disk);
		form_ni_entry("bundle%ddisk_qty", bundle, bundle_opt[bundle].disk_qty);
		form_sel_entry("bundle%dnkase", bundle, kase);
		form_ni_entry("bundle%dkase_qty", bundle, bundle_opt[bundle].kase_qty);
		form_sel_entry("bundle%drack", bundle, rack);
		form_ni_entry("bundle%drack_qty", bundle, bundle_opt[bundle].rack_qty);
		form_d_entry("bundle%dcost", bundle, bundle_opt[bundle].cost);
		form_row_end();
	}
	for ( ; bundle < MAXOPTS && bundle < (bundle_opts + EXTRA_OPTS) ; bundle++) {
		/* when bundle does not exist, choose a sane default */
		bundle_opt[bundle].nic = 0;
		bundle_opt[bundle].cab = 0;
		bundle_opt[bundle].sw = 0;
		bundle_opt[bundle].proc = 0;
		bundle_opt[bundle].mom = 0;
		bundle_opt[bundle].mem = 0;
		bundle_opt[bundle].disk = 0;
		bundle_opt[bundle].kase = 0;
		bundle_opt[bundle].rack = 0;
		form_row_start();
		form_s_entry("bundle%dname", bundle, "");
		form_sel_entry("bundle%dnic", bundle, nic);
		form_ns_entry("bundle%dnic_qty",  bundle, "");
		form_sel_entry("bundle%dcab", bundle, cab);
		form_ns_entry("bundle%dcab_qty",  bundle, "");
		form_sel_entry("bundle%dsw", bundle, sw);
		form_ns_entry("bundle%dsw_qty",   bundle, "");
		form_sel_entry("bundle%dproc", bundle, proc);
		form_ns_entry("bundle%dproc_qty", bundle, "");
		form_sel_entry("bundle%dmom", bundle, mom);
		form_ns_entry("bundle%dmom_qty",  bundle, "");
		form_sel_entry("bundle%dmem", bundle, mem);
		form_ns_entry("bundle%dmem_qty",  bundle, "");
		form_sel_entry("bundle%ddisk", bundle, disk);
		form_ns_entry("bundle%ddisk_qty", bundle, "");
		form_sel_entry("bundle%dkase", bundle, kase);
		form_ns_entry("bundle%dkase_qty", bundle, "");
		form_sel_entry("bundle%drack", bundle, rack);
		form_ns_entry("bundle%drack_qty", bundle, "");
		form_s_entry("bundle%dcost", bundle, "");
		form_row_end();
	}
	fprintf(cgiOut,
		"</TABLE>\n"
		"<P>\n");
	more_button("bundle");
}
