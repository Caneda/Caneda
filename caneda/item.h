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

#ifndef ITEM_H
#define ITEM_H

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

// forward declaration
class QMenu;

class SchematicScene;

namespace Qucs {
    class XmlReader;
    class XmlWriter;

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
        TopLeftHandle = 1, //0001
        TopRightHandle = 2, //0010
        BottomRightHandle = 4, //0100
        BottomLeftHandle = 8, //1000
    };

    Q_DECLARE_FLAGS(ResizeHandles, ResizeHandle);
    Q_DECLARE_OPERATORS_FOR_FLAGS(Qucs::ResizeHandles);

    static const QPen handlePen(Qt::darkRed);
    static const QBrush handleBrush(Qt::NoBrush);
    static const QRectF handleRect(-5, -5, 10, 10);

    void drawHighlightRect(QPainter *p, QRectF rect, qreal pw = 1,
            const QStyleOptionGraphicsItem *o = 0);

    void drawResizeHandle(const QPointF &centrePos, QPainter *painter);
    void drawResizeHandles(ResizeHandles handles, const QRectF& rect, QPainter *painter);

    ResizeHandle handleHitTest(const QPointF& point, ResizeHandles handles,
            const QRectF& rect);

}


//! \brief Qucs item - The base class for components, wires, nodes..
class QucsItem : public QGraphicsItem
{
public:
    /*!
     * \brief This enum helps in polymorphic cast without using dynamic_cast.
     *
     * Actually a bitpattern is used to determine whether the cast
     * is valid or not. The cast function is approximately defined like this
     * cast(a,b) {
     * return (a&b) == a;
     * }
     * \sa qucsitem_cast and PATTERN.
     */
    enum QucsItemTypes {
        //!Recognizes all classes derived from QucsItem
        QucsItemType = (1 << (std::numeric_limits<int>::digits-1)),
        //!Recognizes classes derived from SvgItem
        SvgItemType = PATTERN(QucsItemType, 1),
        //!Recognizes classes derived from Component
        ComponentType = PATTERN(SvgItemType, 1),
        //!Recognizes classes derived from Wire
        WireType = PATTERN(QucsItemType, 3),
        //!Recognizes classes derived from Painting
        PaintingType = PATTERN(QucsItemType, 4),
        //!Recognizes classes derived from Display
        DisplayType = PATTERN(QucsItemType, 5)
    };

    //! Item identifier \sa QucsItemTypes
    enum {
        Type = QucsItemType
    };

    QucsItem(QGraphicsItem* parent = 0, SchematicScene* scene = 0);
    virtual ~QucsItem();

    //! Return type of item
    int type() const { return QucsItemType; }
    //! Return bounding box
    QRectF boundingRect() const { return m_boundingRect; }
    //! Return the shape of the item.
    QPainterPath shape() const { return m_shape; }

    SchematicScene* schematicScene() const;

    //! Virtual method to write item's properties to writer.
    virtual void saveData(Qucs::XmlWriter *) const {}
    //! Virtual method to read item's properties from reader.
    virtual void loadData(Qucs::XmlReader *) {}

    QString saveDataText() const;
    void loadDataFromText(const QString &str);

    virtual void mirrorAlong(Qt::Axis axis);
    virtual void rotate90(Qucs::AngleDirection dir = Qucs::AntiClockwise);

    virtual QucsItem* copy(SchematicScene *scene = 0) const;
    virtual void copyDataTo(QucsItem *item) const;

    //! This is convenience method used for rtti.
    virtual bool isComponent() const { return false; }
    //! This is convenience method used for rtti.
    virtual bool isWire() const { return false; }

    //! Subclasses should implement this to launch its own dialog.
    virtual int launchPropertyDialog(Qucs::UndoOption) { return QDialog::Accepted; }

    QMenu* defaultContextMenu() const;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

    void setShapeAndBoundRect(const QPainterPath& path,
            const QRectF& rect,
            qreal penWidth = 1.0);

    QRectF m_boundingRect; //!< Bounding box cache
    QPainterPath m_shape; //!< Shape cache
};

/*!
 * \brief rtti cast function with polymorphic support.
 *
 * This function actually works for items following the rules.
 * Firstly, items should use appropriate Type constant.
 * Secondly, type() should return this Type.
 * \sa QucsItemTypes
 */
template<typename T> T qucsitem_cast(QGraphicsItem *item)
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
 * \brief This function returns a list of qucsitems present in \a items.
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
        T *tItem = qucsitem_cast<T*>(item);
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
 * \brief This function returns a list of qucsitems present in \a items.
 * \param items  The list from which items are to be filtered.
 * \param option Indication whether to remove non matching items from items passed
 *               or not.
 */
template<typename T>
QList<T*> filterItems(QList<QucsItem*> &items, FilterOption option = DontRemoveItems)
{
    QList<T*> tItems;
    QList<QucsItem*>::iterator it = items.begin();
    while(it != items.end()) {
        QucsItem *item = *it;
        T *tItem = qucsitem_cast<T*>(item);
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

#endif //ITEM_H
