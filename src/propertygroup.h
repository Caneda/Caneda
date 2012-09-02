/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef PROPERTYGROUP_H
#define PROPERTYGROUP_H

#include <QGraphicsItemGroup>
#include <QObject>

namespace Caneda
{
    // Forward declarations.
    class Component;
    class CGraphicsScene;
    class PropertyItem;

    /*!
     * \brief This class groups the properties of a item.
     *
     * Gouping all PropertyItems of a component into a QGraphicsItemGroup
     * provides a convenient way of handling them all together. In this
     * way, the properties of a component can be selected and moved
     * all at once.
     *
     * This class takes care of creation and destruction of PropertyItems
     * as well.
     *
     * While Property class holds actual properties, PropertyItem
     * class is the object that renders them on a scene. Finally, PropertiesGroup
     * is the class that groups all PropertyItems to allow selection and
     * moving of all properties at once.
     *
     * \sa Property, PropertyItem
     */
    class PropertiesGroup : public QObject, public QGraphicsItemGroup
    {
        Q_OBJECT

    public:
        enum { PropertiesGroupType = UserType + 73 };
        enum { Type = PropertiesGroupType };

        PropertiesGroup(CGraphicsScene *scene = 0);

        //! Class identifier.
        int type() { return PropertiesGroupType; }

        void realignItems();
        void forceUpdate();

        CGraphicsScene* cGraphicsScene() const;
        Component* component() const;

        void setFontSize(int pointSize);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget = 0 );

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    private:
        //! Internal storage of property items for book keeping.
        QMap<QString, PropertyItem*> m_propertyItemsMap;
        int m_pointSize;
    };

} // namespace Caneda

#endif //PROPERTYGROUP_H
