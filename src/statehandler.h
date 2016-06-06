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

    class StateHandler : public QObject
    {
        Q_OBJECT

    public:
        static StateHandler* instance();
        ~StateHandler();

        void registerWidget(GraphicsView *widget);
        void unregisterWidget(GraphicsView *widget);

    public Q_SLOTS:
        void setNormalAction();
        void performToggleAction(bool on);
        void performToggleAction(const QString& actionName, bool on);

        void insertItem(const QString& item, const QString& category);

        void handlePaste();

        void objectDestroyed(QObject *object);
        void updateFocussedWidget(GraphicsView *widget);

    private:
        explicit StateHandler(QObject *parent = 0);

        void applyCursor(GraphicsView *widget);
        void applyState(GraphicsView *widget);

        void clearInsertibles();

        Caneda::MouseAction mouseAction;
        QList<GraphicsItem*> insertibles;
        Painting *paintingDrawItem;

        QSet<GraphicsView*> widgets;
        GraphicsView *focussedWidget;
    };

} // namespace Caneda

#endif //STATE_HANDLER_H
