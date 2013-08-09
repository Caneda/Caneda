/***************************************************************************
 * Copyright (C) 2010-2013 by Pablo Daniel Pareja Obregon                  *
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

#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QTextBrowser>

namespace Caneda
{
    /*!
     * \brief This class implements a very basic web browser, to be used in
     * conjuction with the \ref DocumentViewFramework.
     *
     * In a manner similar to Qt's Graphics View Architecture, the WebView
     * class provides the view widget, which visualizes the contents of the
     * document. The view is included as a pointer to WebPage, that contains
     * all the view specific methods. This way, this class represents the
     * QObject counterpart for the WebView class.
     *
     * Although conceptually the WebPage class should be part of the WebView
     * class implementation, abstracting the \ref DocumentViewFramework from
     * the QObject implementations increases the resulting code quality and
     * readability a lot (although there is a slight increase in the number of
     * classes used).
     *
     * \sa \ref DocumentViewFramework, WebView
     */
    class WebPage : public QTextBrowser
    {
        Q_OBJECT

    public:
        WebPage(QUrl *url);

        void setPointSize(qreal size);

    Q_SIGNALS:
        void focussed();

    protected:
        void focusInEvent(QFocusEvent *event);
    };

} // namespace Caneda

#endif //WEBPAGE_H
