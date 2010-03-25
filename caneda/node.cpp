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

#include "node.h"
#include "schematicscene.h"
#include "undocommands.h"
#include "wire.h"

#include <QDebug>
#include <QPainter>

#include <cmath>

/*!
 * \brief Radius of node
 * Radius of node in pixels
 * \todo Allow custumization. Is this really needed ?
 */
const qreal Node::Radius = 3.0;
const qreal Node::ZValue = 1.0;
const QRectF Node::BoundRect = QRectF(-Node::Radius, -Node::Radius,
        2*Node::Radius, 2*Node::Radius);
QPainterPath Node::Shape;
/*!
 * \brief Compute euclidian distance between two points
 * \todo Why not use
 * Qpoint d = p1 - p2;
 * return hypot(d.x(),d.y());
 */
inline qreal distance(const QPointF& p1, const QPointF& p2)
{
    qreal dx = p1.x() - p2.x();
    qreal dy = p1.y() - p2.y();
    return std::sqrt((dx*dx)+(dy*dy));
}

/*!
 * \brief Construct a new node
 *
 * Construct a new node named by name. A node does not react to mouse
 * and is always drawn before wires and components.
 * Node must be in the foreground of component and wires.
 */
Node::Node(const QString& name,SchematicScene *scene) : QucsItem(0,scene)
{
    setName(name);
    // Ensure flags is zero so that no useless checks are made on node.
    setFlags(0);
    setAcceptedMouseButtons(0);
    setZValue(Node::ZValue);
    m_controller = 0;
    if(Node::Shape.isEmpty()) {
        Node::Shape.addEllipse(Node::BoundRect);
    }
}

//! \brief Destructor
Node::~Node()
{
}

/*!
 * \brief Add a component to node
 * Add a component and update node
 * \param comp component to add
 * \todo Comp is const
 */
void Node::addComponent(Component *comp)
{
    if(!m_components.contains(comp)) {
        m_components << comp;
        update();
    }
}

/*!
 * \brief Remove a component to node
 *
 * Remove a component and update node
 * \param comp component to delete
 * \todo Why update()?
 * \todo Comp is const
 */
void Node::removeComponent(Component* comp)
{
    int index =  m_components.indexOf(comp);
    if(index != -1) {
        m_components.removeAt(index);
        update();
    }
}

//!\brief Returns a list of selected components which belong to this node.
QList<Component*> Node::selectedComponents() const
{
    QList<Component*> selCom;
    foreach(Component *c, m_components) {
        if(c->isSelected()) {
            selCom << c;
        }
    }
    return selCom;
}

//!\brief Return if all the components belonging to this node are selected./
bool Node::areAllComponentsSelected() const
{
    QList<Component*>::const_iterator it = m_components.constBegin();
    const QList<Component*>::const_iterator end = m_components.constEnd();
    for(; it != end; ++it) {
        if(!((*it)->isSelected())) {
            return false;
        }
    }
    return true;
}

/*!
 * \brief Add all components from node n to this node
 *
 * Add all components from node n to this node and update node
 * \param n: origin node
 */
void Node::addAllComponentsFrom(const Node *n)
{
    foreach(Component *c, n->m_components) {
        if(!m_components.contains(c)) {
            m_components << c;
        }
    }
    update();
}

/*!
 * \brief Add a wire to this node
 *
 * Add a wire to this node and update node
 * \param w wire to add
 * \todo w is const....
 */
void Node::addWire(Wire *w)
{
    if(!m_wires.contains(w)) {
        m_wires << w;
        //signal repaint since node changes its
        //appearance(not necessarily though)
        update();
    }
}

/*!
 * \brief Remove a wire to node
 *
 * Remove a wire to node and update node
 * \param w wire to remove
 * \todo w const...
 * \todo why update
 */
void Node::removeWire(Wire *w)
{
    int index = m_wires.indexOf(w);
    if(index != -1) {
        m_wires.removeAt(index);
        update();
    }
}

//! \brief Return the list of selected wire belonging to this node.
QList<Wire*> Node::selectedWires() const
{
    QList<Wire*> selWires;
    foreach(Wire *w, m_wires) {
        if(w->isSelected()) {
            selWires << w;
        }
    }
    return selWires;
}

/*!
 * \brief Add all wire of node n to this node
 * \param n node to copy
 * \todo n const...
 */
void Node::addAllWiresFrom(const Node *n)
{
    if(!n->m_wires.isEmpty()) {
        foreach(Wire *w, n->m_wires) {
            if(!m_wires.contains(w)) {
                m_wires << w;
            }
        }
        update();
    }
}


/*!
 * \brief Draw a node
 *
 * Draw a node ie a little circle
 * \todo Color should be customisable
 */
void Node::paint(QPainter* p,const QStyleOptionGraphicsItem *o, QWidget *w)
{
    Q_UNUSED(o);
    Q_UNUSED(w);
    p->setPen(Qt::darkRed);
    if(!isOpen()) {
        p->setBrush(QBrush(Qt::cyan, Qt::SolidPattern));
    }
    p->drawEllipse(boundingRect());
}

/*!
 * \brief Test if object collide this another object
 * \param other another object
 * \return true if this item collides with other; otherwise returns false.
 */
bool Node::collidesWithItem(QGraphicsItem *other) const
{
    //Check if the other item is also a node.
    Node *port = qucsitem_cast<Node*>(other);
    //If other is not a node call base implementation.
    if(!port) {
        return QGraphicsItem::collidesWithItem(other);
    }
    //Else use this faster way to determine collision.
    qreal dist = distance(pos(),port->pos());

    return (dist <= (2. * Node::Radius));
}

/*!
 * \brief Set name of node
 * \todo Take care of updating wirelabels.
 */
void Node::setName(const QString& name)
{
    m_name = name;
}
