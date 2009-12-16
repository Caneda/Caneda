/***************************************************************************
 * This file is part of the KDE libraries.                                 *
 * Copyright 2003,2007 by Oswald Buddenhagen <ossi@kde.org>                *
 * Copyright 2008 by e_k <e_k@users.sourceforge.net>                       *
 * Rewritten for QT4 by e_k <e_k@users.sourceforge.net>                    *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/


#ifndef kpty_p_h
#define kpty_p_h

#include "kpty.h"

#include <QtCore/QByteArray>

struct KPtyPrivate {
    Q_DECLARE_PUBLIC(KPty)

    KPtyPrivate();
    bool chownpty(bool grant);

    int masterFd;
    int slaveFd;

    QByteArray ttyName;

    KPty *q_ptr;
};

#endif
