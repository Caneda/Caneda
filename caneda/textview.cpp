/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "textview.h"

#include "textcontext.h"
#include "textdocument.h"
#include "textedit.h"

#include <QApplication>
#include <QFont>
#include <QFontInfo>

namespace Caneda
{
    TextView::TextView(TextDocument *document) :
        IView(document),
        m_zoomRange(6.0, 30.0),
        m_originalZoom(QFontInfo(qApp->font()).pointSizeF())
    {
        m_currentZoom = m_originalZoom;
        m_textEdit = new TextEdit(document->textDocument());
        connect(m_textEdit, SIGNAL(focussed()), this,
                SLOT(onFocussed()));
    }

    TextView::~TextView()
    {
        delete m_textEdit;;
    }

    TextEdit* TextView::textEdit() const
    {
        return m_textEdit;
    }

    QWidget* TextView::toWidget() const
    {
        return m_textEdit;
    }

    IContext* TextView::context() const
    {
        return TextContext::instance();
    }

    void TextView::setZoomLevel(qreal zoomLevel)
    {
        if (!m_zoomRange.contains(zoomLevel)) {
            return;
        }

        if (qFuzzyCompare(zoomLevel, m_currentZoom)) {
            return;
        }

        m_currentZoom = zoomLevel;

        m_textEdit->setPointSize(m_currentZoom);
    }

    void TextView::zoomIn()
    {
        setZoomLevel(m_currentZoom + 1);
    }

    void TextView::zoomOut()
    {
        setZoomLevel(m_currentZoom - 1);
    }

    void TextView::zoomFitInBest()
    {
        setZoomLevel(4);
    }

    void TextView::zoomOriginal()
    {
        setZoomLevel(m_originalZoom);
    }

    IView* TextView::duplicate()
    {
        return document()->createView();
    }

    void TextView::updateSettingsChanges()
    {
    }

    void TextView::onFocussed()
    {
        emit focussedIn(static_cast<IView*>(this));
    }

} // namespace Caneda
