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

#ifndef CANEDA_ICONTEXT_H
#define CANEDA_ICONTEXT_H

#include "globals.h"

#include <QObject>

// Forward declaration
class QFileInfo;
class QToolBar;
class QWidget;

namespace Caneda
{
    // Forward declarations.
    class IDocument;
    class IView;

    class IContext : public QObject
    {
    Q_OBJECT
    public:
        IContext(QObject *parent = 0);
        virtual ~IContext();

        virtual void init();

        virtual QToolBar* toolBar();
        virtual QWidget* statusBarWidget();
        virtual QWidget* sideBarWidget(Caneda::SideBarRole role);

        virtual bool canOpen(const QFileInfo& info) const = 0;
        virtual QStringList fileNameFilters() const = 0;

        virtual IDocument* open(const QString& filename, QString *errorMessage = 0) = 0;
        virtual IView* createView(IDocument *document) = 0;

    };
} // namespace Caneda

#endif //CANEDA_ICONTEXT_H
