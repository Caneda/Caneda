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

#include "clineedit.h"

#include "global.h"

#include <QStyle>
#include <QToolButton>

namespace Caneda
{
    CLineEdit::CLineEdit(QWidget *parent) : QLineEdit(parent)
    {
        clearButton = new QToolButton(this);

        clearButton->setIcon(Caneda::icon("clearFilterText"));
        clearButton->setCursor(Qt::ArrowCursor);
        clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
        clearButton->setWhatsThis(tr("Clear text\n\nClears the filter text"));
        clearButton->setToolTip(tr("Clear text"));
        clearButton->hide();

        connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
        connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateCloseButton(const QString&)));
    }

    void CLineEdit::resizeEvent(QResizeEvent *)
    {
        QSize sz = clearButton->sizeHint();

        int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

        clearButton->move(rect().right() - frameWidth - sz.width() - 3,
                          (rect().bottom() + 3 - sz.height())/2 );
    }

    void CLineEdit::updateCloseButton(const QString& text)
    {
        clearButton->setVisible(!text.isEmpty());
    }

} // namespace Caneda
