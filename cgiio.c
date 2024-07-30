/*	out.c

	Output routines....
*/

#include "config.h"
#include "rules.h"


void
header(register char *s)
{
	cgiHeaderContentType("text/html");

	fprintf(cgiOut,
		"<HTML>\n"
		"<HEAD>\n"
		"<TITLE>\n"
		"%s\n"
		"</TITLE>\n"
		"</HEAD>\n"
		"<BODY>\n"
		"<H1>\n"
		"%s\n"
		"</H1>\n"
		"<P>\n",
		s,
		s);
}

void
trailer(void)
{
	fprintf(cgiOut,
		"<P>\n"
		"<HR>\n"
		"<P>\n"
		"The C program that generated this page was written by\n"
		"<A HREF=\"http://aggregate.org/hankd/\">Hank Dietz</A>\n"
		"using the <A HREF=\"http://www.boutell.com/cgic/\">CGIC</A>\n"
		"library to implement the CGI interface.\n"
		"<P>\n"
		"<HR>\n"
		"<P>\n"
		"<A HREF=\"http://aggregate.org/\"\n"
		"><IMG SRC=\""LOGO_URL"\" WIDTH=160 HEIGHT=32 ALT=\"The Aggregate.\"\n"
		"></A> Cluster Supercomputer Design Rules.\n"
		"</BODY>\n"
		"</HTML>\n");
	fflush(cgiOut);
}

cgiMain()
{
	register int minn, maxn;
	register designt *p, *q;

	cgiFormInteger("ossize", &ossize, 16);
	cgiFormInteger("codesize", &codesize, 16);
	cgiFormInteger("datasize", &datasize, 512);
	cgiFormInteger("disksize", &disksize, 1024);
	cgiFormInteger("useraid", &useraid, 0);
	cgiFormInteger("usebackup", &usebackup, 0);
	cgiFormInteger("maxlatency", &maxlatency, 10000);
	cgiFormInteger("afnlatency", &afnlatency, 10000);
	cgiFormInteger("minbandwidth", &minbandwidth, 100);
	cgiFormInteger("coordinality", &coordinality, 1);
	cgiFormInteger("nodecon", &nodecon, -1);
	cgiFormInteger("racks", &racks, 4);
	cgiFormInteger("amps", &amps, 40);
	cgiFormInteger("video", &video, 0);


	cgiFormInteger("budget", &budget, 10000);
	cgiFormInteger("proccost2", &proccost2, 210);
	cgiFormInteger("proccost1", &proccost1, 160);
	cgiFormInteger("proccost0", &proccost0, 120);
	cgiFormStringNoNewlines("procname2", procname2, MAXLEN);
	cgiFormStringNoNewlines("procname1", procname1, MAXLEN);
	cgiFormStringNoNewlines("procname0", procname0, MAXLEN);
	cgiFormInteger("momcost", &momcost, 80);
	cgiFormInteger("mommems", &mommems, 3);
	cgiFormInteger("dmommems", &dmommems, 2);
	cgiFormInteger("mem128cost", &mem128cost, 15);
	cgiFormInteger("mem256cost", &mem256cost, 25);
	cgiFormInteger("mem512cost", &mem512cost, 60);


	cgiFormInteger("maxpcdisk", &maxpcdisk, 8);
	cgiFormInteger("max1udisk", &max1udisk, 4);
	cgiFormInteger("disksize2", &disksize2, 100);
	cgiFormInteger("disksize1", &disksize1, 60);
	cgiFormInteger("disksize0", &disksize0, 20);
	cgiFormInteger("diskcost2", &diskcost2, 225);
	cgiFormInteger("diskcost1", &diskcost1, 105);
	cgiFormInteger("diskcost0", &diskcost0, 70);
	cgiFormInteger("floppycost", &floppycost, 15);

	cgiFormInteger("dproccost2", &dproccost2, 240);
	cgiFormInteger("dproccost1", &dproccost1, 160);
	cgiFormInteger("dproccost0", &dproccost0, 150);
	cgiFormStringNoNewlines("dprocname2", dprocname2, MAXLEN);
	cgiFormStringNoNewlines("dprocname1", dprocname1, MAXLEN);
	cgiFormStringNoNewlines("dprocname0", dprocname0, MAXLEN);
	cgiFormInteger("dmomcost", &dmomcost, 200);

	cgiFormDouble("gflops2", &gflops2, (1.53 * 2.0));
	cgiFormDouble("gflops1", &gflops1, (1.47 * 2.0));
	cgiFormDouble("gflops0", &gflops0, (1.33 * 2.0));
	cgiFormDouble("dgflops2", &dgflops2, (1.53 * 2.0));
	cgiFormDouble("dgflops1", &dgflops1, (1.40 * 2.0));
	cgiFormDouble("dgflops0", &dgflops0, (1.33 * 2.0));

	cgiFormDouble("nodeamps", &nodeamps, 1.2);
	cgiFormDouble("dnodeamps", &dnodeamps, 2.0);
	cgiFormInteger("nicfecost", &nicfecost, 15);
	cgiFormInteger("swfe8cost", &swfe8cost, 40);
	cgiFormInteger("swfe16cost", &swfe16cost, 80);
	cgiFormInteger("swfe32cost", &swfe32cost, 300);
	cgiFormInteger("swfe48cost", &swfe48cost, 825);
	cgiFormInteger("swfe64cost", &swfe64cost, 1200);
	cgiFormInteger("nicgcost", &nicgcost, 50);
	cgiFormInteger("swg8cost", &swg8cost, 1000);
	cgiFormInteger("sancost", &sancost, 1500);
	cgiFormInteger("afucost", &afucost, 100);
	cgiFormDouble("afuamps", &afuamps, 0.25);
	cgiFormDouble("swamps", &swamps, 1.2);
	cgiFormInteger("casecost", &casecost, 60);
	cgiFormInteger("npershelf", &npershelf, 8);
	cgiFormInteger("shelfcost", &shelfcost, 80);
	cgiFormInteger("rack1ucost", &rack1ucost, 150);
	cgiFormInteger("rackcost", &rackcost, 1300);
	cgiFormInteger("nperrack", &nperrack, 36);
	cgiFormInteger("videocost", &videocost, 200);
	cgiFormDouble("videoamps", &videoamps, 2.0);

	cgiFormInteger("howmany", &howmany, 10);
	cgiFormInteger("sortby", &sortby, 0);
	cgiFormInteger("show", &show, 0);


	for (nodes=1; nodes<=(1024 + ((1024 + 16) / 16)); ++nodes) {
		if (filter(nodes)) continue;

		evals(procname2, 1, proccost2, momcost, minbandwidth, nodeamps, gflops2);
		evals(procname1, 1, proccost1, momcost, minbandwidth, nodeamps, gflops1);
		evals(procname0, 1, proccost0, momcost, minbandwidth, nodeamps, gflops0);
		evals(dprocname2, 2, dproccost2, dmomcost, minbandwidth, dnodeamps, dgflops2);
		evals(dprocname1, 2, dproccost1, dmomcost, minbandwidth, dnodeamps, dgflops1);
		evals(dprocname0, 2, dproccost0, dmomcost, minbandwidth, dnodeamps, dgflops0);
	}

	if (dp == &(good[0])) {
		/* No solutions! */
		header("Cluster Design Rules CGI Output");

		fprintf(cgiOut,
"<H2>Design Search Failed</H2>\n"
"<P>\n"
"There were no designs meeting your constraints.\n"
"Please use your browser back command,\n"
"change some parameters, and try again.\n");

		trailer();
		return(0);
	}

	switch (sortby) {
	case 0:
		/* Sort into decreasing GFLOPS order */
		for (p=&(good[0]); p<dp; ++p) {
			for (q=p+1; q<dp; ++q) {
				if ((p->gflops < q->gflops) ||
				    ((p->gflops == q->gflops) &&
				     (p->cost > q->cost))) {
					designt t = *p;
					*p = *q;
					*q = t;
				}
			}
		}
		break;
	case 1:
		/* Sort into increasing cost order */
		for (p=&(good[0]); p<dp; ++p) {
			for (q=p+1; q<dp; ++q) {
				if ((p->cost > q->cost) ||
				    ((p->cost == q->cost) &&
				     (p->gflops < q->gflops))) {
					designt t = *p;
					*p = *q;
					*q = t;
				}
			}
		}
		break;
	case 2:
		/* Sort into increasing cost/performance order */
		for (p=&(good[0]); p<dp; ++p) {
			for (q=p+1; q<dp; ++q) {
				if ((((double) p->cost / p->gflops) > ((double) q->cost / p->gflops)) ||
				    ((((double) p->cost / p->gflops) == ((double) q->cost / p->gflops)) &&
				     (p->gflops < q->gflops))) {
					designt t = *p;
					*p = *q;
					*q = t;
				}
			}
		}
		break;
	case 3:
		/* Sort into decreasing bisection order */
		for (p=&(good[0]); p<dp; ++p) {
			for (q=p+1; q<dp; ++q) {
				if ((p->bisect < q->bisect) ||
				    ((p->bisect == q->bisect) &&
				     (p->cost > q->cost))) {
					designt t = *p;
					*p = *q;
					*q = t;
				}
			}
		}
		break;
	case 4:
		/* Sort into decreasing memory bandwidth order */
		for (p=&(good[0]); p<dp; ++p) {
			for (q=p+1; q<dp; ++q) {
				if ((p->nodes < q->nodes) ||
				    ((p->nodes == q->nodes) &&
				     ((p->cost > q->cost) ||
				      ((p->cost == q->cost) &&
				       (p->gflops < q->gflops))))) {
					designt t = *p;
					*p = *q;
					*q = t;
				}
			}
		}
		break;
	}

	switch (show) {
	case 0:
		/* Show detailed design options */
		header("Cluster Design Rules CGI Output");
		options();
		break;
	case 1:
		/* Show plot of cost and gflops for
		   uniprocessor vs. multiprocessor
		*/
		cgiHeaderContentType("image/png");
		plot();
		break;
	case 2:
		/* Show plot of cost and gflops for
		   different messaging network types
		*/
		cgiHeaderContentType("image/png");
		plot2();
		break;
	}
}

