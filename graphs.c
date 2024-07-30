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
