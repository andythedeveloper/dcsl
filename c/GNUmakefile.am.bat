ACLOCAL_AMFLAGS = -I config/m4

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	$(linux_CFLAGS)

# link every app against libcan, it's static so it wouldn't hurt
LDADD = \
	libcan.la

noinst_HEADERS = \
	canframelen.h \
	lib.h \
	libj1939.h \
	terminal.h \
	include/linux/can.h \
	include/linux/can/bcm.h \
	include/linux/can/error.h \
	include/linux/can/gw.h \
	include/linux/can/isotp.h \
	include/linux/can/j1939.h \
	include/linux/can/netlink.h \
	include/linux/can/raw.h \
	include/linux/can/vxcan.h \
	include/linux/errqueue.h \
	include/linux/net_tstamp.h \
	include/linux/netlink.h \ 
	/usr/local/include/wiringPi.h \
	/usr/include/wiringPi.h \
	/usr/local/include/wiringSerial.h \
	/usr/include/wiringSerial.h

noinst_LTLIBRARIES = \
	libcan.la \
	libj1939.la

libcan_la_SOURCES = \
	lib.c \
	canframelen.c

libj1939_la_SOURCES = \
	libj1939.c

bin_PROGRAMS = \
	asc2log \
	bcmserver \
	can-calc-bit-timing \
	canbusload \
	candump \
	canfdtest \
	cangen \
	cangw \
	canlogserver \
	canplayer \
	cansend \
	cansequence \
	cansniffer \
	isotpdump \
	isotpperf \
	isotprecv \
	isotpsend \
	isotpserver \
	isotpsniffer \
	isotptun \
	j1939acd \
	j1939cat \
	j1939spy \
	j1939sr \
	log2asc \
	log2long \
	slcan_attach \
	slcand \
	slcanpty \
	testj1939

j1939acd_LDADD = libj1939.la
j1939cat_LDADD = libj1939.la
j1939spy_LDADD = libj1939.la
j1939sr_LDADD = libj1939.la
testj1939_LDADD = libj1939.la

EXTRA_DIST = \
	.travis.yml \
	Android.mk \
	README.md \
	autogen.sh \
	can-j1939-kickstart.md \
	can-j1939.md

MAINTAINERCLEANFILES = \
	configure \
	GNUmakefile.in \
	aclocal.m4 \
	config/autoconf/compile \
	config/autoconf/config.guess \
	config/autoconf/config.sub \
	config/autoconf/depcomp \
	config/autoconf/install-sh \
	config/autoconf/ltmain.sh \
	config/autoconf/mdate-sh \
	config/autoconf/missing \
	config/autoconf/texinfo.tex \
	config/m4/libtool.m4 \
	config/m4/ltoptions.m4 \
	config/m4/ltsugar.m4 \
	config/m4/ltversion.m4 \
	config/m4/lt~obsolete.m4 \
	$(DIST_ARCHIVES)

maintainer-clean-local:
	-chmod -R a+rw $(distdir)
	-rm -fr $(distdir)