plot(void)
{
	char tmpfile[1024], buf[1024 * 64];
	char pmtfile[1024];
	FILE *tmp, *pmt;

	sprintf(tmpfile, "/tmp/amd_XXXXXX");
	if (mkstemp(tmpfile) == 0) {
		return(1);
	}
	if ((tmp = fopen(tmpfile, "w+")) == 0) {
		return(1);
	}
	chmod(tmpfile, (S_IRUSR | S_IWUSR |
			S_IRGRP | S_IWGRP |
			S_IROTH | S_IWOTH));
	sprintf(pmtfile, "/tmp/dma_XXXXXX");
	if (mkstemp(pmtfile) == 0) {
		return(1);
	}
	chmod(pmtfile, (S_IRUSR | S_IWUSR |
			S_IRGRP | S_IWGRP |
			S_IROTH | S_IWOTH));

	fprintf(tmp,
"set terminal png small color\n"
"set xlabel \"Total Cost of Cluster\"\n"
"set ylabel \"Peak GFLOPS (double precision)\"\n"
"set key left Left reverse\n"
"plot \"-\" title \"Uniprocessor nodes\" with points ps 2 ,"
" \"-\" title \"Dual-processor nodes\" with points ps 2\n");
	pp = &(good[0]);
	while ((pp < dp) && (pp < &(good[howmany]))) {
		if (pp->procpernode == 1) {
			fprintf(tmp,
				"%d %f\n",
				pp->cost,
				pp->gflops);
		}
		++pp;
	}
	fprintf(tmp, "e\n");
	pp = &(good[0]);
	while ((pp < dp) && (pp < &(good[howmany]))) {
		if (pp->procpernode > 1) {
			fprintf(tmp,
				"%d %f\n",
				pp->cost,
				pp->gflops);
		}
		++pp;
	}
	fprintf(tmp, "e\n");
	fclose(tmp);

	sprintf(buf, GNUPLOT" <%s >%s", tmpfile, pmtfile);
	system(buf);
	unlink(tmpfile);
	if ((pmt = fopen(pmtfile, "r")) == 0) {
		return(1);
	}
	fwrite(buf, 1, fread(buf, 1, (64 * 1024), pmt), cgiOut);
	fclose(pmt);
	unlink(pmtfile);
	return(0);
}

