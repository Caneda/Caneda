## Process this file with automake to produce Makefile.in
#
# qucs/xmlutilities/Makefile.am
#
# Automake input file.
#
# Copyright (C) 2004 Stefan Jahn <stefan@lkcc.org>
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

noinst_LIBRARIES = libxmlutilities.a

libxmlutilities_a_SOURCES = QRelaxNGvalidator.cpp  QXmlStreamReaderExt.cpp  QXsltTransformer.cpp  validators.cpp \
						xmlutilities.cpp

noinst_HEADERS = QRelaxNGvalidator.h  QXmlStreamReaderExt.h  QXsltTransformer.h  validators.h xmlutilities.h

INCLUDES = $(X11_INCLUDES) $(QT_INCLUDES) $(LIBXML2_INCLUDES) -I$(top_srcdir)/qucs

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in *.moc.cpp
DISTCLEANFILES = *.moc.cpp
