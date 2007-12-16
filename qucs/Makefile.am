## Process this file with automake to produce Makefile.in
#
# qucs/Makefile.am
#
# Automake input file.
#
# Copyright (C) 2004, 2005, 2006 Stefan Jahn <stefan@lkcc.org>
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this package; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
# Boston, MA 02110-1301, USA.
#

SUBDIRS = dialogs bitmaps diagrams paintings
# components


if COND_WIN32
bin_SCRIPTS = qucsdigi.bat
else
bin_SCRIPTS = qucsdigi
endif

bin_PROGRAMS = qucs

MOCHEADERS = schematicscene.h schematicview.h mainwindowbase.h \
			 qucsmainwindow.h componentssidebar.h svgitem.h svgtest.h \
			 propertyitem.h propertygroup.h component.h

MOCFILES = $(MOCHEADERS:.h=.moc.cpp)

qucs_SOURCES = 	wire.cpp componentssidebar.cpp  item.cpp \
				schematicscene.cpp  sidebarmodel.cpp \
                main.cpp mainwindowbase.cpp qucsmainwindow.cpp qucsview.cpp \
				schematicview.cpp  undocommands.cpp fileformathandler.cpp \
				xmlutilities.cpp  \
				svgitem.cpp svgtest.cpp property.cpp propertyitem.cpp \
				propertygroup.cpp component.cpp port.cpp library.cpp
# qucsprimaryformat.cpp xmlformat.cpp

nodist_qucs_SOURCES = $(MOCFILES)

INCLUDES = $(X11_INCLUDES) $(QT_INCLUDES) $(QT_EXTRA_INCLUDES) -I$(top_srcdir)
qucs_LDFLAGS = $(X11_LDFLAGS) $(QT_LDFLAGS)
qucs_LDADD = ../qucs-tools/libqucstools.a dialogs/libdialogs.a \
  $(X11_LIBS) $(QT_LIBS) diagrams/libdiagrams.a paintings/libpaintings.a
# components/libcomponents.a

noinst_HEADERS = $(MOCHEADERS) wire.h wireline.h item.h undocommands.h qucsview.h \
                               sidebarmodel.h fileformathandler.h  \
                               xmlutilities.h svgitem.h property.h port.h library.h
# qucsprimaryformat.h xmlformat.h

# rules for translations
TRANSLATIONS = qucs_de.ts qucs_pl.ts qucs_fr.ts qucs_it.ts qucs_ro.ts \
  qucs_pt.ts qucs_es.ts qucs_jp.ts qucs_hu.ts qucs_he.ts qucs_sv.ts   \
  qucs_tr.ts qucs_uk.ts qucs_ru.ts
GENTRANSLATIONS = qtgeneric_untranslated.ts qtgeneric_de.ts qtgeneric_fr.ts \
  qtgeneric_es.ts qtgeneric_ro.ts qtgeneric_jp.ts qtgeneric_hu.ts \
  qtgeneric_it.ts qtgeneric_he.ts qtgeneric_sv.ts qtgeneric_pl.ts \
  qtgeneric_tr.ts qtgeneric_uk.ts qtgeneric_ru.ts
TRANSFILES = $(TRANSLATIONS:.ts=.qm)

qucs_%.ts:
# $(qucs_SOURCES)
# 	@echo "Updating translations...";
# 	@list='$(TRANSLATIONS)'; for file in $$list; do \
# 	  $(LUPDATE) `$(PATHXFORM) $(srcdir)/*.cpp $(srcdir)/paintings/*.cpp \
# 	  $(srcdir)/diagrams/*.cpp $(srcdir)/components/*.cpp \
# 	  $(srcdir)/dialogs/*.cpp  $(top_srcdir)/qucs-help/*.cpp \
# 	  $(top_srcdir)/qucs-edit/*.cpp $(top_srcdir)/qucs-filter/*.cpp \
# 	  $(top_srcdir)/qucs-transcalc/*.cpp $(top_srcdir)/qucs-lib/*.cpp \
# 	  $(top_srcdir)/qucs-attenuator/*.cpp` \
# 	  -ts $(srcdir)/$$file; \
# 	done

%.qm: %.ts
	@qucsfile=$<; \
	genfile=`echo $< | sed -e 's/qucs_\(..\)\.ts/qtgeneric_\1\.ts/'`; \
	ts_file=`echo $< | sed -e 's/\(.*\)qucs_\(..\)\.ts/tmp_\2\.ts/'`; \
	qm_file=`echo $< | sed -e 's/\(.*\)qucs_\(..\)\.ts/tmp_\2\.qm/'`; \
	if test -f $$genfile; then \
	  echo "Incorporating '$$genfile'..."; \
	  grep -v "</TS>" $$qucsfile > $$ts_file; \
	  grep -v "<\!DOCTYPE TS><TS>" $$genfile >> $$ts_file; \
	else \
	  cp -f $$qucsfile $$ts_file; \
	fi; \
	echo "$(LRELEASE) $$qucsfile"; \
	$(LRELEASE) $$ts_file; test -f $$qm_file && mv -f $$qm_file $@; \
	rm -f $$ts_file

# installation of translations
langdatadir = ${prefix}/share/qucs/lang
langdata_DATA = $(TRANSFILES)

EXTRA_DIST = $(TRANSLATIONS) $(GENTRANSLATIONS) README.translations \
	qucsdigi qucsdigi.bat

all-local: $(TRANSLATIONS) $(TRANSFILES)

%.moc.cpp: %.h
	$(MOC) -o $@ $<

CLEANFILES = *~ *.rej *.orig
MAINTAINERCLEANFILES = Makefile.in *.moc.cpp *.qm
DISTCLEANFILES = *.moc.cpp *.qm