plot2(void)
{
	char tmpfile[1024], buf[1024 * 64];
	char pmtfile[1024];
	FILE *tmp, *pmt;

	sprintf(tmpfile, "/tmp/amd_XXXXXX");
	if (mkstemp(tmpfile) == 0) {
		return(1);
	}
	if ((tmp = fopen(tmpfile, "w+")) == 0) {
		return(1);
	}
	chmod(tmpfile, (S_IRUSR | S_IWUSR |
			S_IRGRP | S_IWGRP |
			S_IROTH | S_IWOTH));
	sprintf(pmtfile, "/tmp/dma_XXXXXX");
	if (mkstemp(pmtfile) == 0) {
		return(1);
	}
	chmod(pmtfile, (S_IRUSR | S_IWUSR |
			S_IRGRP | S_IWGRP |
			S_IROTH | S_IWOTH));

	fprintf(tmp,
"set terminal png small color\n"
"set xlabel \"Total Cost of Cluster\"\n"
"set ylabel \"Peak GFLOPS (double precision)\"\n"
"set key left Left reverse\n"
"plot \"-\" title \"Gigabit SAN (Myrinet, Dolphin, or cLAN)\" with points ps 2 ,"
" \"-\" title \"Gigabit Ethernet\" with points ps 2,"
" \"-\" title \"100Mb/s Fast Ethernet\" with points ps 2,"
" \"-\" title \"Channel Bonded Fast Ethernet\" with points ps 2,"
" \"-\" title \"Flat Neighborhood Fast Ethernet\" with points ps 2\n");
	pp = &(good[0]);
	while ((pp < dp) && (pp < &(good[howmany]))) {
		if (pp->nettype & NETGBSAN) {
			fprintf(tmp,
				"%d %f\n",
				pp->cost,
				pp->gflops);
		}
		++pp;
	}
	fprintf(tmp, "e\n");
	pp = &(good[0]);
	while ((pp < dp) && (pp < &(good[howmany]))) {
		if (pp->nettype & NETGBE) {
			fprintf(tmp,
				"%d %f\n",
				pp->cost,
				pp->gflops);
		}
		++pp;
	}
	fprintf(tmp, "e\n");
	pp = &(good[0]);
	while ((pp < dp) && (pp < &(good[howmany]))) {
		if ((pp->nettype & NETFE) &&
		    (pp->nicspern == 1)) {
			fprintf(tmp,
				"%d %f\n",
				pp->cost,
				pp->gflops);
		}
		++pp;
	}
	fprintf(tmp, "e\n");
	pp = &(good[0]);
	while ((pp < dp) && (pp < &(good[howmany]))) {
		if ((pp->nettype & NETFE) &&
		    (pp->nicspern > 1)) {
			fprintf(tmp,
				"%d %f\n",
				pp->cost,
				pp->gflops);
		}
		++pp;
	}
	fprintf(tmp, "e\n");
	pp = &(good[0]);
	while ((pp < dp) && (pp < &(good[howmany]))) {
		if (pp->nettype & NETFNN) {
			fprintf(tmp,
				"%d %f\n",
				pp->cost,
				pp->gflops);
		}
		++pp;
	}
	fprintf(tmp, "e\n");
	fclose(tmp);

	sprintf(buf, GNUPLOT" <%s >%s", tmpfile, pmtfile);
	system(buf);
	unlink(tmpfile);
	if ((pmt = fopen(pmtfile, "r")) == 0) {
		return(1);
	}
	fwrite(buf, 1, fread(buf, 1, (64 * 1024), pmt), cgiOut);
	fclose(pmt);
	unlink(pmtfile);
	return(0);
}


#define	BARSIZE	32

bar(char *tag,
int min,
int val,
int max)
{
	register int i, pos;

	if (min == max) return;

	fprintf(cgiOut, "%-20s  ", tag);
	fprintf(cgiOut, "%-8d  ", val);
	fprintf(cgiOut, "%8d [", min);
	pos = (((val - min) * ((BARSIZE - 3.0) / (max - min))) + 0.5);
	for (i=0; i<(BARSIZE-2); ++i) {
		fprintf(cgiOut, ((i == pos) ? "*" : " "));
	}
	fprintf(cgiOut, "] %-8d\n", max);
}

dbar(char *tag,
double min,
double val,
double max)
{
	register int i, pos;

	if (min == max) return;

	fprintf(cgiOut, "%-20s  ", tag);
	fprintf(cgiOut, "%-8.2f  ", val);
	fprintf(cgiOut, "%8.2f [", min);
	pos = (((val - min) * ((BARSIZE - 3.0) / (max - min))) + 0.5);
	for (i=0; i<(BARSIZE-2); ++i) {
		fprintf(cgiOut, ((i == pos) ? "*" : " "));
	}
	fprintf(cgiOut, "] %-8.2f\n", max);
}


