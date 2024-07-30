FAWNCalc

This is a work in progress - sort of a fork of a fork.

Once, I took advantage of the Cluster Design Rules[1] to build my own version, using an enhanced list of X86 and specific network and interconnect hardware specific to the project at hand.

With the advent of the Beagle Board[3], Raspberry Pi[4], and the notion of "Brambles"[5] accompanying the introduction of commercial hardware like the ZT Systems R1801e 'Data Center Server'[6], I felt compelled to get my head around some the feasibility of large-scale implementations of ARM devices in massively parallel and high performance compute scenarios.

This tool should be looked at as a method by which to model ARM clusters of various density, taking advantage of as many connection, storage, interconnect, power consumption and physical footprint datapoints as can be determined from available specifications.

Where CDR and the previous fork differ from this tool specifically is in the specific aim of this tool to take the DIY and COTS nature of these emerging technologies into account, with Bluetooth, WiFi Serial (XBee) and other avenues of communication included in the assessments.

Where CDR is Macro, FAWNCalc is Micro.


References:

 [1] http://aggregate.org/CDR/
 [2] http://www.ztsystems.com/Default.aspx?tabid=1483
 [3] http://antipastohw.blogspot.com/2010/09/how-to-make-beagleboard-elastic-r.html
 [4] http://www.raspberrypi.org/
 [5] http://www.raspberrypi.org/forum/projects-and-collaboration-general/cluster-bramble-design-discussion-advanced
 [6] http://www.ztsystems.com/Default.aspx?tabid=1483http://www.ztsystems.com/Default.aspx?tabid=1483

TODO:

* Strip database of X86 stuff
* Add wireless/mesh interconnect specs
* Add ARM SoM specs
* Add ARM Dev Board specs
* Add ARM COTS Boxen
* Add CF, SSD, and Flash specs
* Collect benchmarks


Installation:

	To compile:

	./configure
	make

By default the cgi is put in /var/www/cgi-bin and temporary graph
files are put in /var/www/html/TMP.  If these are not good locations,
they can be changed by using the --prefix option to configure.  For
example, if your web pages are in $HOME/public_html and you want to
run the BDR out of your home directory, then type:

	./configure --prefix=$HOME/public_html
	make
	make install
