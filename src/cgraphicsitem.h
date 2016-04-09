/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012-2014 by Pablo Daniel Pareja Obregon                  *
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

#ifndef C_GRAPHICS_ITEM_H
#define C_GRAPHICS_ITEM_H

#include "global.h"

#include <QDialog>
#include <QGraphicsItem>

#include <limits>

/*!
 * \brief This macro determines the pattern for derived class.
 *
 * The macro accepts two parameter base and shift and returns the
 * the pattern corresponding to derived class.
 */
#define PATTERN(base,shift) (((base) >> (shift)) | (base))

namespace Caneda
{
    // Forward declarations
    class XmlReader;
    class XmlWriter;
    class CGraphicsScene;
    class Port;

    /*!
     * \brief The CGraphicsItem class forms part of the Graphics-View framework,
     * implementing all the common methods to be used by the items in a
     * CGraphicsScene. This class is the base class from which components,
     * wires, paintings, etc, are inherited.
     *
     * Although in theory, a CGraphicsItem could be used directly on a scene,
     * the idea is to subclass CGraphicsItem to allow for the different items
     * to have a more specific behaviour. In this way, this class only
     * implements those methods that are common to all item related classes,
     * as for example rotating and mirroring.
     *
     * \sa CGraphicsScene, Component, Wire, Painting
     */
    class CGraphicsItem : public QGraphicsItem
    {
    public:
        CGraphicsItem(QGraphicsItem* parent = 0, CGraphicsScene* scene = 0);

        /*!
         * \brief CGraphicsItem identification types.
         *
         * This enum helps in the polymorphic cast \a Caneda::canedaitem_cast()
         * without the need to use a dynamic_cast.
         *
         * A type of CGraphicsItemTypes is used in each CGraphicsItem derived
         * class (at the declaration of the Type enum value), in conjunction
         * with a reimplementation of the virtual method type() and this allows
         * during casts to determine the type of CGraphicsItem derived classes.
         *
         * Each enum type represents an item type used by the graphics view
         * framework's cast mechanism. A bitpattern (returned by PATTERN() ) is
         * used to determine whether the cast is valid or not.
         *
         * \sa Caneda::canedaitem_cast(), type(), Type, PATTERN()
         */
        enum CGraphicsItemTypes {
            //!Recognizes all classes derived from CGraphicsItem
            CGraphicsItemType = (1 << (std::numeric_limits<int>::digits-1)),
            //!Recognizes classes derived from Component
            ComponentType = PATTERN(CGraphicsItemType, 1),
            //!Recognizes classes derived from Wire
            WireType = PATTERN(CGraphicsItemType, 2),
            //!Recognizes classes derived from PortSymbol
            PortSymbolType = PATTERN(CGraphicsItemType, 3),
            //!Recognizes classes derived from Painting
            PaintingType = PATTERN(CGraphicsItemType, 4)
        };

        /*!
         * \brief Item identifier
         *
         * This is the type value returned by the virtual type() function, and
         * used by the "Run-Time Type Identification" (RTTI) cast function to
         * determine the type of this item.
         *
         * This \a Type enum is used in in all standard graphics item classes
         * in Qt. All such standard graphics item classes are associated with
         * a unique value for Type, e.g. the value returned by
         * QGraphicsPathItem::type() is 2.
         *
         * \sa type(), CGraphicsItemTypes, Caneda::canedaitem_cast()
         */
        enum { Type = CGraphicsItemType };

        /*!
         * \brief Returns the type of an item as an int.
         *
         * All standard graphicsitem classes are associated with a unique value
         * (one of CGraphicsItemTypes). This type information is used by casts
         * (e.g. qgraphicsitem_cast() ) to distinguish between types. In the
         * particular case of Caneda, a special type of cast is implemented in
         * the method canedaitem_cast(), to recognize any CGraphicsItem derived
         * classes.
         *
         * To enable use of canedaitem_cast() with a custom item, as in this
         * case, this function must be reimplemented and a Type enum value
         * declared (equal to the customized item's type).
         *
         * \sa Type, CGraphicsItemTypes, canedaitem_cast()
         */
        int type() const { return Type; }

        //! Returns a list of ports of the item.
        QList<Port*> ports() const { return m_ports; }

        //! Return bounding box.
        QRectF boundingRect() const { return m_boundingRect; }
        //! Return the shape of the item.
        QPainterPath shape() const { return m_shape; }

        //! Virtual method to write item's properties to writer.
        virtual void saveData(Caneda::XmlWriter *) const {}
        //! Virtual method to read item's properties from reader.
        virtual void loadData(Caneda::XmlReader *) {}

        QString saveDataText() const;
        void loadDataFromText(const QString &str);

        virtual void mirrorAlong(Qt::Axis axis);
        virtual void rotate90(Caneda::AngleDirection dir = Caneda::AntiClockwise);

        virtual CGraphicsItem* copy(CGraphicsScene *scene = 0) const = 0;
        virtual void copyDataTo(CGraphicsItem*item) const;

        //! \brief Subclasses should implement this, to launch its own dialog.
        virtual int launchPropertyDialog(Caneda::UndoOption) { return QDialog::Accepted; }

    protected:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

        void setShapeAndBoundRect(const QPainterPath& path,
                const QRectF& rect,
                qreal penWidth = 1.0);

        QRectF m_boundingRect; //! Bounding box cache
        QPainterPath m_shape; //! Shape cache

        QList<Port*> m_ports; //! Ports list
    };

    /*!
     * \brief Returns the given item cast to type T if item is of type T;
     * otherwise, 0 is returned.
     *
     * This is a "Run-Time Type Identification" (RTTI) cast function with
     * polymorphic support, to recognize any CGraphicsItem derived classes.
     *
     * This function actually works for items with the following rules:
     * \li First, items should use an appropriate Type constant.
     * \li Second, type() should return the previously defined Type.
     *
     * The cast function is approximately defined like this:
     *
     *   cast(a,b) { return (a&b) == a; }
     *
     * thus with the use of the PATTERN method in CGraphicsItemTypes, all
     * CGraphicsItem derived classed are recognized.
     *
     * \sa CGraphicsItemTypes, Type, type()
     */
    template<typename T> T canedaitem_cast(QGraphicsItem *item)
    {
        bool firstCond = int(static_cast<T>(0)->Type) == int(QGraphicsItem::Type);
        bool secondCond = !firstCond && item &&
            ((int(static_cast<T>(0)->Type) & item->type()) == (int(static_cast<T>(0)->Type)));
        bool result = firstCond | secondCond;
        return result ? static_cast<T>(item)  : 0;
    }

    /*!
     * \brief Returns a list of Caneda items present in \a items.
     *
     * \param items  The list from which items are to be filtered.
     */
    template<typename T>
    QList<T*> filterItems(QList<QGraphicsItem*> &items)
    {
        QList<T*> tItems;

        foreach(QGraphicsItem *item, items) {
            T *tItem = canedaitem_cast<T*>(item);
            if(tItem) {
                tItems << tItem;
            }
        }

        return tItems;
    }

    //! Key used to store the current position of an item in it's data field.
    static const int PointKey = 10;
    void storePos(QGraphicsItem *item, const QPointF& pos);
    QPointF storedPos(QGraphicsItem *item);

} // namespace Caneda

#endif //C_GRAPHICS_ITEM_H