relgraph(register designt *q)
{
	int minnodes = 1000000, maxnodes = 0;
	int minprocs = 1000000, maxprocs = 0;
	double mingflops = 1000000, maxgflops = 0;
	int minmem = 1000000, maxmem = 0;
	int mindisk = 1000000, maxdisk = 0;
	int minbisect = 1000000, maxbisect = 0;
	double minbpp = 1000000, maxbpp = 0;
	int mincost = 1000000, maxcost = 0;
	double mincostperf = 1000000, maxcostperf = 0;
	double minpower = 1000000, maxpower = 0;
	int minfloor = 1000000, maxfloor = 0;
	register designt *p = &(good[0]);

	fprintf(cgiOut,
"<PRE>\n"
		);

	/* Determine min, max for each thing */
	for (; p<dp; ++p) {
		register int t;
		register double dt;
		if ((t = p->nodes) < minnodes) minnodes = t;
		if (t > maxnodes) maxnodes = t;
		if ((t = (p->nodes * p->procpernode)) < minprocs) minprocs = t;
		if (t > maxprocs) maxprocs = t;
		if ((dt = p->gflops) < mingflops) mingflops = dt;
		if (dt > maxgflops) maxgflops = dt;
		if ((t = p->nodes * p->nodemem) < minmem) minmem = t;
		if (t > maxmem) maxmem = t;
		if ((t = p->nodes * p->disk) < mindisk) mindisk = t;
		if (t > maxdisk) maxdisk = t;
		if ((t = p->bisect) < minbisect) minbisect = t;
		if (t > maxbisect) maxbisect = t;
		if ((dt = ((double) p->bisect / (p->nodes * p->procpernode))) < minbpp) minbpp = dt;
		if (dt > maxbpp) maxbpp = dt;
		if ((t = p->cost) < mincost) mincost = t;
		if (t > maxcost) maxcost = t;
		if ((dt = (((double) p->cost) / p->gflops)) < mincostperf) mincostperf = dt;
		if (dt > maxcostperf) maxcostperf = dt;
		if ((dt = p->amps) < minpower) minpower = dt;
		if (dt > maxpower) maxpower = dt;
		if ((t = (p->userack ? p->racks : p->shelves)) < minfloor) minfloor = t;
		if (t > maxfloor) maxfloor = t;
	}

	/* Now plot this one... */
	bar("Nodes",
	    minnodes,
	    q->nodes,
	    maxnodes);
	bar("Processors",
	    minprocs,
	    (q->nodes * q->procpernode),
	    maxprocs);
	dbar("Peak GFLOPS",
	     mingflops,
	     q->gflops,
	     maxgflops);
	bar("Total Memory (MB)",
	    minmem,
	    (q->nodes * q->nodemem),
	    maxmem);
	bar("Total Disk (GB)",
	    mindisk,
	    (q->disk * q->nodes),
	    maxdisk);
	bar("Bisection (Mb/s)",
	    minbisect,
	    q->bisect,
	    maxbisect);
	dbar("Bisection/Processor",
	     minbpp,
	     ((double) q->bisect / (q->nodes * q->procpernode)),
	     maxbpp);
	bar("Cost",
	    mincost,
	    q->cost,
	    maxcost);
	dbar("Cost/Peak GFLOPS",
	     mincostperf,
	     (((double) q->cost) / q->gflops),
	     maxcostperf);
	dbar("Total Power (Amps)",
	     minpower,
	     q->amps,
	     maxpower);
	bar("Floorspace (2x2)",
	    minfloor,
	    (q->userack ? q->racks : q->shelves),
	    maxfloor);

	fprintf(cgiOut, "\n</PRE>\n");
}

