# $Id$
#
# Makefile for SQLMailer
#
PROGRAM=sqlmailer
SRCDIR=src

INSTALL_PROGRAM=install -c -s -o root -g bin -m 755
INSTALL_DATA_DIR=install -d -o root -g bin -m 755
INSTALL_DATA=install -c -o root -g bin -m 444
INSTALL_SAMPLE=install -c -o root -g wheel -m 644
INSTALL_SCRIPT=install -c -o root -g bin -m 555
INSTALL_MAN=install -c -o root -g bin -m 444

all:
	cd $(SRCDIR) && $(MAKE)

clean:
	rm -f $(PROGRAM)
	cd $(SRCDIR) && $(MAKE) clean

$(PROGRAM):
	$(MAKE) all

install: $(PROGRAM)
	$(INSTALL_PROGRAM) sqlmailer /usr/local/bin
	$(INSTALL_MAN) sqlmailer.1 /usr/local/man/man1
	if [ ! -f /etc/sqlmailer.conf ]; then \
		$(INSTALL_SAMPLE) sqlmailer.conf /etc ; \
	else \
		echo "sample file /etc/sqlmailer.conf exists!" ; \
	fi
