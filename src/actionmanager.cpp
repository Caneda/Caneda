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

#include "actionmanager.h"

namespace Caneda
{
    //! \brief Constructor.
    ActionManager::ActionManager(QObject *parent) : QObject(parent)
    {
    }

    //! \copydoc MainWindow::instance()
    ActionManager* ActionManager::instance()
    {
        static ActionManager *instance = 0;
        if (!instance) {
            instance = new ActionManager();
        }
        return instance;
    }

    /*!
     * \brief Create a new action and add it to the hash.
     *
     * Creates a new action and adds it to an internal hash, to be able to
     * recover it afterwars when needed by using the actionForName() method
     * call.
     *
     * \sa actionForName()
     */
    QAction *ActionManager::createAction(const QString& id,
            const QIcon& icon, const QString& text)
    {
        QAction *action = new QAction(icon, text, this);
        action->setObjectName(id);
        m_actionHash.insert(id, action);
        return action;
    }

    //! \copydoc createAction()
    QAction *ActionManager::createAction(const QString& id,
            const QString& text)
    {
        return createAction(id, QIcon(), text);
    }

    /*!
     * \brief Create a new mouse action and add it to the hash.
     *
     * Creates a new mouse action and adds it to an internal hash, to be able
     * to recover it afterwars when needed by using the actionForName() method
     * call. The difference with the createAction() method call is that mouse
     * actions are checkable by default. A separate hash is also kept to be
     * able to differenciate the two kind of actions.
     *
     * \sa actionForName(), createAction()
     */
    QAction *ActionManager::createMouseAction(const QString& id,
            Caneda::MouseAction mouseAction, const QIcon& icon,
            const QString& text)
    {
        QAction *action = createAction(id, icon, text);
        action->setCheckable(true);
        m_mouseActionHash.insert(action, mouseAction);
        return action;
    }

    //! \copydoc createMouseAction()
    QAction *ActionManager::createMouseAction(const QString& id,
            Caneda::MouseAction mouseAction, const QString& text)
    {
        return createMouseAction(id, mouseAction, QIcon(), text);
    }

    /*!
     * \brief Create a new recent files action and add it to the hash.
     *
     * Creates a new recent files action and adds it to an internal hash, to be
     * able to recover it afterwars when needed. This is a special kind of
     * actions that are used to hold the names of the recently opened files and
     * used during the creation of the recent files menu.
     *
     * \sa recentFilesActions(), MainWindow::openRecent(),
     * DocumentViewManager::updateRecentFilesActionList()
     */
    QAction *ActionManager::createRecentFilesAction()
    {
        QAction *action = new QAction(this);
        action->setVisible(false);
        m_recentFileActionList.append(action);
        return action;
    }

    //! \brief Return the action \a name from the hash.
    QAction* ActionManager::actionForName(const QString& name) const
    {
        return m_actionHash.value(name, static_cast<QAction*>(0));
    }

    //! \brief Return the mouse action \a ma from the hash
    Caneda::MouseAction ActionManager::mouseActionForAction(QAction *action) const
    {
        return m_mouseActionHash.value(action);
    }

    //! \brief Return the list of mouse actions available.
    QList<QAction*> ActionManager::mouseActions() const
    {
        return m_mouseActionHash.keys();
    }

    //! \brief Return the list of recently opened files available represented by actions.
    QList<QAction *> ActionManager::recentFilesActions() const
    {
        return m_recentFileActionList;
    }

} // namespace Caneda