options(void)
{
	register int i;

	fprintf(cgiOut,
"<P>\n"
"The following designs were created to meet your specifications\n"
"using a WWW form and an automated CGI script.\n"
"A total of %d possible designs were evaluated in detail\n"
"and %d of them were found to satisfy the requirements given.\n"
"Although the author of the CGI believes it to be reasonably\n"
"accurate, the performance and cost data are inherently approximate,\n"
"and should be used only as a starting point for design of a cluster.\n"
"<P>\n"
"<STRONG>The author does NOT sell clusters nor components</STRONG>.\n"
"The CGI that created the following designs is the result of academic\n"
"research conducted by\n"
"<A HREF=\"http://aggregate.org/hankd/\">Professor Hank Dietz</A> in\n"
"the <A HREF=\"http://www.engr.uky.edu/ece/\">Electrical &AMP; Computer\n"
"Engineering Department</A> of the\n"
"<A HREF=\"http://www.uky.edu/\">University of Kentucky</A>\n"
"in Lexington, KY USA.\n"
"<P>\n",
		tried,
		(dp - &(good[0])));

	pp = &(good[0]);
	for (i=1; ((pp < dp) && (i<=howmany)); ++i) {

		fprintf(cgiOut,
"<P>\n"
"<HR>\n"
"<H2>\n"
"<P ALIGN=CENTER>\n"
"<EM>Design Option %d</EM>\n"
"<P ALIGN=CENTER>\n"
"%d %s %s node%s%s</H2>\n"
"<P>\n"
"The general characteristics of this configuration are:\n"
"<P ALIGN=CENTER>\n"
"<CENTER>\n",
			i,
			pp->nodes,
			((pp->procpernode < 2) ? " " :
			 (pp->procpernode < 3) ? "dual " :
			 "multiprocessor "),
			pp->procname,
			((pp->nodes > 1) ? "s" : ""),
			((pp->nettype & NETGBSAN) ? " with Gigabit SAN" :
			 ((pp->nettype & NETGBE) ? " with Gigabit Ethernet" :
			  ((pp->nettype & NETFE) ? ((pp->nicspern < 2) ?
						    " with Fast Ethernet" :
						    " with Channel Bonded Fast Ethernet") :
			   ((pp->nettype & NETFNN) ? " with Flat Neighborhood Fast Ethernet" :
			    "")))));

		relgraph(pp);

		fprintf(cgiOut,
"</CENTER>\n"
"<P>\n"
"The peak performance of the cluster is %g GFLOPS\n"
"using IA32 legacy 64/80-bit double-precision floating point.\n"
"For 32-bit single-precision floating point using\n"
"<EM>3DNow!</EM> <A HREF=\"http://aggregate.org/SWAR/\">SWAR</A>,\n"
"peak performance is approximately %g GFLOPS.\n"
"Because most applications mix FLOPs with other work,\n"
"substantially lower GFLOPS are usually achieved.\n",
			pp->gflops,
			(2.0 * pp->gflops));

		if (((pp->bisect / (pp->nodes * pp->procpernode)) >= 100) &&
		    ((pp->gflops / 3.0) > (1.4 * 1.4 * 67.78))) {
			fprintf(cgiOut,
"(That said, this configuration is likely to be fast enough\n"
"running the parallel Linpack benchmark code so that it should\n"
"be able to make the Spring 2002\n"
"<A HREF=\"http://www.top500.org/\">list of the 500 fastest\n"
"supercomputers in the world</A>.)\n");
		}

		fprintf(cgiOut,
"Total main memory is %d MB.\n",
			(pp->nodes * pp->nodemem));

		if (pp->disk) {
			fprintf(cgiOut,
"Total disk space on the cluster is %d GB.\n",
				(pp->disk * pp->nodes));
		}

		fprintf(cgiOut,
"Bisection bandwidth of the network approaches %dMb/s.\n",
			pp->bisect);

		fprintf(cgiOut,
"Although the costs used in creating this configuration\n"
"are very approximate, the system cost should be close\n"
"to $%d, yielding price/peak performance of\n"
"$%.2f/GFLOPS (double precision) and $%.2f/GFLOPS (single precision).\n",
			pp->cost,
			(pp->cost / pp->gflops),
			(pp->cost / (2.0 * pp->gflops)));

		fprintf(cgiOut,
"<P>\n"
"The components used, and their costs, are summarized here:\n");

		fprintf(cgiOut,
"<P ALIGN=CENTER>\n"
"<TABLE ALIGN=CENTER BORDER>\n"
"<TR>\n"
"<TH ALIGN=LEFT>Quantity</TH>\n"
"<TH ALIGN=LEFT>Component Description</TH>\n"
"<TH ALIGN=LEFT>Part Price</TH>\n"
"<TH ALIGN=LEFT>Extended Price</TH>\n"
"</TR>\n"
			);

		fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>%s processor &AMP; heat sink</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
			(pp->nodes * pp->procpernode),
			pp->procname,
			pp->proccost,
			(pp->nodes * ((pp->procpernode == 1) ?
				      pp->proccost :
				      (2 * pp->proccost))));

		fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>%s</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
			pp->nodes,
			((pp->procpernode == 1) ?
			 "Motherboard (for uniprocessor node)" :
			 "Motherboard (for MPS node)"),
			((pp->procpernode == 1) ?
			 momcost :
			 dmomcost),
			(pp->nodes * ((pp->procpernode == 1) ?
				     momcost :
				     dmomcost)));

		fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>%dMB memory</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
			(pp->nodes * (pp->nodemem / pp->mempart)),
			pp->mempart,
			((pp->mempart == 128) ? mem128cost :
			 ((pp->mempart == 256) ? mem256cost :
			  mem512cost)),
			pp->memcost);

		if (pp->disk == 0) {
			/* No hard disk, use floppy for netboot */
			fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>Floppy disk drive (for netboot)</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
				pp->nodes,
				floppycost,
				(pp->nodes * floppycost));
		} else {
			/* Hard disks */
			fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>%dGB disk drive (netboot%s)</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
				pp->nodes,
				pp->diskpart,
				((useraid & usebackup) ? ", RAID, &AMP; backup" :
				 ((useraid) ? " &AMP; RAID" :
				  ((usebackup) ? " &AMP; backup" :
				   " &AMP; local data"))),
				((pp->diskpart == disksize2) ? diskcost2 :
				 ((pp->diskpart == disksize1) ? diskcost1 :
				  diskcost0)),
				pp->nodes * ((pp->diskpart == disksize2) ? diskcost2 :
					     ((pp->diskpart == disksize1) ? diskcost1 :
					      diskcost0)));
		}


		if (pp->userack) {
			fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>1U rack mount case + power supply</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
				pp->nodes,
				rack1ucost,
				(pp->nodes * rack1ucost));
			fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>Rack mount cabinet</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
				pp->racks,
				rackcost,
				(pp->racks * rackcost));
		} else {
			fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>Mid-tower PC case + power supply</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
				pp->nodes,
				casecost,
				(pp->nodes * casecost));
			if (pp->shelves > 1) {
				fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>2x4 footprint shelving unit with wheels</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
				(pp->shelves / 2),
				shelfcost,
				((pp->shelves / 2) * shelfcost));
			}
			if (pp->shelves & 1) {
				fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>1</TD>\n"
"<TD ALIGN=LEFT>2x2 footprint shelving unit with wheels</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
				shelfcost,
				shelfcost);
			}
		}

		switch (pp->nettype & ~NETAFN) {
		case NETGBSAN:
			fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>Gigabit SAN hardware (approx. per node);\n"
"<BR>\n"
"E.g., <A HREF=\"http://www.myri.com/\">Myrinet</A>,\n"
"<A HREF=\"http://www.dolphinics.com/\">Dolphin</A>,\n"
"or <A HREF=\"http://www.emulex.com/\">cLAN</A></TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
				pp->nodes,
				sancost,
				(pp->nodes * sancost));
			break;
		case NETGBE:
			fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>Gigabit Ethernet NIC &AMP; Cat5 cable</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
				pp->nodes,
				nicgcost,
				(pp->nodes * nicgcost));
			fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>Gigabit Ethernet 8-port switch</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
				1,
				swg8cost,
				swg8cost);
			break;
		case NETFE:
			fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>100Mb/s Fast Ethernet NIC &AMP; Cat5 cable</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
				(pp->nodes * pp->nicspern),
				nicfecost,
				(pp->nodes * pp->nicspern * nicfecost));
			if (pp->nodes > 2) {
				fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>100Mb/s Fast Ethernet %d-port switch</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
					pp->nicspern,
					((pp->nodes <= 8) ? 8 :
					 ((pp->nodes <= 16) ? 16 :
					  ((pp->nodes <= 32) ? 32 :
					   ((pp->nodes <= 48) ? 48 :
					    64)))),
					((pp->nodes <= 8) ? swfe8cost :
					 ((pp->nodes <= 16) ? swfe16cost :
					  ((pp->nodes <= 32) ? swfe32cost :
					   ((pp->nodes <= 48) ? swfe48cost :
					    swfe64cost)))),
					((pp->nodes <= 8) ? swfe8cost :
					 ((pp->nodes <= 16) ? swfe16cost :
					  ((pp->nodes <= 32) ? swfe32cost :
					   ((pp->nodes <= 48) ? swfe48cost :
					    swfe64cost)))) * pp->nicspern);
			}
			break;
		case NETFNN:
			fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>100Mb/s Fast Ethernet NIC &AMP; Cat5 cable</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
				(pp->nodes * pp->nicspern),
				nicfecost,
				(pp->nodes * nicfecost));
			fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>100Mb/s Fast Ethernet %d-port switch</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
				pp->netsw,
				pp->swwidth,
				((pp->swwidth <= 8) ? swfe8cost :
				 ((pp->swwidth <= 16) ? swfe16cost :
				  ((pp->swwidth <= 32) ? swfe32cost :
				   ((pp->swwidth <= 48) ? swfe48cost :
				    swfe64cost)))),
				((pp->swwidth <= 8) ? swfe8cost :
				 ((pp->swwidth <= 16) ? swfe16cost :
				  ((pp->swwidth <= 32) ? swfe32cost :
				   ((pp->swwidth <= 48) ? swfe48cost :
				    swfe64cost)))) * pp->netsw);
			break;
		}
		if (pp->nettype & NETAFN) {
			fprintf(cgiOut,
"<TR>\n"
"<TD ALIGN=RIGHT>%d</TD>\n"
"<TD ALIGN=LEFT>4-way AFU &AMP; parallel port cables;\n"
"<BR>\n"
"custom harwdare like\n"
"<A HREF=\"http://aggregate.org/AFN/960801/Index.html/\">this public-domain design</A></TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n",
				pp->afus,
				afucost,
				(pp->afus * afucost));
		}

		fprintf(cgiOut,
"<TR>\n"
"<TD></TD>\n"
"<TD ALIGN=RIGHT>Total</TD>\n"
"<TD></TD>\n"
"<TD ALIGN=RIGHT>$%d</TD>\n"
"</TR>\n"
"</TABLE>\n",
			pp->cost);

		++pp;
	}

	trailer();
	return(0);
}

