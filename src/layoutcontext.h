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

#ifndef LAYOUT_CONTEXT_H
#define LAYOUT_CONTEXT_H

#include "icontext.h"

namespace Caneda
{
    //Forward declarations
    class Action;
    class SidebarBrowser;

    class LayoutContext : public IContext
    {
        Q_OBJECT

    public:
        static LayoutContext* instance();
        ~LayoutContext();

        // IContext interface methods
        virtual void init();

        virtual QWidget* sideBarWidget();

        virtual bool canOpen(const QFileInfo &info) const;
        virtual QStringList fileNameFilters() const;
        virtual QString defaultSuffix() const { return "xlay";}

        virtual IDocument* newDocument();
        virtual IDocument* open(const QString &fileName, QString *errorMessage = 0);
        // End of IContext interface methods

        void addNormalAction(Action *action);
        void addMouseAction(Action *action);

    private Q_SLOTS:
        void slotIntoHierarchy();
        void slotPopHierarchy();

        void slotAlignTop();
        void slotAlignBottom();
        void slotAlignLeft();
        void slotAlignRight();
        void slotDistributeHorizontal();
        void slotDistributeVertical();
        void slotCenterHorizontal();
        void slotCenterVertical();

    private:
        LayoutContext(QObject *parent = 0);

        void alignElements(Qt::Alignment alignment);
        void setNormalAction();

        //FIXME: In future disable/hide actions when context goes out of scope i.e say a Text view
        // was focussed in which case schematic actions become irrelevant.
        QList<Action*> m_normalActions;
        QList<Action*> m_mouseActions;

        SidebarBrowser *m_sidebarBrowser;
    };

} // namespace Caneda

#endif //LAYOUT_CONTEXT_H
