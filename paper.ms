.de iH
\\$1 \\$2 \\$3 \\$4 \\$5 \\$6 \\$7 \\$8
\\.XS
.if \\n(NS-4 \t
.if \\n(NS-3 \t
.if \\n(NS-2 \t
.if \\n(NS-1 \t
\\*(SN \\$1 \\$2 \\$3 \\$4 \\$5 \\$6 \\$7 \\$8
\\.XE
..
.EQ
delim $$
.EN
.nr PO 0.87i
.nr PO 0.81i
.nr LL 6.75i
.nr LT 6.75i
.nr HM 0.87i
.nr HM 1.06i
.nr FM 0.91i
.nr FM 0.92i
.nr PS 9
.nr VS 12
.EQ
delim $$
gsize 9
.EN
.de 0s
.nr PS 8
.nr VS 10
.br
..
.de 1s
.nr PS 8
.nr VS 10
.br
..
.de 2s
.nr PS 9
.nr VS 11
.br
..
.EH ''''
.OH ''''
.EF ''''
.OF ''''
.de cB
.0s
.LP
.BD
.ft 7
..
.de cE
.DE
.2s
.ft R
.LP
..
.fp 7 CW
.fp 8 CW
.2s
.LP
.CD
\fB\s+3CLUSTER DESIGN RULES\s-2\u\(dg\d\s+2\s-3\fR

H. G. Dietz
Compilers, Hardware Architectures, & Operating Systems (KAOS) lab
Electrical and Computer Engineering Department
University of Kentucky
Lexington, KY 40506-0046
\f7hankd@engr.uky.edu\fR
.DE
.LP
.MC 3.22i
.LP
.nh
Abstract\ \(em
\fIOver the past few years, the concept of building one's own
parallel computer by clustering PC hardware has become popular.
Unfortunately, many of the systems built this way have yielded
disappointing performance on the codes they were built to run.
The problem is primarily that the creation of a high-performance
cluster has required considerable engineering expertise and
effort, and this engineering evaluation was never done.  This
paper discusses how we constructed a software tool that can
evaluate designs to determine reasonable solutions to a wide
range of engineering tradeoffs.
.FE
.NH
.iH Introduction
.LP
.nh
In February 1994 at Purdue University in West Lafayette, IN, a
strange collection of hardware and software became the world's
first parallel computer constructed using Linux PCs as nodes.
It was built as an experimental prototype for testing the
validity of a new model for communication and synchronization,
the \fIAggregate Function\fR communication model that most
people know better by the name of our first implementations of
the hardware: \fIPAPERS\fR (Purdue's Adapter for Parallel
Execution and Rapid Synchronization).  Not only did the new
model work exceedingly well, but the cluster itself, a mere
jumble of 486-based PCs, 10Mb/s Ethernet, and PAPERS was very
competitive with the \*Qreal\*U supercomputers then at Purdue.
So, we began developing technologies aimed directly at making
Linux PC clusters even more effective parallel supercomputers,
and we've been building \fBLOTS\fR of clusters ever since....
.LP
.nh
About the same time that we were working on our first cluster,
Thomas Sterling at CalTech/JPL wrote a white paper introducing
what has become known as the \fIBeowulf\fR concept:  the idea
that the \*Qlow hanging fruit\*U of parallel supercomputing
could be reached using a PC cluster built using only commodity
parts.  The obvious weak link was the network, which Don Becker
was repairing by developing Ethernet \fIchannel bonding\fR.
When their first machine was built late in 1994, it did not
include any custom hardware (although it wasn't all commodity
parts either, due to its rack mounting) and it did run at least
one \*Qreal\*U application well enough to get noticed.
.LP
.nh
Now, 7 years later, everybody is building their own Beowulf.
The problem is that most people have forgotten (or never even
knew) the disclaimers about what these machines can and cannot
do.  That's why we built the cluster design rules program:  to
help people arrive at designs that have a good chance of
actually accomplishing what they were built to do.
.LP
.nh
Of course, even this program needs a disclaimer.  The design of
a cluster supercomputer is every bit as difficult a problem as
designing a traditional supercomputer, so we have had to make
quite a few simplifying assumptions.  In other words, the design
rules are imprecise and the program is working from grossly
insufficient data about your application, etc.  This is a tool
to get you quickly focussed on a manageable portion of the huge
cluster design space, not a tool that generates the ultimate
cluster design for your application.
.NH 2
.iH Networks
.LP
.nh
Still the weakest link in most cluster designs, the network
structure is what most clearly distinguishes a PC cluster from a
bunch of networked PCs.  The basic metrics of network
performance are latency and bandwidth.
.LP
.nh
\fILatency\fR is the minimum time it takes for PCs to
communicate with each other, which largely determines how much
parallelism you can use.  For example, if your code can be
parallelized such that each independent \*Qchunk\*U of work
takes at least 500 microseconds to execute, then your code
probably can tolerate a latency that is on the order of 500
microseconds.  Clusters have much lower latency than the
internet, but making latency very low is very expensive.
.LP
.nh
\fIBandwidth\fR is the maximum amount of data that can be sent
per unit time.  In most parallel applications, nearly all the
PCs are communicating at the same time, so the limit on
bandwidth is often the switches between the nodes rather than
the NICs or speed of the wires.  Thus, rather than measuring
peak bandwidth between a pair of nodes, it is usually more
appropriate to measure the \fIbisection bandwidth\fR.  Bisection
bandwidth is measured by dividing the nodes into two equal-sized
groups and determining the total bandwidth available between the
groups; the minimum value of the maximum bandwidth between
groups for all possible groupings is the bisection bandwidth.
Traditionally, bisection bandwidth was easily determined by
finding the minimum number of network data paths that would need
to be cut to partition the nodes into two groups.  However, in
the cluster world, things are more complex.  There are two major
problems in measuring bisection bandwidth of a cluster network.
.LP
.nh
The first problem is that the simplified evaluation method of
cutting the minimum number of data paths assumes that all data
paths have equal bandwidth\ \(em often, they do not.  The
primary reason is that many switches have internal networks that
provide a bisection bandwidth that is less than the sum of the
bandwidths of their ports.  Thus, the cut should be inside the
internal network of a switch, but the internal switch datapaths
are rarely documented well enough to allow this cut to be
determined.
.LP
.nh
The second problem in determining bisection bandwidth is that,
for many cluster networks, the maximum total bandwidth available
between groups of nodes is not constant, but is a function of
the communication pattern.  Most programs written to determine
bisection bandwidth (incorrectly) assume that the maximum
bandwidth will be achieved using any communication pattern in
which the nodes in both groups are paired such that each node
communicates with only one node in the other group.  Due to the
oddities of the networks within switches, the bisection
bandwidth measured can be different if nodes are paired
differently.  Even more significantly, asymmetric networks like
GA-designed FNNs generally do not achieve their maximum
bisection bandwidth unless each node is communicating with
multiple nodes in the other group.
.LP
.nh




  Even with full speed switches, the problem of
finding the minimum cut is very difficult for asymmetric
networks such as the new GA-designed FNNs.

finding such a cut does not always yield the
correct answer because many network switches are capable of
switching only a fraction of the bandwidth of the cables 


The differences between data parallelism (SIMD execution) and control
parallelism (MIMD execution) are at least superficially quite large.
In a data parallel program, parallelism is specified in terms of
performing the same operation simultaneously on all elements of a data
structure; this naturally fits the SIMD execution model.  It is also
easy to see that, because the abilities of a MIMD are a superset of
the abilities of a SIMD, the data parallel model can be extended to
MIMD targets [11] [7].  However, the control parallel model
suggests that each processor can take its own path independent of all
others, and this characteristic seems to require the multiple
instruction streams possible only in MIMD execution.  Control
parallelism is impossible on a SIMD with only one instruction
stream...  or is it?
.PP
There are two basic approaches that might allow SIMD hardware to
efficiently support a control parallel programming model: \*QMIMD
emulation\*U and \*Qmeta-state conversion.\*U
.NH 2
.iH MIMD Emulation
.PP
Perhaps the most obvious way to make SIMD hardware mimic MIMD execution
is to write a SIMD program that will interpretively execute a MIMD
instruction set. In the simplest terms, such an interpreter has a data
structure, replicated in each SIMD PE, that corresponds to the
internal registers of each MIMD processor.  Likewise, each PE's memory
holds a copy of the MIMD code to be executed.  Hence, the interpreter
structure can be as simple as:
.LP
.KS
.LP
\fBBasic MIMD Interpreter Algorithm\fR
.IP 1. 3
Each PE fetches an \*Qinstruction\*U into its \*Qinstruction
register\*U (IR) and updates its \*Qprogram counter\*U (PC).
.IP 2.
Each PE decodes the \*Qinstruction\*U from its IR.
.IP 3.
Repeat steps 3a-3c for each \*Qinstruction\*U type:
.IP 3.a 5
Disable all PEs where the IR holds an \*Qinstruction\*U of a different
type.
.IP 3.b
Simulate execution of the \*Qinstruction\*U on the enabled PEs.
.IP 3.c
Enable all PEs.
.IP 4. 3
Go to step 1.
.LP
.KE
.LP
The only difficulty in implementing an interpreter with the above
structure is that the simulated machine will be very inefficient.
.PP
A number of researchers have used a wide range of \*Qtricks\*U to
produce more efficient MIMD interpreters [9], [12], and
[3].  However, some overhead cannot be removed:
.IP 1. 3
Instructions must be fetched and decoded.
.IP 2.
Instructions must be accessible to all PEs, hence, each PE typically
will have a copy of the entire MIMD program's instructions.  In a
massively-parallel machine, this wastes a huge amount of memory.
.IP 3.
There will be some overhead associated with the interpreter itself,
e.g., the cost of jumping back to the start of the interpreter loop.
.LP
Although problems 1 and 3 merely slow the execution, the second
severely restricts the size of MIMD programs.  For example, the Purdue
University School of Electrical Engineering has a 16K processing
element MasPar MP-1 [1] with only 16K bytes of local memory for
each PE.  Even with very careful encoding, 16K bytes cannot hold a
very large MIMD program.
.PP
Although meta-state conversion is more difficult to implement and more
restrictive in its abilities, it can eliminate even these three
overhead problems.
.NH 2
.iH Meta-State Conversion
.PP
In MIMD execution, each processor has its own state.  Although these
states are generally considered to be independent entities, it is also
possible to view the set of processor states at a particular time as
single, aggregate, \*QMeta State.\*U Using static analysis based on
the timing described in [6], a compiler can convert the MIMD
program into an automaton based on meta states.
.PP
Once a program has been converted into the form of a meta-state
automaton, it is no longer necessary for each PE to fetch and decode
instructions, nor is it necessary that each PE have a copy of the
program in local memory.  Only the SIMD control unit needs to have a
copy of the meta-state automaton; PEs merely hold data.  Further,
because there is no interpreter, there is no interpretation overhead.
Literally, the meta-state automaton is a SIMD program that preserves
the relative timing properties of MIMD execution.
.PP
However, just as interpretation has drawbacks, so too does meta-state
conversion:
.LP
.IP 1. 3
If there are \fIN\fR processors each of which can be in any of \fIS\fR
states, then it is possible that there may be as many as
\fIS!/(S-N)!\fR states in the meta-state automaton.  Without some
means to ensure that the state space is kept manageable, the technique
is not practical.
.IP 2.
In execution, meta-state transitions are based on examining the
aggregate of the MIMD state transitions for all processors.
.IP 3.
Meta-state transitions are N-way branches keyed by the aggregate
of the MIMD state transitions.
.IP 4.
Dynamic creation of new processes is difficult to accommodate,
since construction of the meta-state automaton requires that all
possible MIMD states can be predicted at compile time.
.LP
Fortunately, we have developed a number of techniques that can control
the state space explosion suggested above.  Making meta-state
transitions based on aggregate information is conceptually simple, but
requires some hardware support, e.g., the \*Qglobal or\*U of the
MasPar MP-1 [1].  The efficient implementation of N-way branches
is a difficult problem, but can be accomplished using customized hash
functions indexing jump tables [5].  Unfortunately, the fully
dynamic creation of processes seems to be impractical\ \(em but that
is exactly the case in which the interpretation scheme works best.
Consequently, this paper focuses on techniques to control the state
explosion, and restricts the input MIMD code to be formulated as an
SPMD program.
.PP
The second section of this paper presents the meta-state conversion
algorithm, using an example to clarify the process.  Section 3
discusses issues involving how the resulting meta-state automaton can
be efficiently encoded for SIMD execution.  In section 4, we discuss
how the prototype implementation was constructed, and give a simple
example of the output generated.  Finally, section five summarizes the
contributions of this work and directions for future study.
.NH
.iH Meta-State Conversion
.PP
The meta-state conversion algorithm is surprisingly straightforward;
perhaps it would be more accurate to say that it is familiar.  The
process of converting a set of MIMD states that exist at a particular
point in time into a single meta state is strikingly similar to the
process of converting an NFA into a DFA, as used in constructing
lexical analyzers.
.PP
To begin, the code for the MIMD processes is converted into a set of
control flow graphs in which each node (MIMD state) represents a basic
block [2].  Each of these MIMD states has zero, one, or two, exit
arcs.  A MIMD state with no exit arcs marks the end of that process.
A single exit arc represents unconditional sequencing (e.g., an
unconditional branch), whereas two exit arcs respectively represent
the \*QTRUE\*U and \*QFALSE\*U successors of that MIMD state (e.g.,
targets of a conditional branch).  In addition, it is assumed that we
know in which particular MIMD state each process begins execution;
these states are called MIMD start states.
.PP
The set of MIMD start states forms the start state of the meta-state
automaton.  Since each MIMD start state may have up to two successors,
each process may pick either of its two possible successors.  If we
further assume that there may be multiple processes in each MIMD state,
it is further possible that \fIboth\fR successors might be chosen.
Hence, for a meta state that consists of one MIMD start state, there
may be as many as three meta-state successors.  In general, from
\fIn\fR MIMD start states, there could be as many as
3\s-2\u\fIn\fR\d\s+2 meta-state successors.
.PP
To clarify the operation of the algorithm, we will trace the
algorithm's actions on a simple example.  The framework for the
example is the following SPMD code:
.KS
.BD
\f7if (\fIA\f7) {
    do { \fIB\f7 } while (\fIC\f7);
} else {
    do { \fID\f7 } while (\fIE\f7);
}
\fIF\fR
.DE
.CD
\fBListing 1:\fR Example MIMD (SPMD) Code
.DE
.KE
.LP
It is assumed that all processors begin executing this code
simultaneously and that processors computing different values for the
parallel expressions \fIA\fR, \fIC\fR, and \fIE\fR are the only
sources of asynchrony (i.e., there are no external interrupts).
.NH 2
.iH Construction of the MIMD Control-Flow Graph
.PP
Before meta-state conversion can be applied, the program must be
converted into a form that facilitates the analysis.  The most
convenient form is that of a traditional control-flow graph in which
each node represents a maximal basic block.  Constructing the
control-flow graph in the usual way, code straightening [2] and
removal of empty nodes are applied to obtain the simplest possible
graph.  The result of this is figure 1.  State 0 corresponds to block
\fIA\fR, state 2 corresponds to \fIB\fR followed by \fIC\fR, state 6
corresponds to \fID\fR followed by \fIE\fR, and state 9 corresponds to
\fIF\fR.
.KS
.BD
.PS 1.25i
M0:	ellipse with .c at 3.21i,5.30i width 0.60i height 0.40i "0"
	ellipse with .c at 2.61i,4.30i width 0.60i height 0.40i "2"
	ellipse with .c at 3.81i,4.30i width 0.60i height 0.40i "6"
	ellipse with .c at 3.21i,3.30i width 0.60i height 0.40i "9"
	line from 3.11i,3.60i to 3.21i,3.50i
	line from 3.21i,3.50i to 3.31i,3.60i
	line from 3.11i,3.60i to 3.31i,3.60i
	line from 2.61i,3.90i to 2.71i,3.80i
	line from 3.11i,3.80i to 3.21i,3.70i
	line from 3.21i,3.70i to 3.31i,3.80i
	line from 3.71i,3.80i to 3.81i,3.90i
	line from 2.51i,4.60i to 2.61i,4.50i
	line from 2.51i,4.60i to 2.71i,4.60i
	line from 2.61i,4.50i to 2.71i,4.60i
	line from 3.71i,4.60i to 3.81i,4.50i
	line from 3.81i,4.50i to 3.91i,4.60i
	line from 3.71i,4.60i to 3.91i,4.60i
	line from 2.61i,4.70i to 2.71i,4.80i
	line from 3.11i,4.80i to 3.21i,4.90i
	line from 3.21i,4.90i to 3.31i,4.80i
	line from 3.71i,4.80i to 3.81i,4.70i
	line from 4.11i,4.30i to 4.21i,4.20i
	line from 4.11i,4.30i to 4.21i,4.40i
	line from 4.21i,4.40i to 4.21i,4.20i
	line from 3.01i,4.40i to 3.01i,4.20i
	line from 2.91i,4.30i to 3.01i,4.40i
	line from 2.91i,4.30i to 3.01i,4.20i
	line from 2.71i,4.80i to 3.11i,4.80i
	line from 3.31i,4.80i to 3.71i,4.80i
	line from 3.31i,3.80i to 3.71i,3.80i
	line from 2.71i,3.80i to 3.11i,3.80i
	line from 3.11i,4.30i to 3.21i,4.20i
	line from 3.21i,4.20i to 3.21i,4.10i
	line from 3.11i,4.00i to 3.21i,4.10i
	line from 3.11i,4.30i to 3.21i,4.20i
	line from 3.21i,4.20i to 3.21i,4.10i
	line from 3.11i,4.00i to 3.21i,4.10i
	line from 3.11i,4.30i to 3.21i,4.20i
	line from 3.21i,4.20i to 3.21i,4.10i
	line from 3.11i,4.00i to 3.21i,4.10i
	line from 3.01i,4.30i to 3.11i,4.30i
	line from 3.01i,4.00i to 3.11i,4.00i
	line from 2.82i,4.16i to 3.01i,4.00i
	line from 4.31i,4.30i to 4.41i,4.20i
	line from 4.41i,4.20i to 4.41i,4.10i
	line from 4.31i,4.00i to 4.41i,4.10i
	line from 4.31i,4.30i to 4.41i,4.20i
	line from 4.41i,4.20i to 4.41i,4.10i
	line from 4.31i,4.00i to 4.41i,4.10i
	line from 4.31i,4.30i to 4.41i,4.20i
	line from 4.41i,4.20i to 4.41i,4.10i
	line from 4.31i,4.00i to 4.41i,4.10i
	line from 4.21i,4.30i to 4.31i,4.30i
	line from 4.21i,4.00i to 4.31i,4.00i
	line from 4.02i,4.16i to 4.21i,4.00i
	line from 2.61i,4.10i to 2.61i,3.90i
	line from 3.21i,3.70i to 3.21i,3.60i
	line from 3.81i,4.10i to 3.81i,3.90i
	line from 2.61i,4.70i to 2.61i,4.60i
	line from 3.81i,4.70i to 3.81i,4.60i
	line from 3.21i,5.10i to 3.21i,4.90i
.PE
.DE
.CD
\fBFigure 1:\fR MIMD State Graph for Listing 1
.DE
.KE
.NH 2
.iH Handling Of Function Calls
.PP
Although our example case does not contain any function calls, it is
important that meta-state conversion be applicable to codes that
contain arbitrary function calls\ \(em perhaps including recursive
function invocations.  Thus, we need some way to represent function
call/return directly using control flow arcs in the MIMD state graph.
.PP
In the case of non-recursive function calls, it is sufficient to use
the traditional solution of in-line expansion of the function code
(i.e., of the MIMD state graph for the function body).  Surprisingly,
recursive function calls also can be treated using in-line
expansion\ \(em and an additional \*Qtrick\*U that converts
\f7return\fR statements into ordinary multiway branches.
.PP
Consider the following C-like code fragment in which the \f7main\fR
program invokes the recursive function \f7g\fR:
.KS
.BD
\f7main()
    \|...
a:  g();
b:  \|...
c:  g();
d:  \|...
}

g()
{
    \|...
    g();
e:  \|...
}\fR
.DE
.CD
\fBListing 2:\fR Example Recursive Function Call
.DE
.KE
.LP
The only difficulty in in-line expanding \f7g\fR is that the target of
any \f7return\fR statements in \f7g\fR is not known until runtime.
However, at compile time we can compute the set of all possible
\f7return\fR targets given that \f7g\fR was initially invoked from
a particular position.
.PP
When in-line expanding the call to \f7g\fR from position \f7a\fR, we
know that any \f7return\fR statements within \f7g\fR must return to
either position \f7b\fR or \f7e\fR, and can replace the \f7return\fR
statements with the appropriate multiway branch.  Likewise, when
in-line expanding \f7g\fR called from position \f7c\fR, \f7return\fR
statements are translated into multiway branches targeting \f7d\fR or
\f7e\fR.  The result is a call-free control flow graph for the entire
program; thus, the meta-state conversion algorithm can ignore the
direct handling of function calls without loss of generality.
.NH 2
.iH Base Conversion Algorithm
.PP
The following C-based pseudo code gives the base algorithm for
meta-state conversion.
.LD
\ 





\ 
.DE
.cB
meta_state_convert(x)
set x;
{
 /* Given the start meta state x,
    generate the rest of the automaton
 */
.cE
.cB
 do {
  /* Mark this meta state as done */
  mark_meta_state_done(x);

  /* Add arcs to meta states y| x\(->y */
  reach(x, x, \(es);
            
  /* Get another meta state */
  x = get_unmarked_meta_state();

  /* Repeat for that meta state */
 } while (x != \(es);
}

int
reach(start, s, t)
set start, s, t;
{
 /* Make entries for all meta states
    t| start\(->t
 */
 if (s == \(es) {
  /* All MIMD state transitions from
     within start have been considered,
     hence, t must be a meta state
  */
  make_meta_state_transition(start, t);
 } else {
  /* Select a MIMD state and process
     its transition(s), recursing to
     complete the meta state
  */
  element e, next, fnext;

  e = [e| e \(mo s];
  s = s - {e};
  next = next_MIMD_state(e);
  fnext = next_MIMD_state_if_false(e);

  /* Take each path and both paths */
  if (next) {
   reach(start, s, t \(cu next);
   if (fnext) {
    reach(start, s, t \(cu fnext);
    reach(start, s, t \(cu next \(cu fnext);
   }
  } else {
   reach(start, s, t);
  }
 }
}
.cE
.PP
Applying the above algorithm to our simple example, the resulting
meta-state graph is given in figure 2.
.KS
.BD
.PS 2.8i
M0:	ellipse with .c at 4.26i,6.42i width 0.60i height 0.40i "\fR0"
	ellipse with .c at 3.06i,5.42i width 0.60i height 0.40i "2"
	ellipse with .c at 4.26i,5.42i width 0.60i height 0.40i "2,6"
	line from 2.96i,5.72i to 3.06i,5.62i
	line from 2.96i,5.72i to 3.16i,5.72i
	line from 3.06i,5.62i to 3.16i,5.72i
	line from 4.16i,5.72i to 4.26i,5.62i
	line from 4.26i,5.62i to 4.36i,5.72i
	line from 4.16i,5.72i to 4.36i,5.72i
	line from 3.06i,5.72i to 3.06i,5.92i
	line from 3.06i,5.92i to 3.16i,6.02i
	line from 4.56i,5.42i to 4.66i,5.32i
	line from 4.76i,5.42i to 4.86i,5.32i
	line from 4.86i,5.32i to 4.86i,5.22i
	line from 4.76i,5.12i to 4.86i,5.22i
	line from 4.56i,5.42i to 4.66i,5.52i
	line from 4.66i,5.52i to 4.66i,5.32i
	line from 3.46i,5.52i to 3.46i,5.32i
	line from 3.36i,5.42i to 3.46i,5.52i
	line from 3.36i,5.42i to 3.46i,5.32i
	line from 4.76i,5.42i to 4.86i,5.32i
	line from 4.86i,5.32i to 4.86i,5.22i
	line from 4.76i,5.12i to 4.86i,5.22i
	line from 5.86i,5.52i to 5.86i,5.32i
	line from 5.76i,5.42i to 5.86i,5.52i
	line from 5.76i,5.42i to 5.86i,5.32i
	line from 5.36i,6.02i to 5.46i,5.92i
	line from 5.46i,5.92i to 5.46i,5.72i
	line from 5.36i,5.72i to 5.56i,5.72i
	line from 5.46i,5.62i to 5.56i,5.72i
	line from 5.36i,5.72i to 5.46i,5.62i
	ellipse with .c at 5.46i,5.42i width 0.60i height 0.40i "6"
	line from 4.26i,6.22i to 4.26i,5.72i
	line from 3.16i,6.02i to 4.16i,6.02i
	line from 4.16i,6.02i to 4.26i,6.12i
	line from 4.26i,6.12i to 4.36i,6.02i
	line from 4.36i,6.02i to 5.36i,6.02i
	line from 4.76i,5.42i to 4.86i,5.32i
	line from 4.86i,5.32i to 4.86i,5.22i
	line from 4.76i,5.12i to 4.86i,5.22i
	ellipse with .c at 5.46i,4.42i width 0.60i height 0.40i "6,9"
	line from 5.36i,4.72i to 5.46i,4.62i
	line from 5.46i,4.62i to 5.56i,4.72i
	line from 5.36i,4.72i to 5.56i,4.72i
	line from 5.76i,4.42i to 5.86i,4.32i
	line from 5.76i,4.42i to 5.86i,4.52i
	line from 5.86i,4.52i to 5.86i,4.32i
	line from 3.36i,4.42i to 3.46i,4.32i
	line from 3.36i,4.42i to 3.46i,4.52i
	line from 3.46i,4.52i to 3.46i,4.32i
	line from 4.66i,4.52i to 4.66i,4.32i
	line from 4.56i,4.42i to 4.66i,4.52i
	line from 4.56i,4.42i to 4.66i,4.32i
	ellipse with .c at 4.26i,4.42i width 0.60i height 0.40i "2,6,9"
	ellipse with .c at 3.06i,4.42i width 0.60i height 0.40i "2,9"
	line from 5.36i,5.12i to 5.46i,5.22i
	line from 5.36i,5.12i to 5.56i,5.12i
	line from 5.46i,5.22i to 5.56i,5.12i
	line from 5.46i,5.12i to 5.46i,5.02i
	line from 5.46i,4.82i to 5.46i,4.72i
	line from 5.26i,4.92i to 5.36i,4.92i
	line from 5.36i,4.92i to 5.46i,4.82i
	line from 5.46i,5.02i to 5.56i,4.92i
	line from 5.56i,4.92i to 5.66i,4.92i
	line from 5.66i,4.92i to 5.76i,4.82i
	line from 5.76i,4.82i to 5.76i,4.72i
	line from 5.16i,5.02i to 5.26i,4.92i
	line from 5.16i,5.02i to 5.16i,5.12i
	line from 5.16i,5.12i to 5.25i,5.28i
	line from 5.67i,4.56i to 5.76i,4.72i
	line from 4.47i,4.56i to 4.56i,4.72i
	line from 3.96i,5.12i to 4.05i,5.28i
	line from 3.96i,5.02i to 3.96i,5.12i
	line from 3.96i,5.02i to 4.06i,4.92i
	line from 4.56i,4.82i to 4.56i,4.72i
	line from 4.46i,4.92i to 4.56i,4.82i
	line from 4.36i,4.92i to 4.46i,4.92i
	line from 4.26i,5.02i to 4.36i,4.92i
	line from 4.16i,4.92i to 4.26i,4.82i
	line from 4.06i,4.92i to 4.16i,4.92i
	line from 4.26i,4.82i to 4.26i,4.72i
	line from 4.26i,5.12i to 4.26i,5.02i
	line from 4.26i,5.22i to 4.36i,5.12i
	line from 4.16i,5.12i to 4.36i,5.12i
	line from 4.16i,5.12i to 4.26i,5.22i
	line from 4.16i,4.72i to 4.36i,4.72i
	line from 4.26i,4.62i to 4.36i,4.72i
	line from 4.16i,4.72i to 4.26i,4.62i
	line from 3.27i,4.56i to 3.36i,4.72i
	line from 2.76i,5.12i to 2.85i,5.28i
	line from 2.76i,5.02i to 2.76i,5.12i
	line from 2.76i,5.02i to 2.86i,4.92i
	line from 3.36i,4.82i to 3.36i,4.72i
	line from 3.26i,4.92i to 3.36i,4.82i
	line from 3.16i,4.92i to 3.26i,4.92i
	line from 3.06i,5.02i to 3.16i,4.92i
	line from 2.96i,4.92i to 3.06i,4.82i
	line from 2.86i,4.92i to 2.96i,4.92i
	line from 3.06i,4.82i to 3.06i,4.72i
	line from 3.06i,5.12i to 3.06i,5.02i
	line from 3.06i,5.22i to 3.16i,5.12i
	line from 2.96i,5.12i to 3.16i,5.12i
	line from 2.96i,5.12i to 3.06i,5.22i
	line from 2.96i,4.72i to 3.16i,4.72i
	line from 3.06i,4.62i to 3.16i,4.72i
	line from 2.96i,4.72i to 3.06i,4.62i
	ellipse with .c at 4.26i,3.42i width 0.60i height 0.40i "9"
	line from 4.66i,5.42i to 4.76i,5.42i
	line from 4.66i,5.12i to 4.76i,5.12i
	line from 4.47i,5.28i to 4.66i,5.12i
	line from 3.27i,5.28i to 3.46i,5.12i
	line from 3.46i,5.12i to 3.56i,5.12i
	line from 3.46i,5.42i to 3.56i,5.42i
	line from 3.56i,5.12i to 3.66i,5.22i
	line from 3.66i,5.32i to 3.66i,5.22i
	line from 3.56i,5.42i to 3.66i,5.32i
	line from 3.56i,5.12i to 3.66i,5.22i
	line from 3.66i,5.32i to 3.66i,5.22i
	line from 3.56i,5.42i to 3.66i,5.32i
	line from 3.56i,5.12i to 3.66i,5.22i
	line from 3.66i,5.32i to 3.66i,5.22i
	line from 3.56i,5.42i to 3.66i,5.32i
	line from 3.27i,4.28i to 3.46i,4.12i
	line from 3.46i,4.12i to 3.56i,4.12i
	line from 3.46i,4.42i to 3.56i,4.42i
	line from 3.56i,4.12i to 3.66i,4.22i
	line from 3.66i,4.32i to 3.66i,4.22i
	line from 3.56i,4.42i to 3.66i,4.32i
	line from 3.56i,4.12i to 3.66i,4.22i
	line from 3.66i,4.32i to 3.66i,4.22i
	line from 3.56i,4.42i to 3.66i,4.32i
	line from 3.56i,4.12i to 3.66i,4.22i
	line from 3.66i,4.32i to 3.66i,4.22i
	line from 3.56i,4.42i to 3.66i,4.32i
	line from 4.47i,4.28i to 4.66i,4.12i
	line from 4.66i,4.12i to 4.76i,4.12i
	line from 4.66i,4.42i to 4.76i,4.42i
	line from 4.76i,4.12i to 4.86i,4.22i
	line from 4.86i,4.32i to 4.86i,4.22i
	line from 4.76i,4.42i to 4.86i,4.32i
	line from 4.76i,4.12i to 4.86i,4.22i
	line from 4.86i,4.32i to 4.86i,4.22i
	line from 4.76i,4.42i to 4.86i,4.32i
	line from 4.76i,4.12i to 4.86i,4.22i
	line from 4.86i,4.32i to 4.86i,4.22i
	line from 4.76i,4.42i to 4.86i,4.32i
	line from 5.67i,5.28i to 5.86i,5.12i
	line from 5.86i,5.12i to 5.96i,5.12i
	line from 5.86i,5.42i to 5.96i,5.42i
	line from 5.96i,5.12i to 6.06i,5.22i
	line from 6.06i,5.32i to 6.06i,5.22i
	line from 5.96i,5.42i to 6.06i,5.32i
	line from 5.96i,5.12i to 6.06i,5.22i
	line from 6.06i,5.32i to 6.06i,5.22i
	line from 5.96i,5.42i to 6.06i,5.32i
	line from 5.96i,5.12i to 6.06i,5.22i
	line from 6.06i,5.32i to 6.06i,5.22i
	line from 5.96i,5.42i to 6.06i,5.32i
	line from 5.67i,4.28i to 5.86i,4.12i
	line from 5.86i,4.12i to 5.96i,4.12i
	line from 5.86i,4.42i to 5.96i,4.42i
	line from 5.96i,4.12i to 6.06i,4.22i
	line from 6.06i,4.32i to 6.06i,4.22i
	line from 5.96i,4.42i to 6.06i,4.32i
	line from 5.96i,4.12i to 6.06i,4.22i
	line from 6.06i,4.32i to 6.06i,4.22i
	line from 5.96i,4.42i to 6.06i,4.32i
	line from 5.96i,4.12i to 6.06i,4.22i
	line from 6.06i,4.32i to 6.06i,4.22i
	line from 5.96i,4.42i to 6.06i,4.32i
	line from 4.26i,4.22i to 4.26i,3.72i
	line from 4.16i,3.72i to 4.26i,3.62i
	line from 4.16i,3.72i to 4.36i,3.72i
	line from 4.26i,3.62i to 4.36i,3.72i
	line from 3.86i,5.42i to 3.96i,5.42i
	line from 3.76i,5.32i to 3.86i,5.42i
	line from 3.76i,5.32i to 3.76i,4.52i
	line from 3.66i,4.42i to 3.76i,4.52i
	line from 3.56i,4.42i to 3.66i,4.42i
	line from 5.06i,4.52i to 5.16i,4.42i
	line from 5.06i,4.52i to 5.06i,4.32i
	line from 5.06i,4.32i to 5.16i,4.42i
	line from 4.96i,4.42i to 5.06i,4.42i
	line from 4.86i,4.52i to 4.96i,4.42i
	line from 4.86i,4.52i to 4.86i,5.02i
	line from 4.76i,5.12i to 4.86i,5.02i
	line from 4.86i,4.32i to 4.96i,4.42i
	line from 3.66i,4.42i to 3.96i,4.42i
	line from 4.16i,3.92i to 4.26i,3.82i
	line from 3.16i,3.92i to 4.16i,3.92i
	line from 3.06i,4.02i to 3.16i,3.92i
	line from 4.26i,3.82i to 4.36i,3.92i
	line from 4.36i,3.92i to 5.36i,3.92i
	line from 5.36i,3.92i to 5.46i,4.02i
	line from 5.46i,4.22i to 5.46i,4.02i
	line from 3.06i,4.22i to 3.06i,4.02i
	line from 2.66i,5.42i to 2.76i,5.42i
	line from 2.56i,5.32i to 2.66i,5.42i
	line from 2.56i,5.32i to 2.56i,4.02i
	line from 2.56i,4.02i to 2.66i,3.92i
	line from 2.66i,3.92i to 3.16i,3.92i
	line from 5.96i,5.12i to 6.16i,4.92i
	line from 6.16i,4.92i to 6.16i,4.02i
	line from 6.06i,3.92i to 6.16i,4.02i
	line from 5.36i,3.92i to 6.06i,3.92i
	line from 3.76i,4.92i to 3.86i,4.82i
	line from 3.86i,4.82i to 3.86i,4.02i
	line from 3.86i,4.02i to 3.96i,3.92i
.PE
.DE
.CD
\fBFigure 2:\fR Meta-State Graph for Listing 1
\ 
.DE
.KE
.LP
.NH 2
.iH MIMD State Time Splitting Algorithm
.PP
In the base conversion algorithm, we made the assumption that each
MIMD state took exactly the same amount of time to execute.  However,
such an assumption is unrealistic:
.IP \(bu 2
If each instruction is treated as a separate MIMD state, then
reasonable size programs will generate unreasonably large automata.
This makes the analysis for meta-state conversion much slower and also
can result in an impractically large meta-state automaton. In
addition, some computers have instruction sets in which even the
execution time of different types of instruction varies widely.
.IP \(bu
If instead we simply treat each maximal basic block as a MIMD state
and ignore the differences in execution time between these blocks,
this can result in very poor processor utilization.  For example, if a
block that takes 5 clock cycles to execute is placed in the same
meta-state as one that takes 100 cycles, then the parallel machine may
spend up to 95% of its processor cycles simply waiting for the
transition to the next meta state.
.LP
In other words, the meta-state automaton embodies an \fIexecution time
schedule\fR for the code, and it is necessary that the execution time
of each block be taken into account if a good schedule is to be
produced.
.PP
There are many possible ways in which timing information could be
incorporated, but our overriding concern must be keeping the state
space manageable, and this greatly restricts the choice.  Clearly, the
smallest MIMD state automaton results from treating each maximal basic
block as a MIMD state; hence, this will be our initial assumption.  As
the conversion is being performed, we may be fortunate enough to have
all the MIMD states merged into each meta state happen to have the
same cost.  If the costs differ, but do not differ by a significant
enough amount, we can ignore the difference.
.PP
This leaves only the case of a meta state that contains MIMD states of
widely varying cost, for example, the 5 and 100 cycle MIMD states
mentioned above.  The solution we propose is a simple heuristic that
will break the 100 cycle MIMD state into an approximately 5 cycle MIMD
state which is unconditionally followed by the remaining portion of
the original 100 cycle state.  Since this change might also affect the
construction of other meta states that had incorporated the original
100 cycle MIMD state, the construction of the meta-state automaton is
restarted to ensure that the final meta-state automaton is consistent.
.PP
The following pseudocode gives the algorithm for performing MIMD state
splitting based on the variation in timing within a meta state.  It
would be invoked on each meta state as it is created.
.cB
flag
time_split_state(s)
set s;
{
 /* Determine if time imbalance between
    MIMD states within the meta state s
    is sufficient to time split the more
    expensive MIMD states to get better
    balance; this assumes that each MIMD
    state already has an execution time
    associated with it
 */
 flag didsplit;

 /* Ignore zero time components because
    you can't do anything about them
 */
 s = s - {e| e \(mo s, time(e) == 0};

 /* Get min and max MIMD state times */
 min = min_MIMD_state_time(s);
 max = max_MIMD_state_time(s);

 /* Is enough time wasted to be worth
    splitting?  Not if the difference
    between times is already at noise
    level (split_delta) or if the
    utilization is already sure to be
    greater than an acceptable
    percentage (split_percentage)
 */
 if ((min + split_delta) > max) {
  return(FALSE);
 }
 if (min > ((split_percent*max)/100)) {
  return(FALSE);
 }

 /* Splitting seems useful...  do it */
 didsplit = FALSE;
 while (s != \(es) {
  element e;

  e = [e| e \(mo s];
  s = s - {e};
  if (time(e) > min) {
   /* If possible, split this node into
      two nodes, the first with time
      \(~= min, the second with the
      remaining time...
   */
   \|...
   didsplit = TRUE;
  }
 }

 return(didsplit);
}
.cE
.PP
The splitting of a state is illustrated in the next two figures.
The relevant portion of the initial MIMD state graph is:
.KS
.BD
.PS 1.5i
M0:	ellipse with .c at 2.00i,3.60i width 0.60i height 0.40i "\(*a"
	ellipse with .c at 3.20i,3.60i width 0.60i height 0.40i "\(*b"
	line dashed from 3.10i,3.90i to 3.20i,3.80i
	line dashed from 3.20i,3.80i to 3.30i,3.90i
	line dashed from 3.10i,3.90i to 3.30i,3.90i
	line dashed from 2.00i,3.40i to 2.00i,2.90i
	line dashed from 2.00i,2.80i to 2.10i,2.90i
	line dashed from 1.90i,2.90i to 2.10i,2.90i
	line dashed from 1.90i,2.90i to 2.00i,2.80i
	line dashed from 3.20i,4.40i to 3.20i,3.90i
	line dashed from 3.10i,2.90i to 3.20i,2.80i
	line dashed from 3.10i,2.90i to 3.30i,2.90i
	line dashed from 3.20i,2.80i to 3.30i,2.90i
	line dashed from 3.20i,3.40i to 3.20i,2.90i
	 "t\s-2\d\(*a\u\s+2" at 2.30i,3.90i ljust
	 "t\s-2\d\(*b\u\s+2" at 3.40i,3.90i ljust
	line dashed from 3.20i,3.20i to 3.30i,3.10i
	line dashed from 3.30i,3.10i to 3.50i,3.10i
	line dashed from 3.10i,3.10i to 3.20i,3.20i
	line dashed from 2.90i,3.10i to 3.10i,3.10i
	line dashed from 2.00i,3.20i to 2.10i,3.10i
	line dashed from 2.10i,3.10i to 2.30i,3.10i
	line dashed from 1.90i,3.10i to 2.00i,3.20i
	line dashed from 1.70i,3.10i to 1.90i,3.10i
	line dashed from 3.20i,4.00i to 3.30i,4.10i
	line dashed from 3.30i,4.10i to 3.50i,4.10i
	line dashed from 3.10i,4.10i to 3.20i,4.00i
	line dashed from 2.90i,4.10i to 3.10i,4.10i
	line dashed from 2.80i,4.20i to 2.90i,4.10i
	line dashed from 3.50i,4.10i to 3.60i,4.20i
	line dashed from 3.50i,3.10i to 3.60i,3.00i
	line dashed from 1.60i,3.00i to 1.70i,3.10i
	line dashed from 2.30i,3.10i to 2.40i,3.00i
	line dashed from 2.80i,3.00i to 2.90i,3.10i
	line dashed from 1.90i,3.90i to 2.00i,3.80i
	line dashed from 1.90i,3.90i to 2.10i,3.90i
	line dashed from 2.00i,3.80i to 2.10i,3.90i
	line dashed from 2.00i,4.40i to 2.00i,3.90i
	line dashed from 2.00i,4.00i to 2.10i,4.10i
	line dashed from 2.10i,4.10i to 2.30i,4.10i
	line dashed from 1.90i,4.10i to 2.00i,4.00i
	line dashed from 1.70i,4.10i to 1.90i,4.10i
	line dashed from 1.60i,4.20i to 1.70i,4.10i
	line dashed from 2.30i,4.10i to 2.40i,4.20i
.PE
.DE
.CD
\fBFigure 3:\fR MIMD States Before Time Splitting
.DE
.KE
.LP
Suppose that meta-state conversion would combine states \(*a and \(*b
and that \(*b takes much longer to execute than \(*a, i.e.,
t\s-2\d\(*a\u\s+2<t\s-2\d\(*b\u\s+2.  The state splitting algorithm
would attempt to convert this portion of the state graph into:
.KS
.BD
.PS 1.5i
M0:	line from 3.50i,4.20i to 3.50i,3.70i
	line from 3.50i,3.60i to 3.60i,3.70i
	line from 3.40i,3.70i to 3.60i,3.70i
	line from 3.40i,3.70i to 3.50i,3.60i
	ellipse with .c at 3.50i,4.40i width 0.60i height 0.40i "\(*b'"
	ellipse with .c at 3.50i,3.40i width 0.60i height 0.40i "\(*b''"
	 "t\s-2\d\(*a\u\s+2" at 2.60i,4.70i ljust
	 "t\s-2\d\(*b\u\s+2\(emt\s-2\d\(*a\u\s+2" at 3.80i,3.70i ljust
	 "t\s-2\d\(*a\u\s+2" at 3.70i,4.70i ljust
	line dashed from 2.60i,4.90i to 2.70i,5.00i
	line dashed from 1.90i,5.00i to 2.00i,4.90i
	line dashed from 2.00i,4.90i to 2.20i,4.90i
	line dashed from 2.20i,4.90i to 2.30i,4.80i
	line dashed from 2.40i,4.90i to 2.60i,4.90i
	line dashed from 2.30i,4.80i to 2.40i,4.90i
	line dashed from 2.30i,5.20i to 2.30i,4.70i
	line dashed from 2.30i,4.60i to 2.40i,4.70i
	line dashed from 2.20i,4.70i to 2.40i,4.70i
	line dashed from 2.20i,4.70i to 2.30i,4.60i
	line dashed from 3.80i,4.90i to 3.90i,5.00i
	line dashed from 3.10i,5.00i to 3.20i,4.90i
	line dashed from 3.20i,4.90i to 3.40i,4.90i
	line dashed from 3.40i,4.90i to 3.50i,4.80i
	line dashed from 3.60i,4.90i to 3.80i,4.90i
	line dashed from 3.50i,4.80i to 3.60i,4.90i
	line dashed from 3.50i,5.20i to 3.50i,4.70i
	line dashed from 3.50i,4.60i to 3.60i,4.70i
	line dashed from 3.40i,4.70i to 3.60i,4.70i
	line dashed from 3.40i,4.70i to 3.50i,4.60i
	line dashed from 2.60i,3.90i to 2.70i,3.80i
	line dashed from 1.90i,3.80i to 2.00i,3.90i
	line dashed from 2.00i,3.90i to 2.20i,3.90i
	line dashed from 2.20i,3.90i to 2.30i,4.00i
	line dashed from 2.40i,3.90i to 2.60i,3.90i
	line dashed from 2.30i,4.00i to 2.40i,3.90i
	line dashed from 2.20i,3.70i to 2.30i,3.60i
	line dashed from 2.20i,3.70i to 2.40i,3.70i
	line dashed from 2.30i,3.60i to 2.40i,3.70i
	line dashed from 2.30i,4.20i to 2.30i,3.70i
	line dashed from 3.50i,3.20i to 3.50i,2.70i
	line dashed from 3.50i,2.60i to 3.60i,2.70i
	line dashed from 3.40i,2.70i to 3.60i,2.70i
	line dashed from 3.40i,2.70i to 3.50i,2.60i
	line dashed from 3.50i,3.00i to 3.60i,2.90i
	line dashed from 3.60i,2.90i to 3.80i,2.90i
	line dashed from 3.40i,2.90i to 3.50i,3.00i
	line dashed from 3.20i,2.90i to 3.40i,2.90i
	line dashed from 3.10i,2.80i to 3.20i,2.90i
	line dashed from 3.80i,2.90i to 3.90i,2.80i
	ellipse with .c at 2.30i,4.40i width 0.60i height 0.40i "\(*a"
.PE
.DE
.CD
\fBFigure 4:\fR MIMD States After Time Splitting
.DE
.KE
.LP
Thus, states \(*a and \(*b' would be merged\ \(em without any idle
time being introduced for either thread of execution.
.NH 2
.iH Meta State Compression Algorithm
.PP
Despite the reduction in state space possible using maximal basic
blocks and time splitting, the automata created can be very large.
Hence, it is useful to find a way to reduce the upper bound on the
number of meta states created.
.PP
Because MIMD nodes with zero or one exit arc can only increase the
state space linearly, the explosion in meta state space is related to
the occurrence of MIMD states that have two exit arcs.  Each such MIMD
state could contribute three meta states: the \f7TRUE\fR successor,
\f7FALSE\fR successor, and both successors.  However, if there are
many processes in any given MIMD state, it is easy to see that the
most probable case is that of both successors.  Further, the case of
both successors can always emulate either successor, since it has the
code for both.  Thus, a very dramatic reduction in meta state space
can be obtained by simply assuming that both successors are always
taken.
.cB
int
reach(start, s, t)
set start, s, t;
{
 /* Make entries for all meta states
    t| start\(->t
 */

 if (s == \(es) {
  /* All MIMD state transitions from
     within start have been considered,
     hence, t must be a meta state
  */
  make_meta_state_transition(start, t);
 } else {
  /* Select a MIMD state and process
     its transition(s), recursing to
     complete the meta state
  */
  element e, next, fnext;

  e = [e| e \(mo s];
  s = s - {e};
  next = next_MIMD_state(e);
  fnext = next_MIMD_state_if_false(e);

  /* Always take all possible paths... */
  if (next) {
   if (fnext) {
    reach(start, s, t \(cu next \(cu fnext);
   } else {
    reach(start, s, t \(cu next);
   }
  } else {
   reach(start, s, t);
  }
 }
}
.cE
.PP
Returning to our example code, the meta-state compression algorithm
results in a graph with only two meta-states, compared to eight for
the uncompressed graph:
.KS
.BD
.PS .8
M0:	line from 2.43i,2.85i to 2.62i,2.69i
	line from 2.62i,2.69i to 2.72i,2.69i
	line from 2.62i,2.99i to 2.72i,2.99i
	line from 2.72i,2.69i to 2.82i,2.79i
	line from 2.82i,2.89i to 2.82i,2.79i
	line from 2.72i,2.99i to 2.82i,2.89i
	line from 2.22i,3.79i to 2.22i,3.29i
	line from 2.72i,2.69i to 2.82i,2.79i
	line from 2.82i,2.89i to 2.82i,2.79i
	line from 2.72i,2.99i to 2.82i,2.89i
	line from 2.62i,3.09i to 2.62i,2.89i
	line from 2.52i,2.99i to 2.62i,3.09i
	line from 2.72i,2.69i to 2.82i,2.79i
	line from 2.82i,2.89i to 2.82i,2.79i
	line from 2.72i,2.99i to 2.82i,2.89i
	line from 2.52i,2.99i to 2.62i,2.89i
	line from 2.12i,3.29i to 2.32i,3.29i
	line from 2.22i,3.19i to 2.32i,3.29i
	line from 2.12i,3.29i to 2.22i,3.19i
	ellipse with .c at 2.22i,2.99i width 0.60i height 0.40i "2,6,9"
	ellipse with .c at 2.22i,3.99i width 0.60i height 0.40i "0" "2,6"
.PE
.DE
.CD
\fBFigure 5:\fR Compressed Meta-State Graph for Listing 1
.DE
.KE
.LP
Notice that meta-state transitions into compressed portions of the
graph are unconditional; i.e., there is no need to use a \f7globalor\fR
to determine what states are present.  The disadvantage is that the
average meta-state is wider, which implies that the SIMD implementation
will be less efficient.
.NH 2
.iH Barrier Synchronization Algorithm
.PP
While the above compression scheme produces very small automata, it
does increase overhead somewhat in that each meta state becomes much
more complex.  Hence, it is useful to seek yet another method to
reduce the state space\ \(em without adding to the complexity of each
meta state.  Careful use of barrier synchronization provides such a
mechanism.
.cB
set
barrier_sync(s)
set s;
{
 /* If s is a meta state that contains a
    MIMD state which is a barrier
    synchronization point, then the
    barrier should prevent any
    transitions past that MIMD state.
    Hence, unless all processors have
    reached the barrier (i.e., every MIMD
    state within s is a barrier state),
    simply remove barrier states from s
 */
 set waits;

 /* Construct the set of MIMD barrier
    wait states within s
 */
 waits = {e| e \(mo s, is_barrier_wait(e) == TRUE};

 /* Has everyone reached the barrier? */
 if (waits == s) {
  /* Yes; go into all barrier state */
  return(waits);
 } else {
  /* No; remove barriers from s */
  return(s - waits);
 }
}
.cE
.PP
For example, consider modifying the code framework of listing 1
to contain a barrier sync at the end of the \f7if\fR:
.KS
.BD
\f7if (\fIA\f7) {
    do { \fIB\f7 } while (\fIC\f7);
} else {
    do { \fID\f7 } while (\fIE\f7);
}
wait;  /* barrier sync. of all threads */
\fIF\fR
.DE
.CD
\fBListing 3:\fR Listing 1 + Barrier Synchronization
.DE
.KE
.LP
The barrier synchronization does not result in a runtime operation,
but rather constrains the asynchrony as defined by the above algorithm.
The result is a meta-state graph of the form:
.KF
.BD
.PS 3i
M0:	ellipse with .c at 3.20i,4.90i width 0.60i height 0.40i "0"
	ellipse with .c at 2.00i,3.90i width 0.60i height 0.40i "2"
	ellipse with .c at 3.20i,3.90i width 0.60i height 0.40i "2,6"
	line from 1.90i,4.20i to 2.00i,4.10i
	line from 1.90i,4.20i to 2.10i,4.20i
	line from 2.00i,4.10i to 2.10i,4.20i
	line from 3.10i,4.20i to 3.20i,4.10i
	line from 3.20i,4.10i to 3.30i,4.20i
	line from 3.10i,4.20i to 3.30i,4.20i
	line from 2.00i,4.30i to 2.10i,4.40i
	line from 3.50i,3.90i to 3.60i,3.80i
	line from 3.70i,3.90i to 3.80i,3.80i
	line from 3.80i,3.80i to 3.80i,3.70i
	line from 3.50i,3.90i to 3.60i,4.00i
	line from 3.60i,4.00i to 3.60i,3.80i
	line from 2.40i,4.00i to 2.40i,3.80i
	line from 2.30i,3.90i to 2.40i,4.00i
	line from 2.30i,3.90i to 2.40i,3.80i
	line from 3.70i,3.90i to 3.80i,3.80i
	line from 3.80i,3.80i to 3.80i,3.70i
	line from 4.80i,4.00i to 4.80i,3.80i
	line from 4.70i,3.90i to 4.80i,4.00i
	line from 4.70i,3.90i to 4.80i,3.80i
	line from 4.30i,4.40i to 4.40i,4.30i
	line from 4.30i,4.20i to 4.50i,4.20i
	line from 4.40i,4.10i to 4.50i,4.20i
	line from 4.30i,4.20i to 4.40i,4.10i
	ellipse with .c at 4.40i,3.90i width 0.60i height 0.40i "6"
	line from 3.20i,4.70i to 3.20i,4.20i
	line from 2.10i,4.40i to 3.10i,4.40i
	line from 3.10i,4.40i to 3.20i,4.50i
	line from 3.20i,4.50i to 3.30i,4.40i
	line from 3.30i,4.40i to 4.30i,4.40i
	line from 3.70i,3.90i to 3.80i,3.80i
	line from 3.80i,3.80i to 3.80i,3.70i
	ellipse with .c at 3.20i,2.90i width 0.60i height 0.40i "9"
	line from 3.60i,3.90i to 3.70i,3.90i
	line from 2.40i,3.90i to 2.50i,3.90i
	line from 2.60i,3.80i to 2.60i,3.70i
	line from 2.50i,3.90i to 2.60i,3.80i
	line from 2.60i,3.80i to 2.60i,3.70i
	line from 2.50i,3.90i to 2.60i,3.80i
	line from 2.60i,3.80i to 2.60i,3.70i
	line from 2.50i,3.90i to 2.60i,3.80i
	line from 4.80i,3.90i to 4.90i,3.90i
	line from 5.00i,3.80i to 5.00i,3.70i
	line from 4.90i,3.90i to 5.00i,3.80i
	line from 5.00i,3.80i to 5.00i,3.70i
	line from 4.90i,3.90i to 5.00i,3.80i
	line from 5.00i,3.80i to 5.00i,3.70i
	line from 4.90i,3.90i to 5.00i,3.80i
	line from 3.10i,3.20i to 3.20i,3.10i
	line from 3.10i,3.20i to 3.30i,3.20i
	line from 3.20i,3.10i to 3.30i,3.20i
	line from 3.10i,3.40i to 3.20i,3.30i
	line from 2.10i,3.40i to 3.10i,3.40i
	line from 3.20i,3.30i to 3.30i,3.40i
	line from 3.30i,3.40i to 4.30i,3.40i
	line from 4.40i,4.30i to 4.40i,4.20i
	line from 2.00i,4.30i to 2.00i,4.20i
	line from 2.50i,3.60i to 2.60i,3.70i
	line from 2.50i,3.60i to 2.60i,3.70i
	line from 2.50i,3.60i to 2.60i,3.70i
	line from 2.40i,3.60i to 2.50i,3.60i
	line from 2.21i,3.76i to 2.40i,3.60i
	line from 3.70i,3.60i to 3.80i,3.70i
	line from 3.70i,3.60i to 3.80i,3.70i
	line from 3.70i,3.60i to 3.80i,3.70i
	line from 3.60i,3.60i to 3.70i,3.60i
	line from 3.41i,3.76i to 3.60i,3.60i
	line from 4.80i,3.60i to 4.90i,3.60i
	line from 4.61i,3.76i to 4.80i,3.60i
	line from 4.90i,3.60i to 5.00i,3.70i
	line from 3.20i,3.70i to 3.20i,3.20i
	line from 2.50i,3.90i to 2.90i,3.90i
	line from 3.80i,3.80i to 3.90i,3.90i
	line from 3.90i,3.90i to 4.00i,3.90i
	line from 4.00i,4.00i to 4.10i,3.90i
	line from 4.00i,4.00i to 4.00i,3.80i
	line from 4.00i,3.80i to 4.10i,3.90i
	line from 2.00i,3.50i to 2.10i,3.40i
	line from 2.00i,3.50i to 2.00i,3.70i
	line from 4.30i,3.40i to 4.40i,3.50i
	line from 4.40i,3.50i to 4.40i,3.70i
.PE
.DE
.CD
\fBFigure 6:\fR Meta-State Graph for Listing 3
.DE
.KE
.LP
.NH
.iH SIMD Coding of the Meta-State Automaton
.PP
Given a MIMD program that has been converted into a meta-state graph,
it is not trivial to find an efficient coding of the meta-state
automaton for a SIMD architecture.  The meta-state graph does reduce
control flow to a single instruction stream, but that instruction
stream would appear to execute different types of instructions in
parallel\ \(em the meta-state graph employs a variation on VLIW
semantics.
.PP
There are two aspects of the graph that mirror VLIW constructions\**:
.FS
The meta-state graph is not suitable for execution on a traditional
VLIW because which processing elements execute which instructions is
determined statically for VLIW, but dynamically in the graph.  I.e.,
the graph would be appropriate for a VLIW in which each processing
element could select at runtime which instruction field it would
execute, rather than having each processing element statically
associated with a particular instruction field.
.FE
the apparently simultaneous execution of different types of
instructions and the use of multiway branches generated by merging
multiple (binary) branches.  Thus, we must efficiently implement these
VLIW-like execution structures on SIMD hardware.
.NH 2
.iH Common Subexpression Induction
.PP
Any meta state that merged two or more MIMD states effectively contains
multiple instruction sequences that are supposed to execute
simultaneously.  Given that it is impossible for a traditional SIMD
machine to simultaneously execute different types of instructions on
different processing elements, it would appear that these operations
will have to be serialized.  However, it is quite possible and
practical that any operations that would be performed by more than one
sequence can be executed in parallel by all processors.  Common
subexpression induction (CSI) [4] is an optimization technique that
identifies these operations and \*Qfactors\*U them out.
.PP
The CSI algorithm analyzes a segment of code containing operations
executed by any of multiple threads (enabled sets of SIMD PEs).  From
this analysis, it determines where threads can share the same code and
what cost is associated with inducing that sharing.  Finally, it
generates a code schedule that uses this sharing, where appropriate,
to achieve the minimum execution time.  Unfortunately, this implies
that the CSI algorithm is not simple.
.PP
The algorithm can be summarized as follows.  First, a guarded DAG is
constructed for the input, then this DAG is improved using
inter-thread CSE.  The improved DAG is then used to compute
information for pruning the search: earliest and latest, operation
classes, and theoretical lower bound on execution time.  Next, this
information is used to create a linear schedule (SIMD execution
sequence), which is improved using a cheap approximate search and then
used as the initial schedule for the permutation-in-range search that
is the core of the CSI optimization.
.NH 2
.iH Multiway Branch Encoding
.PP
At the end of each meta-state's execution, a particular type of
multiway branch must be executed to move the SIMD machine into the
correct next meta state.  Before discussing the encoding of these
multiway branches, it is useful to specify the precise semantics of
meta-state transitions, so that an optimal coding can be achieved.
The following defines the possible types of meta-state transitions.
.NH 3
.iH No Exit Arc
.PP
A meta state without an exit arc is a terminal node, i.e., it
represents the end of the program's execution.  Thus, it is implicitly
followed by a return to the operating system.  There is no difficulty
in generating code to implement this.
.NH 3
.iH Single Exit Arc
.PP
If there is a single exit arc from a meta state, the code for that
meta state is is followed by a \f7goto\fR (aka, \f7jump\fR) to the
code for the target meta state.  Again, it is simple to generate an
efficient coding.
.PP
Notice that all entries to compressed meta states fall into this
category.
.NH 3
.iH Multiple Exit Arcs
.PP
If there are multiple exit arcs from a meta state, then the aggregate
of the \*Qpc\*U values for each of the processing elements must be used
to determine the next state.  For example, when, at the end of
executing a meta state, some processing elements have \*Qpc\*U value 2
and others have \*Qpc\*U value 6, meta state {2,6} is the next state.
In order to efficiently collect this aggregate, each possible \*Qpc\*U
value is assigned a bit; thus, a \f7globalor\fR of the \*Qpc\*U values
from all processors determines the aggregate.
.NH 3
.iH Multiple Exit Arcs Involving Barriers
.PP
The treatment of multiple exit arcs must be slightly adjusted if some,
but not all, of the processing elements have reached a barrier at the
time a meta state's execution completes.  For example, in figure 6 the
transitions from meta states 2, {2,6}, and 6 into 2, {2,6}, and 6 would
not be sufficient if even one processing element had reached the
barrier (i.e., meta state 9).  Consequently, the processing elements
are allowed to set their \*Qpc\*U value to 9, but they are not
permitted to enter meta state 9 unless all \*Qpc\*U's are 9.
.PP
This is accomplished by a simple check to see if (\f7globalor\fR pc)
is contained within the set of all barrier states.  If it is, then the
state transition proceeds normally.  Otherwise, the next meta state is
determined by subtracting the set of all barrier states from the
result of the \f7globalor\fR.
.NH 3
.iH Restricted Dynamic Process Creation
.PP
Although the completely static nature of meta-state conversion makes
it impossible to efficiently support forking of new processes to
execute different programs, a minor encoding trick can be used to
implement a restricted form of dynamic process creation.  This
restricted type of \f7spawn\fR instruction looks just like a
conditional jump, except the semantics are that both paths must be
taken (i.e., the compressed meta state transition rule).  One exit is
taken by the original processes, the other by the newly created
processes.
.PP
Initially, processing elements that are not in use would be given a
\*Qpc\*U value indicating that they are not in any meta state.  When a
\f7spawn(\fIx\f7)\fR instruction is reached by N processing elements,
the original N processing elements do not change their pc values, but
N currently-disabled processing elements are selected and their pc
values are set to \fIx\fR.  No other changes are needed, provided that
the number of processes requested does not exceed the number of
processors available.
.PP
Note further that processors that complete their processes early can
be returned to the pool of free processors by simply executing a
\f7halt\fR instruction to set their pc value to indicate that they
are not in any meta state.
.NH 2
.iH Allocation of Bits for \*Qpc\*U Values
.PP
Although it is easy to implement each \*Qpc\*U value by assigning a
different bit to each MIMD state, this would result in impractically
long bit strings for large meta state automata.  Thus, although
conceptually a different bit is used to represent each MIMD state,
bits actually can be reused without changing the basic conversion
algorithm.  This bit allocation problem is similar to that of
allocating registers to values where the number of values can be
larger than the number of registers.  However, unlike register
allocation, there is no concept of \*Qspilling\*U a bit position; if
an allocation is not found using \fImaxbit\fR bits, we must increase
the number of bits.  Fortunately, it is unlikely that \fImaxbit\fR will
need to be large; in our preliminary experiments, it never was
necessary to use more than 8 bits.
.PP
Intuitively, there are just two rules that govern the reuse of a bit
to represent multiple MIMD states:
.IP 1. 3
No two MIMD states contained within the same meta state can be
allocated the same bit.  If this were violated, it would be impossible
to tell which code within that meta state should be executed by each
processing element.
.IP 2.
No two meta states which are successors of the same meta state (i.e.,
which are sibling meta states) can be allocated the same bit pattern.
If two siblings had the same bit pattern, the meta state automaton
would not be able to decide which of these sibling meta states to
execute.
.LP
The algorithm which we have implemented is given below. It applies
rule 1 directly, but uses a safe approximation to rule 2.  The
approximation is simply that no two distinct MIMD states that appear
in sibling meta states are allocated the same bit.
.cB
allocate_bits(maxbit)
int maxbit;
{
 /* Allocate bits to "pc" values, using at
    most maxbit bits.  Returns with ERROR
    if need more than maxbit bits.
 */
 set M, C, S;
 int pos, pat, n, c_n, bit[];

 M = {m| m \(mo meta states};

 /* Assign start state bit 0 */
 m\d0\u = (start meta state \(mo M);
 bit[m\d0\u] = 2\s-2\u0\d\s+2;

 for (m| m \(mo (M - m\d0\u)) {
  /* Set of all next meta states of m */
  C = {c| c \(mo M, m\(->c };
  S = {s| s \(mo C, bits of s have been allocated };
  C = C - S;

  /* OR patterns of all members of S */
  pat = OR(p| p is a pattern of some s \(mo S); 
  n = # of distinct MIMD states in S;
  if (# of 1 bits in pat \(!= n) {
   /* Some MIMD states have same bit pattern */
   return(ERROR);
  }

  if (C != \(es)) {
   /* There are some MIMD states whose
      bit patterns need to be allocated
   */
   for (c| c \(mo C) {
    if (c has more than 1 MIMD state) {
     bit[c] = OR(p| p is pattern of MIMD state\(moc);
     c_n = # of MIMD states in c;
     if (# of 1 bits in bit[c] \(!= c_n) {
      return(ERROR);
     }
     pat = OR(pat, bit[c]);
    } else {
     /* If only 1 MIMD state, assign pattern */
     if (# of 0 bits in pat == 0) {
      /* No free bit to allocate */
      return(ERROR);
     }
     pos = position of first 0 bit in pat;
     bit[c] = 2\s-2\upos\d\s+2;
    }
   }
  }
 }

 return(NO_ERROR);
}
.cE
.NH
.iH Implementation
.PP
The current prototype meta-state converter does not directly generate
executable SIMD code from a MIMD-oriented language.  Instead, it
simply outputs a set of meta-state definitions.  Each of these meta
states must then be common subexpression inducted and the meta-state
transitions (multiway branches) must be encoded using hash functions.
However, these last two steps are implemented by two software tools
developed earlier:
.IP \(bu 2
A common subexpression inductor, described in [4].
.IP \(bu
A hash function generator, described in [5].
.LP
Thus, in this paper we will confine the discussion to the
implementation of the prototype meta-state converter.  The meta-state
converter was written in C using PCCTS [10] and is actually a
modified version of the \f7mimdc\fR compiler described in [3].
.NH 2
.iH The Input Language
.PP
The language accepted by the meta-state converter is a parallel
dialect of C called MIMDC.  It supports most of the basic C
constructs.  Data values can be either \f7int\fR or \f7float\fR, and
variables can be declared as \f7mono\fR (shared) or \f7poly\fR
(private) [11].
.PP
There are two kinds of shared memory reference supported.  The
\f7mono\fR variables are replicated in each processor's local memory
so that loads execute quickly, but stores involve a broadcast to
update all copies.  It is also possible to directly access
\f7poly\fR values from other processors using \*Qparallel
subscripting\*U:
.LP
.BD
\f7x[||i] = y[||j] + z;\fR
.DE
.LP
would use the values of \f7i\fR, \f7j\fR, and \f7z\fR on this processor
to fetch the value of \f7y\fR from processor \f7j\fR, add \f7z\fR, and
store the result into the \f7x\fR on processor \f7i\fR.  In addition
to allowing use of shared memory for synchronization, MIMDC supports
barrier synchronization [6] using a \f7wait\fR statement.
.NH 2
.iH The Conversion Process
.PP
A brief outline of the prototype implementation is:
.IP 1. 3
As the PCCTS-generated parser reads the source code, a traditional
control-flow graph whose nodes are expression trees is built.  This
control-flow graph is constructed in a \*Qnormalized\*U form that
ensures, for example, that loops are all of the type that execute the
body one or more times, rather than zero or more (e.g., by replicating
some code and inserting an additional \f7if\fR statement).
.IP 2.
The control-flow graph is straightened and empty nodes are removed.
This maximizes the size of the nodes.
.IP 3.
The meta-state conversion algorithm is applied.  Except for the
handling of function calls, the prototype implements the full
algorithm.
.IP 4.
The resulting meta-state graph is straightened and output.
.LP
The current prototype implementation does not perform the final
encoding of the meta-state automaton.  Hence, a CSI tool [4] and a
tool for finding hash functions [5] are applied by hand to
produce the final SIMD code in MPL.
.NH 2
.iH An Example
.PP
To illustrate how the prototype meta-state converter works, consider
the MIMDC program presented in listing 4.  This example has the same
control structure given in listing 1, but is a complete program, so
that the actual code generated can be given.
.KS
.BD
\f7main()
{
    poly int x;

    if (x) {
        do { x = 1; } while (x);
    } else {
        do { x = 2; } while (x);
    }

    return(x);
}\fR
.DE
.CD
\fBListing 4:\fR Example MIMDC Program
.DE
.KE
.PP
Without compression or time cracking, the resulting meta-state SIMD
automaton, written in MPL [8] for the MasPar MP-1 [1], is given in
listing 5 (note that the algorithm in section 3.3 was not applied).
The code within each meta state is simple SIMD stack code using MPL
macros for each operation.  The only surprising stack operation is
\f7JumpF(\fIx\f7,\ \fIy\f7)\fR, which simply sets each processing
element's \f7pc\fR equal to 2\s-2\u\fIx\fR\d\s+2 if the top-of-stack
value is \*QFALSE\*U or to 2\s-2\u\fIy\fR\d\s+2 if it is \*QTRUE.\*U
The \f7apc\fR is simply the aggregate obtained by oring the values of
all the individual \f7pc\fRs; the switch at the end of each meta state
simply employs a customized hash function to ensure that the multiway
branch is implemented efficiently.  For example, at the end of meta
state 0 (i.e., \f7ms_0\fR), instead of a \f7switch\fR on \f7apc\fR
with cases for \f7BIT(2)|BIT(6)\fR, \f7BIT(6)\fR, and
\f7BIT(2)\fR, a hash function is applied to make the case values
contiguous so that the MPL compiler will use a jump table to implement
the \f7switch\fR.
.NH
.iH Conclusions
.PP
Although meta-state conversion is a complex and slow process, it does
provide a mechanical way to transform control-parallel (MIMD) programs
into pure SIMD code.  Further, the execution of the meta-state program
can be very efficient.  In particular, fine-grain MIMD code is
generally inefficient on most MIMD machines due to the cost of runtime
synchronization, but synchronization is implicit in the meta-state
converted SIMD code, and hence has no runtime cost.
.PP
While the prototype implementation demonstrates the feasibility and
correctness of the meta-state conversion algorithm, it does not yet
automate the process of generating the final SIMD code.  Future work
will integrate the code generation process and will benchmark
performance on \*Qreal\*U programs.
.KS
.SH
References
.IP [1] 4
T. Blank,
\*QThe MasPar MP-1 Architecture,\*U
35th IEEE Computer Society International Conference (COMPCON),
February 1990, pp. 20-24.
.IP [2]
J. Cocke and J.T. Schwartz,
\fIProgramming Languages and Their Compilers,\fR
Courant Institute of Mathematical Sciences, New York University,
April 1970.
.IP [3]
H.G. Dietz and W.E. Cohen,
\*QA Control-Parallel Programming Model Implemented On SIMD
Hardware,\*U in Proceedings of the \fIFifth Workshop on
Programming Languages and Compilers for Parallel Computing\fR,
August 1992.
.KE
.IP [4]
H.G. Dietz,
\*QCommon Subexpression Induction,\*U
Proceedings of the
\fI1992 International Conference on Parallel Processing,\fP
Saint Charles, Illinois, August 1992, vol. II, pp. 174-182.
.IP [5]
H.G. Dietz,
\*QCoding Multiway Branches Using Customized Hash Functions,\*U
Technical Report TR-EE 92-31,
School of Electrical Engineering, Purdue University, July 1992.
.IP [6]
H.G. Dietz, M.T. O'Keefe, and A. Zaafrani,
\*QAn Introduction to Static Scheduling for MIMD Architectures,\*U
Advances in Languages and Compilers for Parallel Processing,
edited by A. Nicolau, D. Gelernter, T. Gross, and D. Padua, The MIT
Press, Cambridge, Massachusetts, 1991, pp. 425-444.
.IP [7]
M. S. Littman and C. D. Metcalf,
\fIAn Exploration of Asynchronous Data-Parallelism\fR,
Technical Report, Yale University, July 1990.
.IP [8]
MasPar Computer Corporation,
\fIMasPar Programming Language (ANSI C compatible MPL) Reference
Manual, Software Version 2.2\fR, Document Number 9302-0001, Sunnyvale,
California, November 1991.
.IP [9]
M. Nilsson and H. Tanaka,
\*QMIMD Execution by SIMD Computers,\*U
Journal of Information Processing,
Information Processing Society of Japan,
vol. 13, no. 1, 1990, pp. 58-61.
.IP [10]
T.J. Parr, H.G. Dietz, and W.E. Cohen,
\*QPCCTS Reference Manual (version 1.00),\*U
\fIACM SIGPLAN Notices\fR,
Feb. 1992, pp. 88-165.
.IP [11]
M.J. Phillip,
\*QUnification of Synchronous and Asynchronous Models
for Parallel Programming Languages\*U
Master's Thesis,
School of Electrical Engineering,
Purdue University, West Lafayette, Indiana,
June 1989.
.IP [12]
P.A. Wilsey, D.A. Hensgen, C.E. Slusher, N.B. Abu-Ghazaleh, and D.Y.
Hollinden,
\*QExploiting SIMD Computers for Mutant Program Execution,\*U
Technical Report No. TR 133-11-91,
Department of Electrical and Computer Engineering,
University of Cincinnati, Cincinnati, Ohio,
November 1991.
.bp
.cB
ms_0:
  if (pc & BIT(0)) {
    Push(0) LdL JumpF(6,2)
  }
  apc = globalor(pc);
  switch (((-apc) >> 5) & 3) {
  case 1: goto ms_2_6;
  case 2: goto ms_6;
  case 3: goto ms_2;
  }

ms_2:
  if (pc & BIT(2)) {
    Push(1) Push(0) LdL Push(12) StL
    Pop(2) Push(4) LdL JumpF(9,2)
  }
  apc = globalor(pc);
  switch (((-apc) >> 8) & 3) {
  case 1: goto ms_2_9;
  case 2: goto ms_9;
  case 3: goto ms_2;
  }

ms_9:
  if (pc & BIT(9)) {
    Push(4) LdL Ret(3)
  }
  /* no next meta state */
  exit(0);

ms_2_9:
  if (pc & BIT(2)) {
    Push(1) Push(0) LdL
    Push(12) StL Pop(2)
  }
  if (pc & (BIT(2) | BIT(9))) {
    Push(4) LdL
  }
  if (pc & BIT(2)) JumpF(9,2)
  if (pc & BIT(9)) Ret(3)
  apc = globalor(pc);
  switch (((-apc) >> 8) & 3) {
  case 1: goto ms_2_9;
  case 2: goto ms_9;
  case 3: goto ms_2;
  }

ms_6:
  if (pc & BIT(6)) {
    Push(2) Push(0) LdL Push(12) StL
    Pop(2) Push(4) LdL JumpF(9,6)
  }
  apc = globalor(pc);
  switch (((-apc) >> 8) & 3) {
  case 1: goto ms_6_9;
  case 2: goto ms_9;
  case 3: goto ms_6;
  }
.cE
.KS
.cB
ms_6_9:
  if (pc & BIT(6)) {
    Push(2) Push(0) LdL
    Push(12) StL Pop(2)
  }
  if (pc & (BIT(6) | BIT(9))) {
    Push(4) LdL
  }
  if (pc & BIT(6)) JumpF(9,6)
  if (pc & BIT(9)) Ret(3)
  apc = globalor(pc);
  switch (((-apc) >> 8) & 3) {
  case 1: goto ms_6_9;
  case 2: goto ms_9;
  case 3: goto ms_6;
  }

ms_2_6:
  if (pc & BIT(2)) Push(1)
  if (pc & BIT(6)) Push(2)
  if (pc & (BIT(2) | BIT(6))) {
    Push(0) LdL Push(12) StL
    Pop(2) Push(4) LdL
  }
  if (pc & BIT(2)) JumpF(9,2)
  if (pc & BIT(6)) JumpF(9,6)
  apc = globalor(pc);
  switch (((apc >> 6) ^ apc) & 15) {
  case 5: goto ms_2_6;
  case 8: goto ms_9;
  case 9: goto ms_6_9;
  case 12: goto ms_2_9;
  case 13: goto ms_2_6_9;
  }

ms_2_6_9:
  if (pc & BIT(2)) Push(1)
  if (pc & BIT(6)) Push(2)
  if (pc & (BIT(2) | BIT(6))) {
    Push(0) LdL Push(12) StL Pop(2)
  }
  if (pc & (BIT(2) | BIT(6) | BIT(9))) {
    Push(4) LdL
  }
  if (pc & BIT(2)) JumpF(9,2)
  if (pc & BIT(6)) JumpF(9,6)
  if (pc & BIT(9)) Ret(3)
  apc = globalor(pc);
  switch (((apc >> 6) ^ apc) & 15) {
  case 5: goto ms_2_6;
  case 8: goto ms_9;
  case 9: goto ms_6_9;
  case 12: goto ms_2_9;
  case 13: goto ms_2_6_9;
  }
.cE
.KE
.bp
.LP
.CD
\fBListing 5:\fR Meta-State Converted Example
.DE
.bp
.LP
.PX