eval(char *procname,
int procpernode,
int proccost,
int momcost,
int memsiz,
int minbandwidth,
double nodeamps,
double gflops,
int nettype)
{
	++tried;

	dp->procgflops = gflops;
	dp->proccost = proccost;
	dp->procname = procname;

	minbandwidth *= procpernode;
	dp->procpernode = procpernode;
	dp->gflops = (nodes * procpernode * gflops);

	dp->nodes = nodes;
	dp->cost = (nodes * ((procpernode * proccost) + momcost));

	/* Can we hold enough memory in this many nodes? */
	dp->nodemem = ((ossize + codesize) +
		       ((datasize + nodes - 1) / nodes));
	{
		/* Can do using 128MB, 256MB, or 512MB parts...
		   use whatever is cheapest, prefer bigger in
		   case of a tie
		*/
		register int n = ((dp->nodemem + memsiz - 1) / memsiz);
		if (n <= ((procpernode > 1) ? dmommems : mommems)) {
			dp->mempart = memsiz;
			dp->nodemem = (n * memsiz);
			switch (memsiz) {
			case 128:	dp->cost += (dp->memcost = (nodes * mem128cost * n));
					break;
			case 256:	dp->cost += (dp->memcost = (nodes * mem256cost * n));
					break;
			case 512:	dp->cost += (dp->memcost = (nodes * mem512cost * n));
					break;
			}
		} else {
			if (verbose) printf("%d nodes rejected:  need more nodes to get enough memory\n", nodes);
			return;
		}
	}

	/* Compute network requirements */
	switch (dp->nettype = nettype) {
	case NETNONE:
		if (nodes > 1) {
			if (verbose) printf("%d nodes rejected:  need a network\n", nodes);
			return;
		}
		dp->nicspern = 0;
		dp->netsw = 0;
		dp->bisect = 0;
		goto nonnet;
	case NETGBSAN:
		dp->nicspern = 1;
		dp->netsw = 0;
		if (nodes > 2) {
			dp->netsw = 1;
			if (nodes > 8) {
				dp->netsw = 2;
				if (nodes > 14) {
					/* Two or three level tree */
					dp->netsw = (((nodes + 55) / 56) +
						     ((nodes + 6) / 7));
				}
			}
		}
		dp->cost += (sancost * nodes);
		dp->bisect = (nodes * 1000);
		break;
	case NETGBE:
		if (maxlatency <= 10) {
			if (verbose) printf("%d nodes rejected:  Gb/s Ethernet latency too high\n", nodes);
			return;
		}
		if ((8000 / nodes) < minbandwidth) {
			if (verbose) printf("%d nodes rejected:  Gb/s Ethernet only provides 8Gb/s bisection bandwidth\n", nodes);
			return;
		}
		dp->nicspern = 1;
		dp->netsw = 0;
		if (nodes > 2) {
			dp->netsw = 1;
			if (nodes > 8) {
				dp->netsw = 2;
				if (nodes > 14) {
					/* Two or three level tree */
					dp->netsw = (((nodes + 55) / 56) +
						     ((nodes + 6) / 7));
				}
			}
		}
		dp->cost += (swg8cost * dp->netsw);
		dp->bisect = ((nodes < 8) ? (nodes * 1000) : 8000);
		break;
	case NETFE:
		if (nodes > 64) {
			if (verbose) printf("%d nodes rejected:  Fast Ethernet needs wider switch\n", nodes);
			return;
		}
		if (maxlatency <= 10) {
			if (verbose) printf("%d nodes rejected:  100Mb/s Fast Ethernet latency too high\n", nodes);
			return;
		}
		if (minbandwidth > 500) {
			if (verbose) printf("%d nodes rejected:  100Mb/s Fast Ethernet cannot provide enough bandwidth\n", nodes);
			return;
		}

		/* Use Fast Ethernet(s), possibly channel bonded */
		dp->nicspern = ((minbandwidth + 99) / 100);
		dp->nicspern += (dp->nicspern < 1);
		dp->netsw = ((nodes > 2) ? dp->nicspern : 0);
		dp->cost += (((nodes <= 8) ? swfe8cost :
			      ((nodes <= 16) ? swfe16cost :
			       ((nodes <= 32) ? swfe32cost :
			        ((nodes <= 48) ? swfe48cost :
			          swfe64cost)))) * dp->netsw);
		dp->cost += (nodes * dp->nicspern * nicfecost);
		dp->bisect = (nodes * dp->nicspern * 100);
		break;
	case NETFNN:
		if (nodes <= dp->swwidth) {
			if (verbose) printf("%d nodes rejected:  FNN using a single switch is Channel Bonding\n", nodes);
			return;
		}
		if (maxlatency <= 10) {
			if (verbose) printf("%d nodes rejected:  100Mb/s Fast Ethernet latency too high\n", nodes);
			return;
		}
		if ((minbandwidth > 500) ||
		    (minbandwidth > (coordinality * 100))) {
			if (verbose) printf("%d nodes rejected:  100Mb/s Fast Ethernet cannot provide enough bandwidth\n", nodes);
			return;
		}

		/* Use Flat Neighborhood made of Fast Ethernet */
		dp->nicspern = 1000;
		switch (dp->swwidth) {
		case 8:
			if (26 >= nodes) dp->nicspern = 5;
			if (24 >= nodes) dp->nicspern = 4;
			if (17 >= nodes) dp->nicspern = 3;
			if (12 >= nodes) dp->nicspern = 2;
		case 16:
			if (52 >= nodes) dp->nicspern = 5;
			if (48 >= nodes) dp->nicspern = 4;
			if (32 >= nodes) dp->nicspern = 3;
			if (24 >= nodes) dp->nicspern = 2;
		case 32:
			if (104 >= nodes) dp->nicspern = 5;
			if (96 >= nodes) dp->nicspern = 4;
			if (64 >= nodes) dp->nicspern = 3;
			if (48 >= nodes) dp->nicspern = 2;
		case 48:
			if (160 >= nodes) dp->nicspern = 5;
			if (144 >= nodes) dp->nicspern = 4;
			if (96 >= nodes) dp->nicspern = 3;
			if (72 >= nodes) dp->nicspern = 2;
		case 64:
			if (210 >= nodes) dp->nicspern = 5;
			if (192 >= nodes) dp->nicspern = 4;
			if (128 >= nodes) dp->nicspern = 3;
			if (96 >= nodes) dp->nicspern = 2;
		}
		if (dp->nicspern < ((minbandwidth + 99) / 100)) {
			dp->nicspern = ((minbandwidth + 99) / 100);
		}
		if (dp->nicspern > ((minbandwidth + 99) / 100)) {
			/* Prunes the ones we will try later... */
			return;
		}
		dp->netsw = ((nodes * dp->nicspern + dp->swwidth - 1) /
			     dp->swwidth);
		dp->cost += (((nodes <= 8) ? swfe8cost :
			      ((nodes <= 16) ? swfe16cost :
			       ((nodes <= 32) ? swfe32cost :
			        ((nodes <= 48) ? swfe48cost :
			          swfe64cost)))) * dp->netsw);
		dp->cost += (nodes * dp->nicspern * nicfecost);
		dp->bisect = (nodes * dp->nicspern * 100);
		break;
	}
	/* Do we also need an AFN? */
	if (afnlatency <= 10) {
		dp->nettype |= NETAFN;
	} else if (afnlatency <= 100) {
		if (dp->nettype >= NETGBE) {
			dp->nettype |= NETAFN;
			dp->cost += (afucost * nodes);
		}
	}
nonnet:
	/* Set afus count */
	dp->afus = ((dp->nettype & NETAFN) ?
		    (1 + ((pp->nodes - 2) / 3)) :
		    0);

	/* What about video? */
	dp->video = video;
	if (video == -1) dp->video = nodes;
	if (dp->video > nodes) {
		if (verbose) printf("%d nodes rejected:  cannot have more than 1 display per node\n", nodes);
		return;
	}

	/* Can we power this many nodes, switches, and video? */
	dp->amps = ((nodes * nodeamps) +
		    (dp->netsw * swamps) +
		    (dp->afus * afuamps) +
		    (dp->video * videoamps));
	if (amps < dp->amps) {
		if (verbose) printf("%d nodes rejected:  too much power required\n", nodes);
		return;
	}

	/* Do the nodes and switch fit in physical space?
	   Here, we assume that video is separate....
	   A switch takes a 1U or about 1/3 of a PC case.
	*/
	if (((nodes + dp->netsw + ((dp->afus + 1) / 2) + nperrack - 1) / nperrack) > racks) {
		if (verbose) printf("%d nodes rejected:  too big for even 1U racks\n", nodes);
		return;
	}
	dp->userack = ((((nodes + ((((dp->afus + 1) / 2) + dp->netsw + 2) / 3)) + npershelf - 1) /
			npershelf) > racks);
	if (dp->userack && (dp->nicspern > 2)) {
		if (verbose) printf("%d nodes rejected:  too many NICs for a 1U node\n", nodes);
		return;
	}
	if (dp->userack) {
		dp->racks = ((nodes + dp->netsw + nperrack - 1) / nperrack);
		dp->cost += rackcost * dp->racks;
		dp->cost += rack1ucost * nodes;
	} else {
		dp->shelves = (((nodes + ((dp->netsw + 2) / 3)) + npershelf - 1) / npershelf);
		dp->cost += shelfcost * ((dp->shelves + 1) / 2);
		dp->cost += casecost * nodes;
	}

	/* Can we hold enough disk in this many nodes? */
	dp->disk = ((disksize + nodes - 1) / nodes);
	{
		/* Can do using various disk sizes...
		   use whatever is cheapest, prefer bigger in
		   case of a tie
		*/
		register int n2 = ((dp->disk + disksize2 - 1) / disksize2) + useraid;
		register int n1 = ((dp->disk + disksize1 - 1) / disksize1) + useraid;
		register int n0 = ((dp->disk + disksize0 - 1) / disksize0) + useraid;
		register int nodedisks = (dp->userack ? max1udisk : maxpcdisk);
		if (usebackup) {
			n2 *= 2;
			n1 *= 2;
			n0 *= 2;
		}
		if ((n2 <= nodedisks) &&
		    ((n2 * diskcost2) <= (n0 * diskcost0)) &&
		    ((n2 * diskcost2) <= (n1 * diskcost1))) {
			dp->diskpart = disksize2;
			dp->disk = (n2 * disksize2);
			dp->cost += (n2 * diskcost2);
		} else if ((n1 <= nodedisks) &&
			   ((n1 * diskcost1) <= (n0 * diskcost0)) &&
			   ((n1 * diskcost1) <= (n2 * diskcost2))) {
			dp->diskpart = disksize1;
			dp->disk = (n1 * disksize1);
			dp->cost += (n1 * diskcost1);
		} else if (n0 <= nodedisks) {
			dp->diskpart = disksize0;
			dp->disk = (n0 * disksize0);
			dp->cost += (n0 * diskcost0);
		} else {
			if (verbose) printf("%d nodes rejected:  need more nodes to get enough disk\n", nodes);
			return;
		}
	}
	if (dp->disk == 0) {
		/* Add a floppy for netboot */
		dp->cost += floppycost;
	}

	/* Within budget? */
	if (dp->cost > budget) {
		if (verbose) printf("%d nodes rejected:  too expensive for budget\n", nodes);
		return;
	}

	/* Everything worked!
	   Record this option and try another....
	*/
	++dp;
}

