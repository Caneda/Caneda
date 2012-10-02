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

#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include "global.h"

#include <QAction>
#include <QHash>

namespace Caneda
{

    class Action : public QAction
    {
        Q_OBJECT
    public:
        Action(QObject *parent = 0);
        Action(const QString& text, QObject *parent = 0);
        Action(const QIcon& icon, const QString& text, QObject *parent = 0);
        ~Action();

    Q_SIGNALS:
        void toggled(const QString& sender, bool checked);
        void triggered(const QString& sender, bool checked = false);

    private Q_SLOTS:
        void slotToggled(bool checked);
        void slotTriggered(bool checked);

    private:
        void init();
    };

    class ActionManager : public QObject
    {
    Q_OBJECT
    public:
        static ActionManager* instance();
        ~ActionManager();

        Action* createAction(const QString& id, const QIcon& icon, const QString& text);
        Action* createAction(const QString& id, const QString& text);

        Action* createMouseAction(const QString& id, Caneda::MouseAction action,
                const QIcon& icon, const QString& text);
        Action* createMouseAction(const QString& id, Caneda::MouseAction action,
                const QString& text);

        Action* actionForName(const QString& name) const;
        Action* actionForMouseAction(Caneda::MouseAction ma) const;
        Caneda::MouseAction mouseActionForAction(Action *action) const;

        QList<Action*> mouseActions() const;

    private:
        ActionManager(QObject *parent = 0);

        QHash<QString, Action*> m_actionHash;
        QHash<Action*, Caneda::MouseAction> m_mouseActionHash;
    };

} // namespace Caneda

#endif //ACTIONMANAGER_H
