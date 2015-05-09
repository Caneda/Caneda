/***************************************************************************
 * Copyright (C) 2010 by Pablo Daniel Pareja Obregon                       *
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

#include "webpage.h"

#include "global.h"

namespace Caneda
{
    //! \brief Constructor.
    WebPage::WebPage(QUrl *url)
    {
        setSearchPaths(QStringList(docDirectory() + "/en/"));
        setSource(QUrl(url->path()));
        show();
    }

    void WebPage::setPointSize(qreal size)
    {
        QFont fnt = font();
        fnt.setPointSize(static_cast<int>(qRound(size)));
        setFont(fnt);
    }

    void WebPage::focusInEvent(QFocusEvent *event)
    {
        emit focussed();
        QTextBrowser::focusInEvent(event);
    }

} // namespace Caneda
