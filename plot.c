/*	plot.c

	Code to generate a plot as a PNG image
*/

#include "config.h"
#include "rules.h"

#define TMP_HTML_PATH   "TMP/"
#define	TMP_LEN		strlen(TMP_HTML_PATH)

static	char pmtfile[1024];

char *
plot(void)
{
	char tmpfile[1024], buf[1024 * 64];

	FILE *tmp, *pmt;
	register designt *p;

	sprintf(tmpfile, TMP_HTML_PATH "plot_XXXXXX");
	if (mkstemp(tmpfile) == 0) {
		return(0);
	}
	if ((tmp = fopen(tmpfile, "w+")) == 0) {
		return(0);
	}
	chmod(tmpfile, (S_IRUSR | S_IWUSR |
			S_IRGRP | S_IWGRP |
			S_IROTH | S_IWOTH));

	sprintf(pmtfile, TMP_HTML_PATH "plot_%d.png", getpid());
	if ((pmt = fopen(pmtfile, "w+")) == 0) {
		return(0);
	}
	chmod(pmtfile, (S_IRUSR | S_IWUSR |
			S_IRGRP | S_IWGRP |
			S_IROTH | S_IWOTH));

	fprintf(tmp,
"set terminal png small\n"
"set xlabel \"Total Cost of Cluster\"\n"
"set ylabel \"Percentage of Best Metric Value\"\n"
"set key left Left reverse\n"
"plot");

#define	PLOTANY(cond, label) \
	for (p=&(design[0]); p<dp; ++p) { \
		if (cond) { \
			if (notfirst++) { \
				fprintf(tmp, ","); \
			} \
			fprintf(tmp, \
				" \"-\" title \"" \
				label \
				"\" with points ps 2 "); \
			fflush(tmp); \
			break; \
		} \
	}

	{
		register int notfirst = 0;

		PLOTANY((p->net_type == NET_NONE), "No Network (single node)");
		PLOTANY((p->net_type == NET_DIRECT), "Direct Connections");
		PLOTANY((p->net_type == (NET_DIRECT | NET_CB)), "Direct Connections with Channel Bonding");
		PLOTANY((p->net_type == NET_RING), "Ring");
		PLOTANY((p->net_type == (NET_RING | NET_CB)), "Ring with Channel Bonding");
		PLOTANY((p->net_type == NET_2DMESH), "2D Mesh");
		PLOTANY((p->net_type == (NET_2DMESH | NET_CB)), "2D Mesh with Channel Bonding");
		PLOTANY((p->net_type == NET_3DMESH), "3D Mesh");
		PLOTANY((p->net_type == (NET_3DMESH | NET_CB)), "3D Mesh with Channel Bonding");
		PLOTANY((p->net_type == NET_SWITCH), "Single-Switch Network");
		PLOTANY((p->net_type == (NET_SWITCH | NET_CB)), "Single-Switch with Channel Bonding");
		PLOTANY((p->net_type == (NET_SWITCH | NET_FNN)), "Flat Neighborhood Network");
		PLOTANY((p->net_type == NET_FABRIC), "Switch Fabric (Tree)");
		PLOTANY((p->net_type == (NET_FABRIC | NET_CB)), "Switch Fabric (Tree) with Channel Bonding");
		PLOTANY((p->net_type == (NET_FABRIC | NET_FNN)), "Flat Neighborhood Network of Switch Fabrics (Trees)");
		fprintf(tmp, "\n");
	}

#define	PLOT(xvalue, yvalue, cond, format) \
	{ \
		register int any = 0; \
		for (p=&(design[0]); p<dp; ++p) { \
			if (cond) { \
				fprintf(tmp, format, xvalue, yvalue); \
				++any; \
			} \
		} \
		if (any) fprintf(tmp, "e\n"); \
	}

	PLOT(p->cost, ((100.0 * p->metric) / (dp+2)->metric), (p->net_type == NET_NONE), "%f %f\n")
	PLOT(p->cost, ((100.0 * p->metric) / (dp+2)->metric), (p->net_type == NET_DIRECT), "%f %f\n")
	PLOT(p->cost, ((100.0 * p->metric) / (dp+2)->metric), (p->net_type == (NET_DIRECT | NET_CB)), "%f %f\n")
	PLOT(p->cost, ((100.0 * p->metric) / (dp+2)->metric), (p->net_type == NET_RING), "%f %f\n")
	PLOT(p->cost, ((100.0 * p->metric) / (dp+2)->metric), (p->net_type == (NET_RING | NET_CB)), "%f %f\n")
	PLOT(p->cost, ((100.0 * p->metric) / (dp+2)->metric), (p->net_type == NET_2DMESH), "%f %f\n")
	PLOT(p->cost, ((100.0 * p->metric) / (dp+2)->metric), (p->net_type == (NET_2DMESH | NET_CB)), "%f %f\n")
	PLOT(p->cost, ((100.0 * p->metric) / (dp+2)->metric), (p->net_type == NET_3DMESH), "%f %f\n")
	PLOT(p->cost, ((100.0 * p->metric) / (dp+2)->metric), (p->net_type == (NET_3DMESH | NET_CB)), "%f %f\n")
	PLOT(p->cost, ((100.0 * p->metric) / (dp+2)->metric), (p->net_type == NET_SWITCH), "%f %f\n")
	PLOT(p->cost, ((100.0 * p->metric) / (dp+2)->metric), (p->net_type == (NET_SWITCH | NET_CB)), "%f %f\n")
	PLOT(p->cost, ((100.0 * p->metric) / (dp+2)->metric), (p->net_type == (NET_SWITCH | NET_FNN)), "%f %f\n")
	PLOT(p->cost, ((100.0 * p->metric) / (dp+2)->metric), (p->net_type == NET_FABRIC), "%f %f\n")
	PLOT(p->cost, ((100.0 * p->metric) / (dp+2)->metric), (p->net_type == (NET_FABRIC | NET_CB)), "%f %f\n")
	PLOT(p->cost, ((100.0 * p->metric) / (dp+2)->metric), (p->net_type == (NET_FABRIC | NET_FNN)), "%f %f\n")

	fclose(tmp);

	sprintf(buf, GNUPLOT" <%s >%s", tmpfile, pmtfile);
	system(buf);
	unlink(tmpfile);

	/* pmtfile is left around... return file name */
	return(pmtfile);
}
