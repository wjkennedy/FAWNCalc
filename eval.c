/*	eval.c

	Evaluation of configurations....
*/
/* TODO: need more accurate bundling; eg:
 *	- need to reduce number of bundles based on number of previously included components from other
 *	  bundles
 */
#include <time.h>
#include "rules.h"

#define BIG_DBL  (1.0e30)

#define	MIN(a,b)	(((a)<(b))?(a):(b))
#define	MAX(a,b)	(((a)>(b))?(a):(b))

/* set max to max fld in ar */
#define ARRAY_MAX(max,ar,fld) \
{ \
	int opt; \
        (max) = ar[0].fld; \
	for(opt = 1 ; opt < ar##s ; opt++) { \
		if ((max) < ar[opt].fld) { \
			(max) = ar[opt].fld; \
		} \
	} \
}

/* set min to min fld in ar */
#define ARRAY_MIN(min,ar,fld) \
{ \
	int opt; \
        (min) = ar[0].fld; \
	for(opt = 1 ; opt < ar##s ; opt++) { \
		if ((min) > ar[opt].fld) { \
			(min) = ar[opt].fld; \
		} \
	} \
}

designt best_unavailable = {0, 0};

designt	design[MAXDESIGNS + 3];
designt	*dp = &(design[0]);

/* the following may represent impossible combinations of parts */
metric_t best;			/* metric for the best possible config */

#ifdef DEBUG_METRIC_DISCARD
int     bad_discard = 0;
int     discards = 0;
#endif

int	designs_ok = 0;
int	designs_tried = 0;
int	designs_limited = 0;
int	stuck_at = 0;
double	cheapest = 1000000000;

#ifdef DEBUG_NUM_TRIES
int     num_filter = 0;
int	num_net = 0;
int	num_cab = 0;
int	num_proc = 0;
int	num_mem = 0;
int	num_kase = 0;
int	num_disk = 0;
int 	num_done = 0;
#endif /* DEBUG_NUM_TRIES */ 

int	searchsecs = 1;
int	searchdone = 0;

/* max and min number of nodes to try */
int max_search_nodes;
int min_search_nodes;

static int check_all(designt *p, char **strv, int num_strs);

inline double amps2tons(double amps)
{
	/* To be conservative, assume a power factor of 1. */
	return amps * 0.5 * 120.0 / 3500.0 ;
}

/* ceiling(num/denom) using integer math */
static inline int ceil_div(int num, int denom)
{
	return (num + denom - 1) / denom;
}

#define BUNDLE_PUSH(dp,idx,part,cnt) \
{ \
	if (dp->bundles >= MAXOPTS) { \
		err_exit("Too many bundles added at one time\n"); \
	} \
	dp->bundle[dp->bundles].n = ceil_div((cnt), bundle_opt[idx]. part##_qty); \
	dp->cost += (dp->bundle[dp->bundles].n * bundle_opt[idx].cost); \
	dp->bundle[dp->bundles++].id = idx; \
}

#define  BUNDLE_POP(dp) \
{ \
	--dp->bundles; \
	dp->cost -= (dp->bundle[dp->bundles].n * bundle_opt[dp->bundle[dp->bundles].id].cost); \
}

#define DEC_NEEDED(dp,part) { \
	int bdl; \
	for(bdl = 0 ; bdl < dp->bundles ; bdl++) { \
		if (bundle_opt[dp->bundle[bdl].id]. part == dp-> part \
		    && bundle_opt[dp->bundle[bdl].id]. part##_qty > 0) { \
			dp-> part##s = MAX( dp->part##s - (dp->bundle[bdl].n \
					   * bundle_opt[dp->bundle[bdl].id]. part##_qty), 0); \
		} \
	} \
}

