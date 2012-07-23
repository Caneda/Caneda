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

#ifndef C_GRAPHICS_ITEM_H
#define C_GRAPHICS_ITEM_H

#include <QBrush>
#include <QDialog>
#include <QFlags>
#include <QGraphicsItem>
#include <QPen>

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

    //! This enum determines the rotation direction.
    enum AngleDirection {
        Clockwise,
        AntiClockwise
    };

    enum UndoOption {
        DontPushUndoCmd,
        PushUndoCmd
    };

    enum ResizeHandle {
        NoHandle = 0,
        TopLeftHandle = 1,     //0001
        TopRightHandle = 2,    //0010
        BottomRightHandle = 4, //0100
        BottomLeftHandle = 8   //1000
    };

    Q_DECLARE_FLAGS(ResizeHandles, ResizeHandle)
    Q_DECLARE_OPERATORS_FOR_FLAGS(Caneda::ResizeHandles)

    static const QPen handlePen(Qt::darkRed);
    static const QBrush handleBrush(Qt::NoBrush);
    static const QRectF handleRect(-5, -5, 10, 10);

    void drawHighlightRect(QPainter *p, QRectF rect, qreal pw = 1,
            const QStyleOptionGraphicsItem *o = 0);

    void drawResizeHandle(const QPointF &centrePos, QPainter *painter);
    void drawResizeHandles(ResizeHandles handles, const QRectF& rect, QPainter *painter);

    ResizeHandle handleHitTest(const QPointF& point, ResizeHandles handles,
            const QRectF& rect);


    //! \brief CGraphicsItem - The base class for components, wires, nodes...
    class CGraphicsItem : public QGraphicsItem
    {
    public:
        /*!
         * \brief Graphics View Framework id. This enum helps in polymorphic cast
         * without using dynamic_cast.
         *
         * Represents item type used by graphics view framework's cast
         * mechanism. Actually a bitpattern is used to determine whether the cast
         * is valid or not. The cast function is approximately defined like this
         * cast(a,b) { return (a&b) == a; }
         *
         * \sa canedaitem_cast and PATTERN.
         */
        enum CGraphicsItemTypes {
            //!Recognizes all classes derived from CGraphicsItem
            CGraphicsItemType = (1 << (std::numeric_limits<int>::digits-1)),
            //!Recognizes classes derived from Component
            ComponentType = PATTERN(CGraphicsItemType, 1),
            //!Recognizes classes derived from Wire
            WireType = PATTERN(CGraphicsItemType, 2),
            //!Recognizes classes derived from Node
            NodeType = PATTERN(CGraphicsItemType, 3),
            //!Recognizes classes derived from Painting
            PaintingType = PATTERN(CGraphicsItemType, 4)
        };

        //! Item identifier \sa CGraphicsItemTypes
        enum {
            Type = CGraphicsItemType
        };

        CGraphicsItem(QGraphicsItem* parent = 0, CGraphicsScene* scene = 0);
        virtual ~CGraphicsItem() {}

        //! Returns a list of ports of the item.
        QList<Port*> ports() const { return m_ports; }
        int checkAndConnect(Caneda::UndoOption opt);

        //! Return type of item
        int type() const { return CGraphicsItemType; }
        //! Return bounding box
        QRectF boundingRect() const { return m_boundingRect; }
        //! Return the shape of the item.
        QPainterPath shape() const { return m_shape; }

        CGraphicsScene* cGraphicsScene() const;

        //! Virtual method to write item's properties to writer.
        virtual void saveData(Caneda::XmlWriter *) const {}
        //! Virtual method to read item's properties from reader.
        virtual void loadData(Caneda::XmlReader *) {}

        QString saveDataText() const;
        void loadDataFromText(const QString &str);

        virtual void mirrorAlong(Qt::Axis axis);
        virtual void rotate90(Caneda::AngleDirection dir = Caneda::AntiClockwise);

        virtual CGraphicsItem* copy(CGraphicsScene *scene = 0) const;
        virtual void copyDataTo(CGraphicsItem*item) const;

        //! This is convenience method used for rtti.
        virtual bool isComponent() const { return false; }
        //! This is convenience method used for rtti.
        virtual bool isWire() const { return false; }

        //! Subclasses should implement this to launch its own dialog.
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
     * \brief rtti cast function with polymorphic support.
     *
     * This function actually works for items following the rules.
     * Firstly, items should use appropriate Type constant.
     * Secondly, type() should return this Type.
     * \sa CGraphicsItemTypes
     */
    template<typename T> T canedaitem_cast(QGraphicsItem *item)
    {
        bool firstCond = int(static_cast<T>(0)->Type) == int(QGraphicsItem::Type);
        bool secondCond = !firstCond && item &&
            ((int(static_cast<T>(0)->Type) & item->type()) == (int(static_cast<T>(0)->Type)));
        bool result = firstCond | secondCond;
        return result ? static_cast<T>(item)  : 0;
    }

    //! This enum is used in \a filterItems method to determine filtering.
    enum FilterOption {
        DontRemoveItems,
        RemoveItems
    };

    /*!
     * \brief This function returns a list of canedaitems present in \a items.
     * \param items  The list from which items are to be filtered.
     * \param option Indication whether to remove non matching items from items passed
     *               or not.
     */
    template<typename T>
    QList<T*> filterItems(QList<QGraphicsItem*> &items, FilterOption option = DontRemoveItems)
    {
        QList<T*> tItems;
        QList<QGraphicsItem*>::iterator it = items.begin();
        while(it != items.end()) {
            QGraphicsItem *item = *it;
            T *tItem = canedaitem_cast<T*>(item);
            if(tItem) {
                tItems << tItem;
                if(option == RemoveItems) {
                    it = items.erase(it);
                }
                else {
                    ++it;
                }
            }
            else {
                ++it;
            }
        }
        return tItems;
    }

    /*!
     * \brief This function returns a list of canedaitems present in \a items.
     * \param items  The list from which items are to be filtered.
     * \param option Indication whether to remove non matching items from items passed
     *               or not.
     */
    template<typename T>
    QList<T*> filterItems(QList<CGraphicsItem*> &items, FilterOption option = DontRemoveItems)
    {
        QList<T*> tItems;
        QList<CGraphicsItem*>::iterator it = items.begin();
        while(it != items.end()) {
            CGraphicsItem *item = *it;
            T *tItem = canedaitem_cast<T*>(item);
            if(tItem) {
                tItems << tItem;
                if(option == RemoveItems) {
                    it = items.erase(it);
                }
                else {
                    ++it;
                }
            }
            else {
                ++it;
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
