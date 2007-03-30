## Process this file with automake to produce Makefile.in
# 
# qucs-tools/Makefile.am
#
# Automake input file.
#
# Copyright (C) 2006 Stefan Jahn <stefan@lkcc.org>
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

noinst_LIBRARIES = libqucstools.a

MOCHEADERS = propertygrid.h
MOCFILES = $(MOCHEADERS:.h=.moc.cpp)

libqucstools_a_SOURCES = units.cpp propertygrid.cpp helpdialog.cpp

nodist_libqucstools_a_SOURCES = $(MOCFILES)

noinst_HEADERS = $(MOCHEADERS) global.h units.h helpdialog.h

INCLUDES = $(X11_INCLUDES) $(QT_INCLUDES) -I$(top_srcdir)/qucs

%.moc.cpp: %.h
	$(MOC) -o $@ $<

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in *.moc.cpp
DISTCLEANFILES = *.moc.cpp
