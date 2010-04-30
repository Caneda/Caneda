/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef SCHEMATICVIEW_H
#define SCHEMATICVIEW_H

#include "canedaview.h"

#include <QGraphicsView>

namespace Caneda
{
    // Forward declarations
    class SchematicItem;
    class SchematicScene;

    class SchematicWidget : public QGraphicsView, public CanedaView
    {
    Q_OBJECT
    public:
        static const qreal zoomFactor;

        SchematicWidget(SchematicScene *sc = 0,QWidget *parent = 0);
        ~SchematicWidget();

        SchematicScene* schematicScene() const;

        //reimplemented virtuals from CanedaView
        void setFileName(const QString& name);
        QString fileName() const;

        bool load();
        bool save();

        void zoomIn();
        void zoomOut();

        void showAll();
        void showNoZoom();

        bool isSchematicWidget() const { return true; }

        QWidget* toWidget() const;
        SchematicWidget* toSchematicWidget() const;

        bool isModified() const;

        void copy() const;
        void cut();
        void paste();

        void saveScrollState();
        void restoreScrollState();

    public Q_SLOTS:
        void setModified(bool m);

    signals:
        void modificationChanged(bool modified);
        void fileNameChanged(const QString& file);
        void titleToBeUpdated();
        void cursorPositionChanged(const QString& newPos);
        void focussed(SchematicWidget *view);
        void pasteInvoked();

    protected:
        void mouseMoveEvent(QMouseEvent *event);
        void focusInEvent(QFocusEvent *event);

    private:
        int m_horizontalScroll;
        int m_verticalScroll;
    };

} // namespace Caneda

#endif //SCHEMATICVIEW_H
