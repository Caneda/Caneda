/***************************************************************************
 * Copyright (C) 2012 by Pablo Daniel Pareja Obregon                       *
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

#ifndef PROPERTYDISPLAY_H
#define PROPERTYDISPLAY_H

#include <QGraphicsSimpleTextItem>
#include <QObject>

namespace Caneda
{
    // Forward declarations.
    class Component;
    class CGraphicsScene;

    /*!
     * \brief Class used to render a properties on a graphics scene.
     *
     * Gouping all properties of a component into a QMap (PropertyGroup)
     * provides a convenient way of handling them all together. In this
     * way, the properties of a component can be selected and moved
     * all at once.
     *
     * While Property class holds actual properties, PropertyGroup class
     * groups them all together and PropertyDisplay class is the object
     * that renders them on a scene, allowing selection and moving of all
     * properties at once.
     *
     * \sa Property, PropertyGroup
     */
    class PropertyDisplay : public QObject, public QGraphicsSimpleTextItem
    {
        Q_OBJECT

    public:
        PropertyDisplay(CGraphicsScene *scene = 0);

        void updateProperties();
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget = 0 );

        Component* component() const;

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    };

} // namespace Caneda

#endif //PROPERTYDISPLAY_H
