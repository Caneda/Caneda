/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef SCHEMATIC_CONTEXT_H
#define SCHEMATIC_CONTEXT_H

#include "icontext.h"

namespace Caneda
{
    //Forward declarations
    class Action;
    class SidebarBrowser;

    /*!
     * \brief This class represents the schematic context interface
     * implementation.
     *
     * Only one instance of this class is used during the whole life span of
     * the program. This class answers the general questions like which file
     * suffixes it can handle, points to the appropiate methods to create new
     * documents of its type, etc.
     *
     * This class also provides objects like the toolbar, statusbar, etc, which
     * are specific to this particular context.
     *
     * This class is a singleton class and its only static instance (returned
     * by instance()) is to be used.
     *
     * \sa IContext, IDocument, IView, \ref DocumentViewFramework
     * \sa SchematicDocument, SchematicView
     */
    class SchematicContext : public IContext
    {
        Q_OBJECT

    public:
        static SchematicContext* instance();
        virtual ~SchematicContext();

        // IContext interface methods
        virtual QWidget* sideBarWidget();

        virtual bool canOpen(const QFileInfo &info) const;
        virtual QStringList fileNameFilters() const;
        virtual QString defaultSuffix() const { return "xsch";}

        virtual IDocument* newDocument();
        virtual IDocument* open(const QString &fileName, QString *errorMessage = 0);
        // End of IContext interface methods

        void addNormalAction(Action *action);
        void addMouseAction(Action *action);

    private:
        SchematicContext(QObject *parent = 0);

        // FIXME: In future disable/hide actions when context goes out of scope i.e say a Text view
        // was focussed in which case schematic actions become irrelevant.
        QList<Action*> m_normalActions;
        QList<Action*> m_mouseActions;

        SidebarBrowser *m_sidebarBrowser;
    };

} // namespace Caneda

#endif //SCHEMATIC_CONTEXT_H
