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

#include "actionmanager.h"

#include "singletonmanager.h"

ActionSignalMapper::ActionSignalMapper(QObject *parent)
    : QObject(parent)
{
}

ActionSignalMapper::~ActionSignalMapper()
{
}

void ActionSignalMapper::setMapping(QAction *sender, const QString &text)
{
    m_stringHash.insert(sender, text);
    connect(sender, SIGNAL(destroyed()), SLOT(slotActionDestroyed()));
    connect(sender, SIGNAL(toggled(bool)), SLOT(slotMapToggled(bool)));
    connect(sender, SIGNAL(triggered(bool)), SLOT(slotMapTriggered(bool)));
}

void ActionSignalMapper::removeMappings(QAction *sender)
{
    m_stringHash.remove(sender);
}

QAction *ActionSignalMapper::mapping(const QString& text) const
{
    return m_stringHash.key(text);
}

void ActionSignalMapper::slotMapToggled(bool status)
{
    QAction *s = qobject_cast<QAction*>(sender());
    if (!s) {
        return;
    }
    Hash::const_iterator it = m_stringHash.constFind(s);
    if (it != m_stringHash.constEnd()) {
        emit mappedToggled(it.value(), status);
    }
}

void ActionSignalMapper::slotMapTriggered(bool status)
{
    QAction *s = qobject_cast<QAction*>(sender());
    if (!s) {
        return;
    }
    Hash::const_iterator it = m_stringHash.constFind(s);
    if (it != m_stringHash.constEnd()) {
        emit mappedTriggered(it.value(), status);
    }
}

void ActionSignalMapper::slotActionDestroyed(QObject *which)
{
    QAction *asAction = qobject_cast<QAction*>(which);
    if (asAction) {
        removeMappings(asAction);
    }
}

Action::Action(QObject *parent) : QAction(parent)
{
    init();
}

Action::Action(const QString& text, QObject *parent)
    : QAction(text, parent)
{
    init();
}

Action::Action(const QIcon& icon, const QString& text, QObject *parent)
    : QAction(icon, text, parent)
{
    init();
}

Action::~Action()
{
}

void Action::init()
{
    connect(this, SIGNAL(toggled(bool)), SLOT(slotToggled(bool)));
    connect(this, SIGNAL(triggered(bool)), SLOT(slotTriggered(bool)));
}

void Action::slotToggled(bool checked)
{
    emit toggled(objectName(), checked);
}

void Action::slotTriggered(bool checked)
{
    emit triggered(objectName(), checked);
}

ActionManager* ActionManager::instance()
{
    return SingletonManager::instance()->actionManager();
}

ActionManager::ActionManager(QObject *parent) : QObject(parent)
{

}

ActionManager::~ActionManager()
{

}

Action* ActionManager::createAction(const QString& id,
        const QIcon& icon, const QString& text)
{
    Action *action = new Action(icon, text, this);
    action->setObjectName(id);
    m_actionHash.insert(id, action);
    return action;
}

Action* ActionManager::createMouseAction(const QString& id,
        SchematicScene::MouseAction mouseAction, const QIcon& icon,
        const QString& text)
{
    Action *action = createAction(id, icon, text);
    action->setCheckable(true);
    m_mouseActionHash.insert(action, mouseAction);
    return action;
}

Action* ActionManager::createAction(const QString& id,
        const QString& text)
{
    return createAction(id, QIcon(), text);
}

Action* ActionManager::createMouseAction(const QString& id,
        SchematicScene::MouseAction mouseAction, const QString& text)
{
    return createMouseAction(id, mouseAction, QIcon(), text);
}

Action* ActionManager::actionForName(const QString& name) const
{
    return m_actionHash.value(name, static_cast<Action*>(0));
}

Action* ActionManager::actionForMouseAction(SchematicScene::MouseAction ma) const
{
    return m_mouseActionHash.key(ma);
}

SchematicScene::MouseAction ActionManager::mouseActionForAction(Action *action) const
{
    return m_mouseActionHash.value(action);
}

QList<Action*> ActionManager::mouseActions() const
{
    return m_mouseActionHash.keys();
}
