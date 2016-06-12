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
     * \brief The ActionManager class is in charge of managing all of Caneda's
     * actions in a single place. It allows creation, listing and getting
     * pointers to actions given their name.
     *
     * This class is a singleton, allowing to access and create any number of
     * actions from any place throughout the code, without risking of having
     * duplicates and allowing the access of actions created somewhere else by
     * other class.
     *
     * While the StateHandler class is in charge of handling which action is
     * currently selected and relaying the cursor and state to all views, this
     * class creates and manages the actual actions throughout their lifetime.
     *
     * This class is a singleton class and its only static instance (returned
     * by instance()) is to be used.
     *
     * \sa MainWindow, StateHandler
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
