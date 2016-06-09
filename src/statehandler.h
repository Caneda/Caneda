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

#ifndef STATE_HANDLER_H
#define STATE_HANDLER_H

#include "global.h"

#include <QObject>

namespace Caneda
{
    // Forward declarations.
    class GraphicsItem;
    class GraphicsView;
    class Painting;

    /*!
     * \brief The StateHandler class is one of Caneda's classes to handle
     * actions (along with the ActionManager class). This class manages the
     * state of the different actions, mantains the mutual exclusiveness of
     * the selectable actions, and applies certain actions to a scene.
     *
     * While the ActionManager is in charge of creating and managing the
     * actual actions (for example returning a pointer to an action of a
     * certain name), this class handles which action is currently selected
     * turning off all other actions. It also applies the corresponding cursor
     * and state to all views, and in certain actions applies the action
     * itself, relagating in the other cases the action to the corresponding
     * scene or view.
     *
     * This class is a singleton class and its only static instance (returned
     * by instance()) is to be used.
     *
     * \sa ActionManager, IView, DocumentViewManager
     */
    class StateHandler : public QObject
    {
        Q_OBJECT

    public:
        static StateHandler* instance();
        ~StateHandler();

    public Q_SLOTS:
        void setNormalAction();
        void performToggleAction(bool on);
        void performToggleAction(const QString& actionName, bool on);

        void insertItem(const QString& item, const QString& category);
        void paste();

    private:
        explicit StateHandler(QObject *parent = 0);

        void applyCursor(GraphicsView *view);
        void applyState(GraphicsView *view);

        void clearInsertibles();

        Caneda::MouseAction mouseAction;
        QList<GraphicsItem*> insertibles;
        Painting *paintingDrawItem;
    };

} // namespace Caneda

#endif //STATE_HANDLER_H
