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

#include <QObject>

namespace Caneda
{
    // Forward declarations.
    class StateHandlerPrivate;
    class CGraphicsView;

    /*!
     * \todo Document this class.
     *
     * This class is a singleton class and its only static instance (returned
     * by instance()) is to be used.
     */
    class StateHandler : public QObject
    {
        Q_OBJECT

    public:
        static StateHandler* instance();
        ~StateHandler();

        void registerWidget(CGraphicsView *widget);
        void unregisterWidget(CGraphicsView *widget);


    public Q_SLOTS:
        void slotSidebarItemClicked(const QString& item, const QString& category);
        void slotHandlePaste();
        void slotOnObjectDestroyed(QObject *object);
        void slotUpdateFocussedWidget(CGraphicsView *widget);
        void slotPerformToggleAction(bool on);
        void slotPerformToggleAction(const QString& actionName, bool on);
        void slotSetNormalAction();
        void slotInsertToolbarComponent(const QString& action, bool on);

    private:
        StateHandler(QObject *parent = 0);

        void applyCursor(CGraphicsView *widget);
        void applyState(CGraphicsView *widget);
        void applyStateToAllWidgets();

        StateHandlerPrivate *d;
    };

} // namespace Caneda

#endif //STATE_HANDLER_H