extern inline void
estats(register designt *p)
{

#define	ESTAT(field) \
{\
	int bund; \
	if (p->field##s > 0) {\
		++(field##_opt[p->field].estat);\
	} else {\
		for(bund = 0 ;\
		    ((bund < p->bundles)\
		     && ((p->field != bundle_opt[p->bundle[bund].id]. field)\
			 || (bundle_opt[p->bundle[bund].id]. field##_qty < 1))) ; \
		    bund++)\
			;\
		if (bund < p->bundles) { \
			++(field##_opt[p->field].estat); \
		} \
	}\
}


	ESTAT(nic);
	ESTAT(cab);
	ESTAT(sw);
	ESTAT(proc);
	ESTAT(mom);
	ESTAT(mem);
	ESTAT(disk);
	ESTAT(kase);
	ESTAT(rack);
}

/* return a value greater than or equal to the actual metric if this design
 * does prove to be viable
 */
inline double partialmetric(register designt *p, int where)
{
	/* Compute metric value of partial configuration (assuming p->n is set correctly) */
	double metric = 0;

	if (where > SA_MEM3) {
		if (datasize > 0) metric += (metmem * ((p->usrmem - datasize) / datasize));
	} else {
		if (datasize > 0) metric += (metmem * ((p->n * best.usrmem - datasize) / datasize));
	}
	if (where >= SA_MEM2) {
		if (minmemgbs > 0) metric += (metmemgbs * ((p->usrmemgbs - minmemgbs) / minmemgbs));
	} else {
		if (minmemgbs > 0) metric += (metmemgbs
					      * ((MAX(best.usrmemgbs * dp->n, minmemgbs) - minmemgbs)
						 / minmemgbs));
	}
	if (where > SA_DISK1) {
		if (disksize > 0) metric += (metdisk * ((p->usrdisk - disksize) / disksize));
	} else { 
		if (disksize > 0) metric += (metdisk * ((dp->n * best.usrdisk - disksize) / disksize));
	}
	if (where > SA_NET0) {
		if (p->net_latency > 0) metric += (metlat * ((p->net_latency - maxlatency)
							     / p->net_latency));
		if (minbandwidth > 0) metric += (metbw * ((p->net_bandwidth - minbandwidth)
							  / minbandwidth));
	} else {
		/* don't really know how to estimate cost without knowing the network type;
		 * calling this function before network type is known will not do any good
		 * if latency or bandwidth are part of your metric.
		 */
		if (p->net_latency > 0) metric += metlat * BIG_DBL;
		if (minbandwidth > 0) metric += metbw * BIG_DBL;
	}

	/* for now assume all additional parts are free */
	if (budget > 0)
		metric += (metcost * ((budget - p->cost) / budget));
	else
		/* this should never happen; budget should always be > 0 */
		metric += metcost * BIG_DBL;

	if (where > SA_PROC3) {
		if (gflops > 0) metric += (metgflops * ((p->usrgflops - gflops) / gflops));
	} else {
		if (gflops > 0)
			metric += (metgflops * ((dp->n * best.procs * best.usrgflops - gflops) / gflops));
	}
	
	if (where > SA_AMPS0) {
		if (amps > 0) metric += (metamps * ((amps - p->usramps) / amps));
	} else {
		if (amps > 0) metric += (metamps * ((amps - best.usramps) / amps));
	}

	if (where > SA_TONS0) {
		if (tons > 0) metric += (mettons * ((tons - p->usrtons) / tons));
	} else {
		if (tons > 0) metric += (mettons * ((tons - best.usrtons) / tons));
	}
	return(metric);
}

/* returns true if the metric will never be good enough for this design to be recorded */
static inline int metric_prune(where) 
{ 
	if (((dp - &design[0] + 1) > howmany) 
	    && (partialmetric(dp, where) < (dp-1)->metric)) { 
#ifdef DEBUG_METRIC_DISCARD
		discards++; 
		dp->discard = where;
#endif
		return 1; 
	}
        return 0; 
}


extern inline double
metricvalue(register designt *p)
{
	/* Compute metric value of configuration */
	double metric = 0;

	if (datasize > 0) metric += (metmem * ((p->usrmem - datasize) / datasize));
	if (minmemgbs > 0) metric += (metmemgbs * ((p->usrmemgbs - minmemgbs) / minmemgbs));
	if (disksize > 0) metric += (metdisk * ((p->usrdisk - disksize) / disksize));
	if (maxlatency > 0) metric += (metlat * ((maxlatency - p->net_latency) / maxlatency));
	if (minbandwidth > 0) metric += (metbw * ((p->net_bandwidth - minbandwidth) / minbandwidth));
	if (budget > 0) metric += (metcost * ((budget - p->cost) / p->cost));
	if (gflops > 0) metric += (metgflops * ((p->usrgflops - gflops) / gflops));
	if (amps > 0) metric += (metamps * ((amps - p->usramps) / amps));
	if (tons > 0) metric += (mettons * ((tons - p->usrtons) / tons));

	return(metric);
}

#define PRINT_UNAVAIL(fld) if (!fld##_opt[p->fld].available) printf("%s unavailable\n<br>", fld##_opt[p->fld].name);
#define PRINT_MATCH(fld) if (fld##_opt[p->fld].match) printf("%s matches\n<br>", fld##_opt[p->fld].name);

inline int check_availability(designt *p)
{
	int available;

	/* this part is available if all parts are in stock... */
	available = (nic_opt[p->nic].available
		     && cab_opt[p->cab].available
		     && sw_opt[p->sw].available
		     && proc_opt[p->proc].available
		     && mom_opt[p->mom].available
		     && mem_opt[p->mem].available
		     && disk_opt[p->disk].available
		     && kase_opt[p->kase].available
		     && rack_opt[p->rack].available);

	/* ... and we're not doing any limiting or
	 * we are only include matching parts and at least one part matches
	 * or we are not excluding matching parts and none of the parts matches.
	 */
	if (limit) {
		available = available
		  /* no include strings or one at least component included */
		  && ((include_strs == 0) || (nic_opt[p->nic].include
					      || cab_opt[p->cab].include
					      || sw_opt[p->sw].include
					      || proc_opt[p->proc].include
					      || mom_opt[p->mom].include
					      || mem_opt[p->mem].include
					      || disk_opt[p->disk].include
					      || kase_opt[p->kase].include
					      || rack_opt[p->rack].include))
		  /* no exclude strings or no component excluded */
		  && ((exclude_strs == 0) || !(nic_opt[p->nic].exclude
					       || cab_opt[p->cab].exclude
					       || sw_opt[p->sw].exclude
					       || proc_opt[p->proc].exclude
					       || mom_opt[p->mom].exclude
					       || mem_opt[p->mem].exclude
					       || disk_opt[p->disk].exclude
					       || kase_opt[p->kase].exclude
					       || rack_opt[p->rack].exclude));
		if (available && (include_strs > 1) && (include_logic == LIMIT_ALL)) {
			available = check_all(p, include_str, include_strs);
		}
		if (available && (exclude_strs > 1) && (exclude_logic == LIMIT_ALL)) {
			available = !check_all(p, exclude_str, exclude_strs);
		}
	}
	return available;
}

/* return true if all strings present in design p */
static int check_all(designt *p, char **strv, int num_strs)
{
	int str;
	int present;

	present = 1;
	for (str = 0 ; str < num_strs && present ; str++) {
		present = (strstr(nic_opt[p->nic].name, strv[str])
			   || strstr(nic_opt[p->nic].ntype, strv[str])
			   || strstr(cab_opt[p->cab].name, strv[str])
			   || strstr(cab_opt[p->cab].ntype, strv[str])
			   || strstr(sw_opt[p->sw].name, strv[str])
			   || strstr(sw_opt[p->sw].ntype, strv[str])
			   || strstr(proc_opt[p->proc].name, strv[str])
			   || strstr(proc_opt[p->proc].ptype, strv[str])
			   || strstr(mom_opt[p->mom].name, strv[str])
			   || strstr(mom_opt[p->mom].ptype, strv[str])
			   || strstr(mom_opt[p->mom].mtype, strv[str])
			   || strstr(mom_opt[p->mom].ctype, strv[str])
			   || strstr(mem_opt[p->mem].name, strv[str])
			   || strstr(mem_opt[p->mem].mtype, strv[str])
			   || strstr(disk_opt[p->disk].name, strv[str])
			   || strstr(kase_opt[p->kase].name, strv[str])
			   || strstr(kase_opt[p->kase].ctype, strv[str])
			   || strstr(kase_opt[p->kase].rtype, strv[str])
			   || strstr(rack_opt[p->rack].name, strv[str])
			   || strstr(rack_opt[p->rack].rtype, strv[str]));
	}

	return present;
}

extern inline void
e_done(void)
{
	/* evaluation is done...
	   insertion sort and/or prune
	*/
	register designt *p, *q;

#ifdef DEBUG_NUM_TRIES
	num_done++;
#endif /* DEBUG_NUM_TRIES */ 

	/* Are we out of time? */
	if (timeup) return;

	/* Another design tried... */
	++designs_tried;

	/* Is this where things stopped? */
	if (stuck_at < SA_COST0) stuck_at = SA_COST0;

	/* Compute total cost...
	   used to use totalcost(), now incremental
	*/
	if (budget < dp->cost) {
		/* Too expensive... */
		if (cheapest > dp->cost) {
			cheapest = dp->cost;
		}
#ifdef DEBUG_METRIC_DISCARD
		dp->discard = 0;
#endif
		return;
	}

	/* Compute metric... */
	dp->metric = metricvalue(dp);

	if (!check_availability(dp)) {
		designs_limited++;
		if (dp->metric > best_unavailable.metric)
			best_unavailable = *dp;
		return;
	}
	/* Another design met the specs... */
	++designs_ok;
	estats(dp);

	/* Find dp's sorted position */
	for (p=(dp-1); p>=&(design[0]); --p) {
		if (p->metric >= dp->metric) {
			++p;
			goto got_place;
		}
	}
	p = &(design[0]);

got_place:

#ifdef DEBUG_METRIC_DISCARD
	if (dp->discard && (p->metric < dp->metric)) {
		bad_discard++;
	}
#endif /* DEBUG_METRIC_DISCARD */

	/* Shift p..dp back one position */
	for (q=dp; q>=p; --q) {
		*(q+1) = *q;
	}

	/* Copy the new entry into place */
	*p = *(dp+1);

	/* Should we be extending the list? */
	if (dp == &(design[howmany])) {
		/* No, restore dp to new entry value */
		*dp = *(dp+1);
	} else {
		/* Yes; bump dp */
		++dp;
	}
#ifdef DEBUG_METRIC_DISCARD
	dp->discard = 0;
#endif
}

static void e_disk_add(int disk_bdl)
{
	/* Incremental cost update */
	register double cost = dp->cost;

	/* For each viable number of disks...
	   Only allow one read-only (boot) disk
	   Usebackup implies disks%2=0
	   Useraid implies +1 (before backup)
	   Have enough total disk space
	*/

	/* Is this where things stopped? */
	if (stuck_at < SA_DISK1) stuck_at = SA_DISK1;

	for (dp->diskpnode=MIN(mom_opt[dp->mom].ide, kase_opt[dp->kase].ide);
	     dp->diskpnode>0;
	     --(dp->diskpnode)) {

		if (((dp->diskpnode < 2) || (disk_opt[dp->disk].gb != 0))
		    && ((!usebackup) || ((dp->diskpnode & 1) == 0))
		    && ((!useraid) || (dp->diskpnode > (1 + usebackup)))) {

			/* First, account for backup space... */
			dp->usrdisk = dp->diskpnode;
			if (usebackup) dp->usrdisk /= 2;

			/* Then account for +1 RAID */
			if (useraid) --(dp->usrdisk);

			/* Convert to GB from drive count */
			dp->usrdisk *= disk_opt[dp->disk].gb;

			/* Then remove swap space (mem is in MB, disk in GB) */
			dp->usrdisk -= (((int) (swapsize * dp->mempmom + 1023)) / 1024);

			/* Now make it the space for all non-spare nodes... */
			dp->usrdisk *= dp->n;
					
			if (disksize <= dp->usrdisk) {
				dp->ddisks = dp->disks = (dp->diskpnode * (dp->n + dp->spares));
				DEC_NEEDED(dp, disk);
				if (disk_bdl >= 0) {
					BUNDLE_PUSH(dp, disk_bdl, disk, dp->disks);
					dp->disks = 0;
				} else {
					dp->cost += (dp->disks * disk_opt[dp->disk].cost);
				}
				if (stuck_at < SA_DISK2) stuck_at = SA_DISK2;
				
				if (budget >= dp->cost)
					e_done();

				if (disk_bdl >= 0) {
					BUNDLE_POP(dp);
				}
				
				/* Restore cost */
				dp->cost = cost;
			}
		}
	}
}

extern inline void
e_disk(void)
{
	int curbund;

#ifdef DEBUG_NUM_TRIES
	num_disk++;
#endif /* DEBUG_NUM_TRIES */ 
	    
	/* Are we out of time? */
	if (timeup) return;

	/* Is this where things stopped? */
	if (stuck_at < SA_DISK0) stuck_at = SA_DISK0;

	if (mom_opt[dp->mom].pxe && disksize == 0) {
		/* if we have PXE and don't need local disk, then try diskless */
		dp->usrdisk = 0;
		dp->diskpnode = 0;
		dp->ddisks = dp->disks = 0;
		e_done();
	}

	/* For each bundle in which disks are the first component */
	for (curbund = disk_bund ;
	     curbund < bundle_opts && bundle_opt[curbund].disk_qty > 0 ;
	     curbund++) {
		dp->disk = bundle_opt[curbund].disk;
		e_disk_add(curbund);
	}
	/* For each disk part... */
	for (dp->disk=0; dp->disk<disk_opts; ++(dp->disk)) {
		e_disk_add(-1);
	}
}

static void e_rack_add(int kase_bdl, int rack_bdl)
{
	/* Incremental cost update */
	register double cost = dp->cost;

	/* Is this where things stopped? */
	if (stuck_at < SA_RACK1) stuck_at = SA_RACK1;
	
	dp->dkases = dp->kases = (dp->n + dp->spares);
	dp->dracks = dp->racks = ((((kase_opt[dp->kase].u * dp->kases) +
				    (sw_opt[dp->sw].u * dp->sws)) +
				   rack_opt[dp->rack].u - 1) /
				  rack_opt[dp->rack].u);
	if ((dp->racks * rack_opt[dp->rack].floor) <= racks) {
		DEC_NEEDED(dp, kase);
		if (kase_bdl >= 0) {
			BUNDLE_PUSH(dp, kase_bdl, kase, dp->kases);
			dp->kases = 0;
		}
		DEC_NEEDED(dp, rack);
		if (rack_bdl >= 0) {
			BUNDLE_PUSH(dp, rack_bdl, rack, dp->racks);
			dp->racks = 0;
		}
		dp->cost += ((dp->kases * kase_opt[dp->kase].cost) +
			     (dp->racks * rack_opt[dp->rack].cost));
		
		if (stuck_at < SA_RACK2) stuck_at = SA_RACK2;
		
		if (budget >= dp->cost) {
			e_disk();
		}

		if (rack_bdl >= 0) {
			BUNDLE_POP(dp);
		}
		if (kase_bdl >= 0) {
			BUNDLE_POP(dp);
		}

		/* Restore cost */
		dp->cost = cost;
	}
}

static void e_kase_add(int kase_bdl)
{
	int curbund;

	/* Is this where things stopped? */
	if (stuck_at < SA_AMPS0) stuck_at = SA_AMPS0;

	/* Check power... */
	dp->usramps = ((dp->sws * sw_opt[dp->sw].amps) +
		       (dp->nodes * kase_opt[dp->kase].amps));
	if (amps < dp->usramps) {
		/* Insufficient power */
		return;
	}

	if (stuck_at < SA_TONS0) stuck_at = SA_TONS0;
	/* check cooling */
	dp->usrtons = amps2tons(dp->usramps);
	if(tons < dp->usrtons)
		/* Insufficient cooling */
		return;

		
	/* Is this where things stopped? */
	if (stuck_at < SA_RACK0) stuck_at = SA_RACK0;

	if (metric_prune(SA_RACK0))
		return;

	/* For each bundle in which a rack is the first component */
	for (curbund = rack_bund;
	     curbund < bundle_opts && bundle_opt[curbund].rack_qty > 0 ;
	     curbund++) {
		dp->rack = bundle_opt[curbund].rack;
		if (kase_opt[dp->kase].rtype == rack_opt[dp->rack].rtype) {
			e_rack_add(kase_bdl, curbund);
		}
	}
	/* For each viable rack/shelving... */
	for (dp->rack=0; dp->rack<rack_opts; ++(dp->rack)) {
		if (kase_opt[dp->kase].rtype == rack_opt[dp->rack].rtype) {
			e_rack_add(kase_bdl, -1);
		}
	}
}

extern inline void
e_kase(void)
{
	int curbund;
#ifdef DEBUG_NUM_TRIES
	num_kase++;
#endif /* DEBUG_NUM_TRIES */ 

	/* Are we out of time? */
	if (timeup) return;

	/* Is this where things stopped? */
	if (stuck_at < SA_KASE0) stuck_at = SA_KASE0;

	if (metric_prune(SA_KASE0)) {
		return;
	}

	/* For each bundle in which cases are the first thing available */
	for(curbund = kase_bund;
	    curbund < bundle_opts && bundle_opt[curbund].kase_qty > 0 ;
	    curbund++) {
		dp->kase = bundle_opt[curbund].kase;
		e_kase_add(curbund);
	}

	/* For each kase option... */
	for (dp->kase=0; dp->kase<kase_opts; ++(dp->kase)) {
		e_kase_add(-1);
	}
}

static void e_mem_add(int mem_bdl)
{
	/* Incremental cost update */
	register double cost = dp->cost;
	register double usrgflops = dp->usrgflops;
	register int mems;

	/* Is this where things stopped? */
	if (stuck_at < SA_MEM1) stuck_at = SA_MEM1;

	/* Do we have enough memory bandwidth? */
	dp->usrmemgbs = ((mem_opt[dp->mem].gbs * dp->n) / dp->usrgflops);
	if (dp->usrmemgbs < minmemgbs) {
		dp->usrgflops *= (dp->usrmemgbs / minmemgbs);
		dp->usrmemgbs = minmemgbs;
	}
	if (dp->usrgflops >= gflops) {

		/* Is this where things stopped? */
		if (stuck_at < SA_MEM2) stuck_at = SA_MEM2;

		if (metric_prune(SA_MEM2))
			return;

		/* For each viable number of mem/mom... */
		for (dp->mempmom=mom_opt[dp->mom].mem;
		     dp->mempmom>=1;
		     --(dp->mempmom)) {

				/* Is this where things stopped? */
			if (stuck_at < SA_MEM3) stuck_at = SA_MEM3;

			dp->usrmem = (dp->n *
				      ((mem_opt[dp->mem].mb * dp->mempmom) -
				       (ossize + codesize)));
			if (dp->usrmem >= datasize) {
				mems = (dp->mempmom * (dp->n + dp->spares));
				dp->dmems = dp->mems = mems;
				DEC_NEEDED(dp, mem);
				if (mem_bdl >= 0) {
					BUNDLE_PUSH(dp, mem_bdl, mem, mems);
					dp->mems = 0;
				} else {
					dp->cost += (dp->mems * mem_opt[dp->mem].cost);
				}

				if (stuck_at < SA_MEM4) stuck_at = SA_MEM4;

				if (budget >= dp->cost) {
					e_kase();
				}

				if (mem_bdl >= 0) {
					BUNDLE_POP(dp);
				}
				dp->cost = cost;
			}
		}
	}

	/* Undo any adjustment made... */
	dp->usrgflops = usrgflops;
}

extern inline void
e_mem(void)
{
	int curbund;
#ifdef DEBUG_NUM_TRIES
	num_mem++;
#endif /* DEBUG_NUM_TRIES */ 

	/* Are we out of time? */
	if (timeup) return;

	/* Is this where things stopped? */
	if (stuck_at < SA_MEM0) stuck_at = SA_MEM0;

	if (metric_prune(SA_MEM0))
		return;

	for (curbund = mem_bund ;
	     curbund < bundle_opts && bundle_opt[curbund].mem_qty > 0 ;
	     curbund++) {
		dp->mem = bundle_opt[curbund].mem;
		if (mom_opt[dp->mom].mtype == mem_opt[dp->mem].mtype) {
			e_mem_add(curbund);
		}
	}
	/* For each viable memory part size... */
	for (dp->mem=0; dp->mem<mem_opts; ++(dp->mem)) {
		if (mom_opt[dp->mom].mtype == mem_opt[dp->mem].mtype) {
			e_mem_add(-1);
		}
	}
}

static void e_proc_add(int proc_bdl)
{
	register int procs;

	/* Is this where things stopped? */
	if (stuck_at < SA_PROC1) stuck_at = SA_PROC1;

	/* For each viable number of proc/mom... */
	for (dp->procpmom=mom_opt[dp->mom].n;
	     dp->procpmom>=1;
	     --(dp->procpmom)) {
		/* Incremental cost update */
		register int cost = dp->cost;

		/* Filter by processor count constraints, if any */
		if (nodecon0 == 1) {
			register int i = 1, j;

			switch (nodecon1) {
			default:
				/* No constraints */
				break;
			case 1:
				/* Power of 2? */
				j = (dp->procpmom * dp->n);
				if (j & (j - 1)) continue;
				break;
			case 2:
				/* Square? */
				j = (dp->procpmom * dp->n);
				while ((i * i) < j) ++i;
				if ((i * i) != j) continue;
				break;
			case 3:
				/* Cube? */
				j = (dp->procpmom * dp->n);
				while ((i * i * i) < j) ++i;
				if ((i * i * i) != j) continue;
				break;
			case 4:
				/* Exact count */
				j = (dp->procpmom * dp->n);
				if (j != nodecon2) continue;
			}
		}

		/* Is this where things stopped? */
		if (stuck_at < SA_PROC2) stuck_at = SA_PROC2;

		if ((dp->net_bandwidth / (dp->nodes * dp->procpmom)) >= minbandwidth) {

				/* Is this where things stopped? */
			if (stuck_at < SA_PROC3) stuck_at = SA_PROC3;

			procs = dp->procpmom * (dp->n + dp->spares);
			dp->usrgflops = ((dp->procpmom * dp->n) * proc_opt[dp->proc].gflops);

			if (dp->usrgflops >= gflops) {
				dp->dprocs = dp->procs = procs;
				DEC_NEEDED(dp, proc);
				if (proc_bdl >=0) {
					BUNDLE_PUSH(dp, proc_bdl, proc, procs);
					dp->procs = 0;
				} else {
					dp->cost += (dp->procs * proc_opt[dp->proc].cost);
				}

				if (stuck_at < SA_PROC4) stuck_at = SA_PROC4;

				if (budget >= dp->cost)
					e_mem();

				if (proc_bdl >= 0) {
					BUNDLE_POP(dp);
				}
			}
		}

		/* Restore cost */
		dp->cost = cost;
	}
}

extern inline void
e_proc(void)
{
	int curbund;
#ifdef DEBUG_NUM_TRIES
	num_proc++;
#endif /* DEBUG_NUM_TRIES */ 

	/* Are we out of time? */
	if (timeup) return;

	/* Is this where things stopped? */
	if (stuck_at < SA_PROC0) stuck_at = SA_PROC0;

	for(curbund = proc_bund ;
	    curbund < bundle_opts && bundle_opt[curbund].proc_qty > 0 ;
	    curbund++) {
		dp->proc = bundle_opt[curbund].proc;
		if (mom_opt[dp->mom].ptype == proc_opt[dp->proc].ptype) {
			e_proc_add(curbund);
		}
	}

	/* For each processor usable in that mom... */
	for (dp->proc=0; dp->proc<proc_opts; ++(dp->proc)) {
		if (mom_opt[dp->mom].ptype == proc_opt[dp->proc].ptype) {
			e_proc_add(-1);
		}
	}
}

static void e_cab_add(int cab_bdl)
{
	register int cost = dp->cost;

	DEC_NEEDED(dp, cab);
	if (cab_bdl >= 0) {
		BUNDLE_PUSH(dp, cab_bdl, cab, dp->cabs);
		dp->cabs = 0;
	}
		
	/* Incremental cost update */
	dp->cost += (dp->cabs * cab_opt[dp->cab].cost);
	if (stuck_at < SA_CAB1) {
		stuck_at = SA_CAB1;
	}
	if (budget >= dp->cost) {
		e_proc();
	}

	if (cab_bdl >= 0) {
		BUNDLE_POP(dp);
	}

	/* Restore cost */
	dp->cost = cost;
}

extern inline void
e_cab(void)
{
	/* Incremental cost update */
	register int cost = dp->cost;
	int curbund;
	int cabs;

#ifdef DEBUG_NUM_TRIES
	num_cab++;
#endif /* DEBUG_NUM_TRIES */ 

	/* Are we out of time? */
	if (timeup) return;

	if (dp->net_latency > maxlatency) return;

	if(metric_prune(SA_CAB0))
		return;

	dp->cost += ((dp->nics * nic_opt[dp->nic].cost) +
		    (dp->sws * sw_opt[dp->sw].cost));

	if (stuck_at < SA_NET1) stuck_at = SA_NET1;

	if (budget < dp->cost) {
		/* we've run out of money; try something else */
		goto restore_cost;
	}

	/* Is this where things stopped? */
	if (stuck_at < SA_CAB0) stuck_at = SA_CAB0;

	cabs = dp->cabs;
	for(curbund = cab_bund ;
	    curbund < bundle_opts && bundle_opt[curbund].cab_qty > 0 ;
	    curbund++) {
		dp->cab = bundle_opt[curbund].cab;
		if (cab_opt[dp->cab].ntype == nic_opt[dp->nic].ntype) {
			e_cab_add(curbund);
			dp->cabs = cabs;
		}
	}

	/* For each compatible cable... */
	for (dp->cab=0; dp->cab<cab_opts; ++(dp->cab)) {
		if (cab_opt[dp->cab].ntype == nic_opt[dp->nic].ntype) {
			e_cab_add(-1);
			dp->cabs = cabs;
		}
	}

restore_cost:
	/* Restore cost */
	dp->cost = cost;
}

static inline void e_nonet(void)
{
	/* No network needed */
	if (dp->nodes == 1) {
		dp->net_type = NET_NONE;
		dp->nic = 0;
		dp->nicpnode = 0;
		dp->dnics = dp->nics = 0;
		dp->cab = 0;
		dp->sw = 0;
		dp->dsws = dp->sws = 0;
		dp->net_latency = 0;
		dp->net_bandwidth = 1000;

		/* Note that we don't use any cables */
		dp->dcabs = dp->cabs = 0;
		e_proc();
	}
}

static void e_direct_nic(int nic_bdl)
{
	for (dp->nicpnode=(dp->nodes-1);
	     dp->nicpnode<=mom_opt[dp->mom].pci;
	     dp->nicpnode+=(dp->nodes-1)) {
		dp->net_type = ((dp->nicpnode >= dp->nodes) ? (NET_DIRECT | NET_CB) : NET_DIRECT);
		dp->dnics = dp->nics = (dp->nicpnode * (dp->n + dp->spares));
		dp->dcabs = dp->cabs = (dp->nics / 2);
		dp->sw = 0;
		dp->dsws = dp->sws = 0;
		DEC_NEEDED(dp, nic);
		if (nic_bdl >= 0) {
			BUNDLE_PUSH(dp, nic_bdl, nic, dp->nics);
			dp->nics = 0;
		}
		dp->net_latency = (2 * nic_opt[dp->nic].us);
		dp->net_bandwidth = (nic_opt[dp->nic].mbs
				     * (dp->nicpnode * dp->nodes));
		e_cab();
		if (nic_bdl >= 0) {
			BUNDLE_POP(dp);
		}
	}
}

static inline void e_direct()
{
	int curbund;
	
	/* Direct connection possible? */

		/* For each bundle in which NIC is the first component... */
	for (curbund = nic_bund ;
	     curbund < bundle_opts && bundle_opt[curbund].nic_qty > 0 ;
	     curbund++) {
		dp->nic = bundle_opt[curbund].nic;
		e_direct_nic(curbund);
	}
	
	/* For each NIC type... */
	for (dp->nic=0; dp->nic<nic_opts; ++(dp->nic)) {
		e_direct_nic(-1);
	}
}

static inline double ring_latency(designt *dp)
{
	return ((dp->nodes / 2) * (NET_FORWARD_US + (2 * nic_opt[dp->nic].us)));
}

static inline double ring_bw(designt *dp)
{
	return nic_opt[dp->nic].mbs * dp->nicpnode;
}


static void e_ring_nic(int nic_bdl)
{
	dp->sw = 0;
	dp->dsws = dp->sws = 0;
	dp->net_x = dp->nodes;
	dp->net_y = 0;
	dp->net_z = 0;
	/* Ring (switchless) connection? */
	for (dp->nicpnode=2;
	     dp->nicpnode<=mom_opt[dp->mom].pci;
	     dp->nicpnode+=2) {
		dp->net_type = ((dp->nicpnode > 2) ? (NET_RING | NET_CB) : NET_RING);
		dp->dnics = dp->nics = (dp->nicpnode * (dp->n + dp->spares));
		dp->dcabs = dp->cabs = (dp->nics / 2);
		DEC_NEEDED(dp, nic);
		if (nic_bdl >= 0) {
			BUNDLE_PUSH(dp, nic_bdl, nic, dp->nics);
			dp->nics = 0;
		}
		dp->net_latency = ring_latency(dp);
		dp->net_bandwidth = ring_bw(dp);
		e_cab();
		if (nic_bdl >= 0) {
			BUNDLE_POP(dp);
		}
	}
}

static inline void e_ring(void)
{
	int curbund;

	/* For each bundle in which NIC is the first component... */
	for(curbund = nic_bund ;
	    curbund < bundle_opts && bundle_opt[curbund].nic_qty > 0 ;
	    curbund++) {
		dp->nic = bundle_opt[curbund].nic;
		e_ring_nic(curbund);
	}
	/* For each NIC type... */
	for (dp->nic=0; dp->nic<nic_opts; ++(dp->nic)) {
		e_ring_nic(-1);
	}
}

static inline double mesh2d_latency(designt *dp)
{
	return (((dp->net_x / 2) + (dp->net_y / 2))
		* (NET_FORWARD_US + (2 * nic_opt[dp->nic].us)));
}

static inline double mesh2d_bw(designt *dp)
{
	return (dp->net_y * nic_opt[dp->nic].mbs * dp->nicpnode / 2);
}

static void e_2dmesh_nic(int nic_bdl)
{
	dp->sw = 0;
	dp->dsws = dp->sws = 0;
	for (dp->nicpnode=4; dp->nicpnode<=mom_opt[dp->mom].pci; dp->nicpnode+=4) {
		dp->net_type = ((dp->nicpnode > 4) ? (NET_2DMESH | NET_CB) : NET_2DMESH);
		dp->dnics = dp->nics = (dp->nicpnode * (dp->n + dp->spares));
		dp->dcabs = dp->cabs = (dp->nics / 2);
		dp->net_latency =  mesh2d_latency(dp);
		dp->net_bandwidth = mesh2d_bw(dp);
		DEC_NEEDED(dp, nic);
		if (nic_bdl >= 0) {
			BUNDLE_PUSH(dp, nic_bdl, nic, dp->nics);
			dp->nics = 0;
		}
		e_cab();
		if (nic_bdl >= 0) {
			BUNDLE_POP(dp);
		}
	}
}

static inline void e_2dmesh(void)
{
	register int i, j = dp->nodes;
	int curbund;

	/* 2D torroid mesh (switchless) connection? */
	if (mom_opt[dp->mom].pci < 4) {
		return;
	}

	/* find i and j such that if the cluster has i rows
	 * and j columns it is as close to square as possible
	 */
	for (i=2; (i*i)<=dp->nodes; ++i) {
		if ((dp->nodes % i) == 0) {
			j = i;
		}
	}
	dp->net_x = MAX((dp->nodes / j), j);
	dp->net_y = MIN((dp->nodes / j), j);
	dp->net_z = 0;

	if (dp->net_y > 1) {
		/* For each bundle in which NIC is the first component... */
		for( curbund = nic_bund ;
		     curbund < bundle_opts && bundle_opt[curbund].nic_qty > 0 ;
		     curbund++) {
			dp->nic = bundle_opt[curbund].nic;
			e_2dmesh_nic(curbund);
		}

		/* For each NIC type... */
		for (dp->nic=0; dp->nic<nic_opts; ++(dp->nic)) {
			e_2dmesh_nic(-1);
		}
	}
}

static inline double mesh3d_latency(designt *dp)
{
	return(((dp->net_x / 2) + (dp->net_y / 2) + (dp->net_z / 2)) *
	       (NET_FORWARD_US + (2 * nic_opt[dp->nic].us)));
}

static inline double mesh3d_bw(designt *dp)
{
	return (dp->net_z * dp->net_y * nic_opt[dp->nic].mbs * dp->nicpnode / 3);
}

static void e_3dmesh_nic(int nic_bdl)
{
	dp->sw = 0;
	dp->dsws = dp->sws = 0;
	for (dp->nicpnode=6; dp->nicpnode<=mom_opt[dp->mom].pci; dp->nicpnode+=6) {
		dp->net_type = ((dp->nicpnode > 6) ? (NET_3DMESH | NET_CB) : NET_3DMESH);
		dp->dnics = dp->nics = (dp->nicpnode * (dp->n + dp->spares));
		dp->dcabs = dp->cabs = (dp->nics / 2);
		dp->net_latency =  mesh3d_latency(dp);
		dp->net_bandwidth = mesh3d_bw(dp);
		DEC_NEEDED(dp, nic);
		if (nic_bdl >= 0) {
			BUNDLE_PUSH(dp, nic_bdl, nic, dp->nics);
			dp->nics = 0;
		}
		e_cab();
		if (nic_bdl >= 0) {
			BUNDLE_POP(dp);
		}
	}
}

static void inline e_3dmesh(void)
{
	register int i, j, k;
	int curbund;

	if (mom_opt[dp->mom].pci < 6) {
		return;
	}

	/* 3D torroid mesh (switchless) connection? */
	k = dp->nodes;
	dp->net_x = dp->nodes;
	dp->net_y = 1;
	dp->net_z = 1;
	for (i=2; (i*i*i)<=dp->nodes; ++i) {
		for (j=2; j<=i; ++j) {
			if ((dp->nodes % (i * j)) == 0) {
				k = (dp->nodes / (i * j));
				dp->net_x = i;
				dp->net_y = j;
				dp->net_z = k;
			}
		}
	}
	if (dp->net_y > dp->net_x) {
		register int t = dp->net_y;
		dp->net_y = dp->net_x;
		dp->net_x = t;
	}
	if (dp->net_z > dp->net_y) {
		register int t = dp->net_y;
		dp->net_y = dp->net_z;
		dp->net_z = t;
	}
	if (dp->net_y > dp->net_x) {
		register int t = dp->net_y;
		dp->net_y = dp->net_x;
		dp->net_x = t;
	}

	if ((dp->net_z > 1) && (dp->net_y > 1)) {
		/* For each bundle in which NIC is the first component... */
		for (curbund = nic_bund ;
		     curbund < bundle_opts && bundle_opt[curbund].nic_qty > 0 ;
		     curbund++) {
			dp->nic = bundle_opt[curbund].nic;
			e_3dmesh_nic(curbund);
		}
		/* For each NIC type... */
		for (dp->nic=0; dp->nic<nic_opts; ++(dp->nic)) {
			e_3dmesh_nic(-1);
		}
	}
}

/* sws_bdl = bundle id if switches came from a bundle, else -1 */
static inline void e_switch_nic(int sw_bdl, int nic_bdl)
{
	/* For each number of NICs per node... */
	for (dp->nicpnode=1; dp->nicpnode<=mom_opt[dp->mom].pci; ++(dp->nicpnode)) {
		dp->net_type = ((dp->nicpnode > 1) ? (NET_SWITCH | NET_CB) : NET_SWITCH);
		dp->dcabs = dp->cabs = (dp->nicpnode * (dp->n + dp->spares));
		dp->dnics = dp->nics = dp->cabs;
		dp->dsws = dp->sws = dp->nicpnode;
		dp->net_latency = (sw_opt[dp->sw].us + (2 * nic_opt[dp->nic].us));
		dp->net_bandwidth = MIN((nic_opt[dp->nic].mbs * dp->sws * dp->nodes),
					(sw_opt[dp->sw].mbs * dp->sws));
		DEC_NEEDED(dp, sw);
		if (sw_bdl >= 0) {
			/* we got the switches from a bundle, but didn't know how
			 * many bundles to add at that point, so update now
			 */
			BUNDLE_PUSH(dp, sw_bdl, sw,  dp->sws);
			dp->sws = 0;
		}
		DEC_NEEDED(dp, nic);
		if (nic_bdl >= 0) {
			BUNDLE_PUSH(dp, nic_bdl, nic, dp->nics);
			dp->nics = 0;
		}
		e_cab();
		if (nic_bdl >= 0) {
			BUNDLE_POP(dp);
		}
		if (sw_bdl >= 0) {
			BUNDLE_POP(dp);
		}
	}
}

static void e_switch_sw(int sw_bdl)
{
	int curbund;

	for(curbund = nic_bund ;
	    curbund < bundle_opts && bundle_opt[curbund].nic_qty > 0 ;
	    curbund++) {
		dp->nic = bundle_opt[curbund].nic;
		if (sw_opt[dp->sw].ntype == nic_opt[dp->nic].ntype) {
			e_switch_nic(sw_bdl, curbund);
		}
	}


	for (dp->nic=0; dp->nic<nic_opts; ++(dp->nic)) {
		if (sw_opt[dp->sw].ntype == nic_opt[dp->nic].ntype) {
			e_switch_nic(sw_bdl, -1);
		}
	}
}

/* Simple switch possible? */
static inline void e_switch(void)
{
	int curbund;

	for(curbund = sw_bund ;
	    curbund < bundle_opts && bundle_opt[curbund].sw_qty > 0 ;
	    curbund++) {
		dp->sw = bundle_opt[curbund].sw;
		if(sw_opt[dp->sw].ports >= dp->nodes) {
			e_switch_sw(curbund);
		}
	}
	for (dp->sw=0; dp->sw<sw_opts; ++(dp->sw)) {
		if (sw_opt[dp->sw].ports >= dp->nodes) {
			e_switch_sw(-1);
		}
	}
}

static inline void e_set_sws(int sws_bdl, int sw2s_bdl)
{
	int swidx;
	int bdl; 

	swidx = dp->bundles - 1;
	DEC_NEEDED(dp, sw);
	if (sws_bdl >= 0) {
		BUNDLE_PUSH(dp, sws_bdl, sw, dp->sws);
		dp->sws = 0;
	}
	for(bdl = 0 ; bdl < dp->bundles ; bdl++) { 
		if (bundle_opt[dp->bundle[bdl].id].sw == dp->sw2 ) { 
			dp->sw2s = MAX(dp->sw2s - (dp->bundle[bdl].n 
					  * bundle_opt[dp->bundle[bdl].id].sw_qty), 0); 
		} 
	} 
	if (sw2s_bdl >= 0) {
		BUNDLE_PUSH(dp, sw2s_bdl, sw, dp->sw2s);
		dp->sw2s = 0;
	}
}

static void e_tree_nic(int sw_bdl, int sw2_bdl, int nic_bdl)
{
	register int sws = 1;
	register int sw2s;


	/* how many lower switches do we need? */
	for (sw2s=1;
	     ((sw_opt[dp->sw].ports + sw2s * (sw_opt[dp->sw2].ports - 2)) < dp->nodes);
	     ++sw2s) ;
	
	/* If it is the same switch, use only sws entry */
	if (dp->sw2 == dp->sw) {
		sws += sw2s;
		sw2s = 0;
	}

	/* For each number of NICs per node... */
	for (dp->nicpnode=1; dp->nicpnode<=mom_opt[dp->mom].pci; ++(dp->nicpnode)) {
		dp->net_type = ((dp->nicpnode > 1) ? (NET_FABRIC | NET_CB) : NET_FABRIC);
		dp->dnics = dp->nics = (dp->nicpnode * (dp->n + dp->spares));
		dp->dsws = dp->sws = (dp->nicpnode * sws);
		dp->dsw2s = dp->sw2s = (dp->nicpnode * sw2s);
		e_set_sws(sw_bdl, sw2_bdl);
		dp->dcabs = dp->cabs = (dp->nics + (dp->nicpnode * (sws + sw2s - 1)));
		dp->net_latency = (sw_opt[dp->sw].us + (2 * sw_opt[dp->sw2].us) +
				   (2 * nic_opt[dp->nic].us));
		dp->net_bandwidth = (sw_opt[dp->sw].mbs * dp->nicpnode);
		DEC_NEEDED(dp, nic);
		if (nic_bdl >= 0) {
			BUNDLE_PUSH(dp, nic_bdl, nic, dp->nics);
			dp->nics = 0;
		}
		e_cab();
		if (nic_bdl >= 0) {
			BUNDLE_POP(dp);
		}
	}
	dp->dsw2s = dp->sw2s = 0;
}

static void e_tree_sw(int sw_bdl, int sw2_bdl)
{
	int curbund;

	for (curbund = nic_bund ;
	     curbund < bundle_opts && bundle_opt[curbund].nic_qty > 0 ;
	     curbund++) {
		dp->nic = bundle_opt[curbund].nic;
		if (sw_opt[dp->sw].ntype == sw_opt[dp->nic].ntype) {
			e_tree_nic(sw_bdl, sw2_bdl, curbund);
		}
	}
	for (dp->nic=0; dp->nic<nic_opts; ++(dp->nic)) {
		if (sw_opt[dp->sw].ntype == sw_opt[dp->nic].ntype) {
			e_tree_nic(sw_bdl, sw2_bdl, -1);
		}
	}
}

/* Then pick same or smaller switch for lower level... */
static void e_tree_sw2(int sws_bdl)
{
	int curbund;

	for (curbund = sw_bund ;
	     curbund < bundle_opts && bundle_opt[curbund].sw_qty > 0 ;
	     curbund++) {
		dp->sw2 = bundle_opt[curbund].sw;
		if ( (sw_opt[dp->sw2].ntype == sw_opt[dp->sw].ntype)
		    && (sw_opt[dp->sw2].ports <= sw_opt[dp->sw].ports)
		    && ((sw_opt[dp->sw].ports * (sw_opt[dp->sw2].ports - 1)) >= dp->nodes) ) {
			e_tree_sw(sws_bdl, curbund);
		}
	}
		
	for (dp->sw2=0; dp->sw2<sw_opts; ++(dp->sw2)) {
		if ( (sw_opt[dp->sw2].ntype == sw_opt[dp->sw].ntype)
		     && (sw_opt[dp->sw2].ports <= sw_opt[dp->sw].ports)
		     && ( (sw_opt[dp->sw].ports * (sw_opt[dp->sw2].ports - 1)) >= dp->nodes) ) {
			e_tree_sw(sws_bdl, -1);
		}
	}
}

static void e_tree(void)
{
	int curbund;

	/* Two-level tree switch fabric?
	   Cheap switches cannot handle trunking, so fat tree topology isn't an option.
	   First, pick (big) switch for top level...
	*/
	for (curbund = sw_bund ; curbund < bundle_opts && bundle_opt[curbund].sw_qty > 0 ; curbund++) {
		dp->sw = bundle_opt[curbund].sw;
		if( (sw_opt[dp->sw].ports < dp->nodes)
		   && ( (sw_opt[dp->sw].ports * (sw_opt[dp->sw].ports - 1)) >= dp->nodes) ) {
			e_tree_sw2(curbund);
		}
	}
	for (dp->sw=0; dp->sw<sw_opts; ++(dp->sw)) {
		if ( (sw_opt[dp->sw].ports < dp->nodes)
		     && ( (sw_opt[dp->sw].ports * (sw_opt[dp->sw].ports - 1)) >= dp->nodes)) {
			e_tree_sw2(-1);
		}
	}
}

static inline void e_fnn_perf(void)
{
	/* Compute approximate FNN performance...
	   originally, just used the upper bound (high);
	   now we average the lower (low) and upper (high) bounds,
	   but weight them based on the coordinality....
	   This might not be right, but it sure is complex!
	*/
	register int i = ((dp->sws * sw_opt[dp->sw].ports) / dp->nodes);
	register int j = ((dp->sws * sw_opt[dp->sw].ports) % dp->nodes);
	register double low, high;
	
	i *= (sw_opt[dp->sw].ports * (sw_opt[dp->sw].ports - 1));
	if (j > 0) {
		/* More than one switch was not full? */
		if (j > sw_opt[dp->sw].ports) {
			j = (((j / 2) * (j / 2 - 1)) +
			     ((j - (j / 2)) * ((j - (j / 2)) - 1)));
		} else {
			j *= (j - 1);
		}
	}
	low = (((double) i + j) / (dp->nodes * (dp->nodes - 1)));
	low *= MIN(nic_opt[dp->nic].mbs, (sw_opt[dp->sw].mbs / sw_opt[dp->sw].ports));
	high = MIN((nic_opt[dp->nic].mbs * dp->nicpnode * dp->nodes),
		   (sw_opt[dp->sw].mbs * dp->sws));
	i = ((coordinality >= dp->nicpnode) ? dp->nicpnode : coordinality);
	dp->net_latency = (sw_opt[dp->sw].us + (2 * nic_opt[dp->nic].us));
	dp->net_bandwidth = (((i * high) +
			      ((0.5 + dp->nicpnode - i) * low)) /
			     (0.5 + dp->nicpnode));
}

static void e_fnn_nic(int sw_bdl, int nic_bdl)
{
	/* For each valid number of NICs per node... */
	for (dp->nicpnode=nics4fnn(dp->nodes, sw_opt[dp->sw].ports);
	     dp->nicpnode<=mom_opt[dp->mom].pci;
	     ++(dp->nicpnode)) {
		dp->net_type = (NET_SWITCH | NET_FNN);
		dp->dnics = dp->nics = (dp->nicpnode * (dp->n + dp->spares));
		dp->dcabs = dp->cabs = dp->nics;
		dp->dsws = dp->sws = ((dp->nics + sw_opt[dp->sw].ports - 1) /
				      sw_opt[dp->sw].ports);
		e_fnn_perf();
		DEC_NEEDED(dp, sw);
		if (sw_bdl >= 0) {
			BUNDLE_PUSH(dp, sw_bdl, sw, dp->sws);
			dp->sws = 0;
		}
		DEC_NEEDED(dp, nic);
		if (nic_bdl >= 0) {
			BUNDLE_PUSH(dp, nic_bdl, nic, dp->cabs);
			dp->nics = 0;
		}
		e_cab();
		if (nic_bdl >= 0) {
			BUNDLE_POP(dp);
		}
		if (sw_bdl >= 0) {
			BUNDLE_POP(dp);
		}
	}
}

static void e_fnn_sw(sw_bdl)
{
	int curbund;

	/* For each compatible NIC... */
	for (curbund = nic_bund ;
	     curbund < bundle_opts && bundle_opt[curbund].nic_qty > 0 ;
	     curbund++) {
		dp->nic = bundle_opt[curbund].nic;
		if (sw_opt[dp->sw].ntype == nic_opt[dp->nic].ntype) {
			e_fnn_nic(sw_bdl, curbund);
		}
	}
	for (dp->nic=0; dp->nic<nic_opts; ++(dp->nic)) {
		if (sw_opt[dp->sw].ntype == nic_opt[dp->nic].ntype) {
			e_fnn_nic(sw_bdl, -1);
		}
	}
}

static inline void e_fnn(void)
{
	int curbund;
	
	for (curbund = sw_bund ;
	     curbund < bundle_opts && bundle_opt[curbund].sw_qty > 0 ;
	     curbund++) {
		dp->sw = bundle_opt[curbund].sw;
		if(sw_opt[dp->sw].ports < dp->nodes) {
			e_fnn_sw(curbund);
		}
	}
	/* Flat Neighborhood Network? */
	for (dp->sw=0; dp->sw<sw_opts; ++(dp->sw)) {
		if (sw_opt[dp->sw].ports < dp->nodes) {
			e_fnn_sw(-1);
		}
	}
}

static void e_switched_fnn_nic(int sw_bdl, int sw2_bdl, int sw2s, int ports, int nic_bdl)
{
	register int sws;

	/* For each valid number of NICs per node... */
	for (dp->nicpnode=nics4fnn(dp->nodes, ports);
	     dp->nicpnode<=mom_opt[dp->mom].pci;
	     ++(dp->nicpnode)) {
		dp->dnics = dp->nics = (dp->nicpnode * (dp->n + dp->spares));
		dp->dcabs = dp->cabs = dp->nics;
		dp->dsw2s = dp->sw2s = sw2s;
		sws = (dp->sws = ceil_div(dp->nics,ports));
		if (dp->sw == dp->sw2) {
			dp->sw2s = 0;
			dp->sws *= (1 + sw2s);
		} else {
			dp->sw2s = sw2s;
		}
		e_set_sws(sw_bdl, sw2_bdl);
		dp->net_latency = (sw_opt[dp->sw].us + (2 * (sw_opt[dp->sw2].us + nic_opt[dp->nic].us)));
		dp->net_bandwidth = (sw_opt[dp->sw].mbs * sws);
 		dp->nics = (dp->nicpnode * (dp->n + dp->spares));
		DEC_NEEDED(dp, nic);
		if (nic_bdl >= 0) {
			BUNDLE_PUSH(dp, nic_bdl, nic, dp->nics);
			dp->nics = 0;
		}
		e_cab();
		if (nic_bdl >= 0) {
			BUNDLE_POP(dp);
		}
	}
	dp->dsw2s = dp->sw2s = 0;
}

static void e_switched_fnn_sw(int sw_bdl, int sw2_bdl)
{
	int curbund;
	register int sw2s, ports;

	for (sw2s=1;
	     ((sw2s <= sw_opt[dp->sw].ports) &&
	      ((ports = ((sw2s * (sw_opt[dp->sw2].ports - 2)) + sw_opt[dp->sw].ports)) < dp->nodes));
	     ++sw2s) {
		for(curbund = nic_bund ;
		    curbund < bundle_opts && bundle_opt[curbund].nic_qty > 0 ;
		    curbund++) {
			dp->nic = bundle_opt[curbund].nic;
			if (sw_opt[dp->sw].ntype == nic_opt[dp->nic].ntype) {
				e_switched_fnn_nic(sw_bdl, sw2_bdl, sw2s, ports, curbund);
			}
		}
		/* For each compatible NIC... */
		for (dp->nic=0; dp->nic<nic_opts; ++(dp->nic)) {
			if (sw_opt[dp->sw].ntype == nic_opt[dp->nic].ntype) {
				e_switched_fnn_nic(sw_bdl, sw2_bdl, sw2s, ports, -1);
			}
		}
	}
}

static void e_switched_fnn_sw2(int sws_bdl)
{
	int curbund;

	for (curbund = sw_bund ;
	     curbund < bundle_opts && bundle_opt[curbund].sw_qty > 0 ;
	     curbund++) {
		dp->sw2 = bundle_opt[curbund].sw;
		if ( (sw_opt[dp->sw2].ntype == sw_opt[dp->sw].ntype)
		     && (sw_opt[dp->sw2].ports <= sw_opt[dp->sw].ports) ) {
				e_switched_fnn_sw(sws_bdl, curbund);
		}
	}
	/* Then pick same or smaller switch for lower level... */
	for (dp->sw2=0; dp->sw2<sw_opts; ++(dp->sw2)) {
		if ( (sw_opt[dp->sw2].ntype == sw_opt[dp->sw].ntype) 
		     && (sw_opt[dp->sw2].ports <= sw_opt[dp->sw].ports) ) {
			/* For each number of lower-level switches... */
				e_switched_fnn_sw(sws_bdl, -1);
		}
	}
}


static void e_switched_fnn(void)
{
	int curbund;
	
	dp->net_type = (NET_FABRIC | NET_FNN);
	/* Flat Neighborhood Network using switch fabrics?
	   First pick upper switch for tree fabric...
	*/
	for (curbund = sw_bund ;
	     curbund < bundle_opts && bundle_opt[curbund].sw_qty > 0 ;
	     curbund++) {
		dp->sw = bundle_opt[curbund].sw;
		if( sw_opt[dp->sw].ports < dp->nodes) {
			e_switched_fnn_sw2(curbund);
		}
	}
	for (dp->sw=0; dp->sw<sw_opts; ++(dp->sw)) {
		if (sw_opt[dp->sw].ports < dp->nodes) {
			e_switched_fnn_sw2(-1);
		}
	}
}

static void
e_net(void)
{
#ifdef DEBUG_NUM_TRIES
	num_net++;
#endif /* DEBUG_NUM_TRIES */ 

	/* Are we out of time? */
	if (timeup) return;

	/* Is this where things stopped? */
	if (stuck_at < SA_NET0) stuck_at = SA_NET0;

	if(metric_prune(SA_NET0))
		return;

	/* make each of these a separate function so gprof can tell me how long each one takes */
	if (dp->nodes == 1) {
		e_nonet();
		return;
	}
	e_direct();
	e_ring();
	e_2dmesh();
	e_3dmesh();
	e_switch();
	e_tree();
	e_fnn();
	e_switched_fnn();
}

static void
e_filter(register int n)
{
	register int i = 1;
	int curbund;

#ifdef DEBUG_NUM_TRIES
	num_filter++;
#endif /* DEBUG_NUM_TRIES */ 

	/* Are we out of time? */
	if (timeup) return;

	/* Filter by node constraints... */
	if (nodecon0 == 0) switch (nodecon1) {
	default:
		/* No constraints */
		break;
	case 1:
		/* Power of 2? */
		if (n & (n - 1)) return;
		break;
	case 2:
		/* Square? */
		while ((i * i) < n) ++i;
		if ((i * i) != n) return;
		break;
	case 3:
		/* Cube? */
		while ((i * i * i) < n) ++i;
		if ((i * i * i) != n) return;
		break;
	case 4:
		/* Exact count */
		if (n != nodecon2) return;
	}
	dp->n = n;

	/* How many more are needed for spares? */
	switch (sparen0) {
	default:
		dp->spares = ((n + sparen1 - 1) / sparen1);
		dp->nodes = (n + dp->spares);
		break;
	case 1:
		dp->spares = ((n + sparen1 - 1) / sparen1);
		dp->nodes = n;
		break;
	case 2:
		dp->spares = sparen1;
		dp->nodes = (n + dp->spares);
		break;
	case 3:
		dp->spares = sparen1;
		dp->nodes = n;
		break;
	}

	/* Is this where things stopped? */
	if (stuck_at < SA_EVAL1) stuck_at = SA_EVAL1;

	if (metric_prune(SA_EVAL1))
		return;

	/* Check for bundles that have motherboards and include as many as required
	 *  to meet the users's requirements them
	 */
	for (curbund = mom_bund
		     ; curbund < bundle_opts && bundle_opt[curbund].mom_qty > 0
		     ; curbund++) {

		dp->mom = bundle_opt[curbund].mom;
		dp->dmoms = (dp->n + dp->spares);
		BUNDLE_PUSH(dp, curbund, mom, dp->dmoms);
		dp->moms = 0;

		if (stuck_at < SA_EVAL2) stuck_at = SA_EVAL2;

		/* Evaluate networks... if we haven't already spent all our money*/
		if (budget >= dp->cost) {
			e_net();
		}

		BUNDLE_POP(dp);
	}
	
	/* For each viable motherboard... */
	for (dp->mom=0; dp->mom<mom_opts; ++(dp->mom)) {
		/* Incremental cost update */
		register int cost = dp->cost;

		dp->dmoms = dp->moms = (dp->n + dp->spares);
		dp->cost += (dp->moms * mom_opt[dp->mom].cost);

		if (stuck_at < SA_EVAL3) stuck_at = SA_EVAL3;

		/* Evaluate networks... if we haven't already spent all our money*/
		if (budget >= dp->cost) 
			e_net();

		/* Restore cost */
		dp->cost = cost;
	}
}

static int
min_nodes(void)
{
	register int i, j;
	register double flops = proc_opt[0].gflops;
	register int mom_n = mom_opt[0].n;
	register int mom_mem = mom_opt[0].mem;
	register int mem_mb = mem_opt[0].mb;
	register double mem_gbs = mem_opt[0].gbs;

	for (i=1; i<proc_opts; ++i) {
		if (flops < proc_opt[i].gflops) {
			flops = proc_opt[i].gflops;
		}
	}
	for (i=1; i<mom_opts; ++i) {
		if (mom_n < mom_opt[i].n) {
			mom_n = mom_opt[i].n;
		}
		if (mom_mem < mom_opt[i].mem) {
			mom_mem = mom_opt[i].mem;
		}
	}
	for (i=1; i<mem_opts; ++i) {
		if (mem_mb < mem_opt[i].mb) {
			mem_mb = mem_opt[i].mb;
		}
		if (mem_gbs < mem_opt[i].gbs) {
			mem_gbs = mem_opt[i].gbs;
		}
	}

	if ((mem_gbs / flops) < minmemgbs) {
		/* Adjust for insufficient memory bandwidth */
		flops *= ((mem_gbs / flops) / minmemgbs);
	}
	i = ((int) (gflops / (flops * mom_n)));
	j = ((int) (datasize / (mom_mem * mem_mb)));
	i = MAX(i, j);
	i = MAX(i, 1);
	return(i);
		   
}

static int
max_nodes(void)
{
	register int i;
	register double proc_cost = proc_opt[0].cost;
	register double mom_cost = mom_opt[0].cost;
	register int mom_n = mom_opt[0].n;
	register double kase_cost = kase_opt[0].cost;

	for (i=1; i<proc_opts; ++i) {
		if (proc_cost > proc_opt[i].cost) {
			proc_cost = proc_opt[i].cost;
		}
	}
	for (i=1; i<mom_opts; ++i) {
		if (mom_cost > mom_opt[i].cost) {
			mom_cost = mom_opt[i].cost;
		}
		if (mom_n < mom_opt[i].n) {
			mom_n = mom_opt[i].n;
		}
	}
	for (i=1; i<mom_opts; ++i) {
		if (kase_cost > kase_opt[i].cost) {
			kase_cost = kase_opt[i].cost;
		}
	}

	return(1 + ((int) (budget / (proc_cost + ((mom_cost + kase_cost) / mom_n)))));
}


inline int
bitrev(register int highbit,
register int value)
{
	register int lowbit = 1;
	register int result = 0;

	while (lowbit <= highbit) {
		if (value & lowbit) result |= highbit;
		if (value & highbit) result |= lowbit;
		lowbit <<= 1;
		highbit >>= 1;
	}
	return(result);
}
		
static void init_best(void)
{
	double max_mb;
	double max_gbs;
	int    max_mem;
	int    max_mom_ide;
	int    max_kase_ide;
	
	ARRAY_MAX(max_mem, mom_opt, mem);
	ARRAY_MAX(max_mb, mem_opt, mb);
	best.usrmem = max_mem * max_mb - ossize - codesize;

	ARRAY_MAX(best.usrgflops, proc_opt, gflops);
	ARRAY_MAX(best.procs, mom_opt, n);
	ARRAY_MAX(max_gbs, mem_opt, gbs);

	/* assume we can at least get the min by cutting gflops */
	best.usrmemgbs = max_gbs / best.usrgflops;

	ARRAY_MAX(max_mom_ide, mom_opt, ide);
	ARRAY_MAX(max_kase_ide, kase_opt, ide);
	/* TODO: account for raid and backup as well */
	ARRAY_MAX(best.usrdisk, disk_opt, gb);
	best.usrdisk = best.usrdisk * MIN(max_mom_ide, max_kase_ide)
		- (((int)(swapsize * dp->mempmom + 1023)) / 1024);

	/* these are hard to do right and hit early so don't worry about
	 * putting in reasonable values
	 */
	best.net_latency = 1.0 / BIG_DBL;
	best.net_bandwidth = BIG_DBL;

	/* ignore switch power useage when computing best case power */
	ARRAY_MIN(best.usramps, kase_opt, amps);

	/* determine minimum component costs */
	ARRAY_MIN(best.cost.nic, nic_opt, cost);
	ARRAY_MIN(best.cost.cab, cab_opt, cost);
	ARRAY_MIN(best.cost.sw, sw_opt, cost);
	ARRAY_MIN(best.cost.sw2, sw_opt, cost);
	ARRAY_MIN(best.cost.proc, proc_opt, cost);
	ARRAY_MIN(best.cost.mom, mom_opt, cost);
	ARRAY_MIN(best.cost.mem, mem_opt, cost);
	ARRAY_MIN(best.cost.disk, disk_opt, cost);
	ARRAY_MIN(best.cost.kase, kase_opt, cost);
	ARRAY_MIN(best.cost.rack, rack_opt, cost);
}

void
eval(void)
{
	register int range;
	register int bit = 1;
	register int start = time(0);
	register int i;

	max_search_nodes = max_nodes();
	min_search_nodes = min_nodes();
	range = (max_search_nodes - min_search_nodes + 1);

	set_timeout();

	while (bit < range) bit <<= 1;
	if ((bit & range) == 0) bit >>= 1;

	dp->cost = 0;
	dp->dsw2s = dp->sw2s = 0;
#ifdef DEBUG_METRIC_DISCARD
	dp->discard = 0;
#endif
	dp->bundles = 0;

	init_best();

	for (i=0; i<(bit+bit); ++i) {
		register int n = bitrev(bit, i);

		/* multiply best and mincost by num proc where appropriate */

		if (n <= range) {
			e_filter(n + min_search_nodes);

			if (timeup) goto end_early;
		}
	}
end_early:
	searchsecs = (time(0) - start);
	if (!timeup) searchdone = 1;
}