filter(register int nodes)
{
	register int i = 1;

	switch (nodecon) {
	case 10000:
		return(0);
	case 10001:
		/* Power of 2? */
		return(nodes & (nodes - 1));
	case 10002:
		/* Power of 2 + 1? */
		return((nodes - 1) & (nodes - 2));
	case 10003:
		/* Power of 2 + spares? */
		return((nodes - ((nodes + 16) / 16)) &
		       (nodes - ((nodes + 32) / 16)));
	case 10004:
		/* Square? */
		while ((i * i) < nodes) ++i;
		return((i * i) != nodes);
	case 10005:
		/* Square + 1? */
		while (((i * i) + 1) < nodes) ++i;
		return(((i * i) + 1) != nodes);
	case 10006:
		/* Square + spares? */
		while (((i * i) + (((i * i) + 16) / 16)) < nodes) ++i;
		return(((i * i) + (((i * i) + 16) / 16)) != nodes);
	case 10007:
		/* Cube? */
		while ((i * i * i) < nodes) ++i;
		return((i * i * i) != nodes);
	case 10008:
		/* Cube + 1? */
		while (((i * i * i) + 1) < nodes) ++i;
		return(((i * i * i) + 1) != nodes);
	case 10009:
		/* Cube + spares? */
		while (((i * i * i) + (((i * i * i) + 16) / 16)) < nodes) ++i;
		return(((i * i * i) + (((i * i * i) + 16) / 16)) != nodes);
	}

	return(nodes != nodecon);
}

