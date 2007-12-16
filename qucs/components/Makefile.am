## Process this file with automake to produce Makefile.in
# 
# qucs/components/Makefile.am
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

noinst_LIBRARIES = libcomponents.a

MOCHEADERS = propertytext.h componentproperty.h
# componentdialog.h spicedialog.h optimizedialog.h
MOCFILES = $(MOCHEADERS:.h=.moc.cpp)

libcomponents_a_SOURCES = component.cpp resistor.cpp componentproperty.cpp propertytext.cpp
# 					am_modulator.cpp ampere_ac.cpp ampere_dc.cpp ampere_noise.cpp amplifier.cpp\
# 					attenuator.cpp biast.cpp bondwire.cpp cccs.cpp ccvs.cpp circulator.cpp \
# 					coaxialline.cpp coplanar.cpp coupler.cpp cpwgap.cpp cpwopen.cpp \
# 					cpwshort.cpp cpwstep.cpp dcblock.cpp dcfeed.cpp d_flipflop.cpp \
# 					digi_source.cpp ground.cpp gyrator.cpp inductor.cpp iprobe.cpp \
# 					ipulse.cpp irect.cpp isolator.cpp jk_flipflop.cpp mscorner.cpp \
# 					mscoupled.cpp msgap.cpp msline.cpp msmbend.cpp msopen.cpp msstep.cpp \
# 					msvia.cpp mutual2.cpp mutual.cpp noise_ii.cpp noise_iv.cpp noise_vv.cpp \
# 					opamp.cpp phaseshifter.cpp pm_modulator.cpp relais.cpp rs_flipflop.cpp \
# 					source_ac.cpp substrate.cpp symtrafo.cpp tline_4port.cpp tline.cpp \
# 					transformer.cpp twistedpair.cpp vccs.cpp vcvs.cpp volt_ac.cpp volt_dc.cpp \
# 					volt_noise.cpp vprobe.cpp vpulse.cpp vrect.cpp

nodist_libcomponents_a_SOURCES = $(MOCFILES)

noinst_HEADERS = $(MOCHEADERS) component.h multisymbolcomponent.h components.h resistor.h
# 						am_modulator.h ampere_ac.h ampere_dc.h ampere_noise.h amplifier.h \
# 						attenuator.h biast.h bondwire.h cccs.h ccvs.h circulator.h \
# 						coaxialline.h coplanar.h coupler.h cpwgap.h cpwopen.h cpwshort.h \
# 						cpwstep.h dcblock.h dcfeed.h d_flipflop.h digi_source.h ground.h \
# 						gyrator.h inductor.h iprobe.h ipulse.h irect.h isolator.h \
# 						jk_flipflop.h mscorner.h mscoupled.h msgap.h msline.h msmbend.h \
# 						msopen.h msstep.h msvia.h mutual2.h mutual.h noise_ii.h noise_iv.h \
# 						noise_vv.h opamp.h phaseshifter.h pm_modulator.h relais.h \
# 						rs_flipflop.h source_ac.h substrate.h symtrafo.h tline_4port.h \
# 						tline.h transformer.h twistedpair.h vccs.h vcvs.h volt_ac.h \
# 						volt_dc.h volt_noise.h vprobe.h vpulse.h vrect.h

INCLUDES = $(X11_INCLUDES) $(QT_INCLUDES) -I$(top_srcdir)/qucs -I$(top_srcdir)

%.moc.cpp: %.h
	$(MOC) -o $@ $<

CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in *.moc.cpp
DISTCLEANFILES = *.moc.cpp
