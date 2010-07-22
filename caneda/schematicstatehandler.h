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

#ifndef SCHEMATIC_STATE_HANDLER_H
#define SCHEMATIC_STATE_HANDLER_H

#include <QObject>

namespace Caneda
{
    // Forward declarations.
    class SchematicStateHandlerPrivate;
    class SchematicScene;
    class SchematicWidget;

    class SchematicStateHandler : public QObject
    {
        Q_OBJECT
    public:
        static SchematicStateHandler* instance();
        ~SchematicStateHandler();

        void registerWidget(SchematicWidget *widget);
        void unregisterWidget(SchematicWidget *widget);


    public Q_SLOTS:
        void slotSidebarItemClicked(const QString& item, const QString& category);
        void slotHandlePaste();
        void slotRotateInsertibles();
        void slotMirrorInsertibles();
        void slotOnObjectDestroyed(QObject *sender);
        void slotUpdateFocussedWidget(SchematicWidget *widget);
        void slotPerformToggleAction(const QString& sender, bool on);
        void slotSetNormalAction();
        void slotInsertToolbarComponent(const QString& action, bool on);
        void slotUpdateToolbarInsertibles();

    private:
        SchematicStateHandler(QObject *parent = 0);

        void applyCursor(SchematicWidget *widget);
        void applyState(SchematicWidget *widget);
        void applyStateToAllWidgets();

        SchematicStateHandlerPrivate *d;
    };

} // namespace Caneda

#endif //SCHEMATIC_STATE_HANDLER_H
