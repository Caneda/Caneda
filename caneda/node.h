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

#ifndef NODE_H
#define NODE_H

#include "component.h"

#include <QList>

// forward declaration
class Wire;

/*!
 * \brief Node class
 * A graphical node is little circle allowing to plug wires or components
 */
class Node : public QucsItem
{
public:
    /*!
     * \brief Graphics View Framework id.
     *
     * Represents item type used by graphics view framework's cast
     * mechanism.
     * \sa qucsitem_cast
     */
    enum {
        Type = QucsItem::NodeType
    };

    //! \brief Represents radius of the node's visual circular representation.
    static const qreal Radius;

    //! \brief Represens the z-level of the node
    static const qreal ZValue;

    //! \brief The bounding rect of a every node is constant. So cache it.
    static const QRectF BoundRect;

    //! \brief The shape of every node is const, so cache it too.
    static QPainterPath Shape;

    Node(const QString& name = QString(),SchematicScene *scene = 0);
    ~Node();

    void addComponent(Component *comp);
    void removeComponent(Component *comp);

    //! \brief Return the list of components attached to this node
    QList<Component*> components() { return m_components; }
    QList<Component*> selectedComponents() const;

    bool areAllComponentsSelected() const;
    void addAllComponentsFrom(const Node *other);

    void addWire(Wire *w);
    void removeWire(Wire *w);

    //! \brief Return list of wire attached to this node
    QList<Wire*> wires() const { return m_wires; }
    QList<Wire*> selectedWires() const;

    void addAllWiresFrom(const Node *other);

    //! \brief Test if this node has one and only one connection
    bool isOpen() const {
        return totalConnections() == 1;
    }
    /*!
     * \brief Test if this node is empty
     * This is mostly used to determine if this node can be deleted.
     */
    bool isEmpty() const {
        return (m_components.isEmpty() && m_wires.isEmpty());
    }

    //! \brief Return the number of connection attached to this node
    int totalConnections() const {
        return m_components.size() + m_wires.size();
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w = 0);

    /*!
     * \brief Test if point is included in node
     * \param pt: point to test
     * \return true if this point is in the node; otherwise returns false.
     * \todo This method should be as fast as possible
     * Therefore try to avoid sqrt
     * do sometthing like pt.x()*pt.x()+pt.y()*pt.y()- Node::Radius*Node::Radius <= 0
     */
    bool contains(const QPointF& pt) const {
        return pt.x()*pt.x() + pt.y()*pt.y() - Node::Radius*Node::Radius <= 0.0;
    }
    bool collidesWithItem(QGraphicsItem *other) const;

    //!\brief Bounding box of this item (square tangent to a circle)
    QRectF boundingRect() const {
        return Node::BoundRect;
    }
    //!\brief Shape of node
    QPainterPath shape() const {
        return Node::Shape;
    }

    /*!
     * \brief Return name of node
     * \sa setName
     */
    QString name() const {
        return m_name;
    }
    void setName(const QString& name);


    /*!
     * \brief Sets the node's movement controller on screen to c.
     *
     * This is needed because a single node is usually shared by many
     * components. When the components are moved care should be taken
     * not to move the same node again and again. In those cases one
     * of component is chosen as the controller.
     * \param c This is the component which when moved also moves this node.
     */
    void setController(Component *c) {
        m_controller = c;
    }
    /*!
     * \brief Returns the active controller component.
     * \sa setController
     */
    Component* controller() const {
        return m_controller;
    }

    /*!
     * \brief return GraphicsView framwork id.
     * \sa Type
     */
    int type() const { return QucsItem::NodeType; }

    //!\brief Empty because circle at any rotation is the same.
    virtual void rotate() {}
    //!\brief Empty because circle when mirrored is the same.
    void mirrorAlong(Qt::Axis) {}
    void writeXml(Qucs::XmlWriter *) {
        qWarning("Node::writeXml(): Nothing to write.");
    }
    void readXml(Qucs::XmlReader *) {
        qWarning("Node::readXml(): Nothing to read.");
    }
    void invokePropertiesDialog() { }
private:
    //! \brief List of components attached to this node.
    QList<Component*> m_components;

    //! \brief List of wires attached to this node
    QList<Wire*> m_wires;

    //! \brief Name of node
    QString m_name;

    /*!
     * \brief Component responsible for controling position of node.
     * \sa setController()
     */
    Component *m_controller;
};

#endif //NODE_H
