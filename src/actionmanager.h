/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2016 by Pablo Daniel Pareja Obregon                       *
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

#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include "global.h"

#include <QAction>
#include <QHash>

namespace Caneda
{
    /*!
     * \brief Singleton class to manage all of Caneda's actions in a single place.
     *
     * This object is a singleton class to manage all of Caneda's actions in a
     * single place. This allows to access and create any number of actions
     * from any place throughout the code, without risking of having duplicates
     * and allowing the access of actions created somewhere else by other class.
     *
     * By using a singleton, only one object instance of this class is allowed
     * and created at any time thoughout all Caneda's process execution. Its
     * only static instance (returned by instance()) is to be used.
     */
    class ActionManager : public QObject
    {
        Q_OBJECT

    public:
        static ActionManager* instance();

        QAction* createAction(const QString& id, const QIcon& icon, const QString& text);
        QAction* createAction(const QString& id, const QString& text);

        QAction* createMouseAction(const QString& id, Caneda::MouseAction action,
                const QIcon& icon, const QString& text);
        QAction* createMouseAction(const QString& id, Caneda::MouseAction action,
                const QString& text);

        QAction* createRecentFilesAction();

        QAction* actionForName(const QString& name) const;
        Caneda::MouseAction mouseActionForAction(QAction *action) const;

        QList<QAction*> actions() const;
        QList<QAction*> mouseActions() const;
        QList<QAction*> recentFilesActions() const;

    private:
        explicit ActionManager(QObject *parent = 0);

        QHash<QString, QAction*> m_actionHash;
        QHash<QAction*, Caneda::MouseAction> m_mouseActionHash;
        QList<QAction*> m_recentFileActionList;
    };

} // namespace Caneda

#endif //ACTIONMANAGER_H