evals(char *procname,
int procpernode,
int proccost,
int momcost,
int minbandwidth,
double nodeamps,
double gflops)
{
	register int bandw;
	register int memsiz;

	for (memsiz=128; memsiz<=512; memsiz+=memsiz) {
		if (nodes < 2) {
			eval(procname, procpernode, proccost, momcost, memsiz, minbandwidth, nodeamps, gflops, NETNONE);
		} else {
			eval(procname, procpernode, proccost, momcost, memsiz, minbandwidth, nodeamps, gflops, NETGBSAN);
			eval(procname, procpernode, proccost, momcost, memsiz, minbandwidth, nodeamps, gflops, NETGBE);
			for (bandw=minbandwidth; bandw<=500; bandw+=100) {
				eval(procname, procpernode, proccost, momcost, memsiz, bandw, nodeamps, gflops, NETFE);
				dp->swwidth = 8;
				eval(procname, procpernode, proccost, momcost, memsiz, bandw, nodeamps, gflops, NETFNN);
				dp->swwidth = 16;
				eval(procname, procpernode, proccost, momcost, memsiz, bandw, nodeamps, gflops, NETFNN);
				dp->swwidth = 32;
				eval(procname, procpernode, proccost, momcost, memsiz, bandw, nodeamps, gflops, NETFNN);
				dp->swwidth = 48;
				eval(procname, procpernode, proccost, momcost, memsiz, bandw, nodeamps, gflops, NETFNN);
				dp->swwidth = 64;
				eval(procname, procpernode, proccost, momcost, memsiz, bandw, nodeamps, gflops, NETFNN);
			}
		}
	}
}
