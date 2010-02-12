/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "propertyitem.h"

#include "component.h"
#include "propertygroup.h"
#include "qucsmainwindow.h"
#include "schematicscene.h"
#include "schematicview.h"
#include "undocommands.h"

#include <QApplication>
#include <QFontMetricsF>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPointF>
#include <QStyleOptionGraphicsItem>
#include <QtDebug>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>

/*!
 * \brief Constructor.
 *
 * \param propName The name of property.
 * \param propMap The reference of property map. The property is fetched from here.
 * \param scene The schematic scene to which this item should belong.
 */
PropertyItem::PropertyItem(const QString &propName , SchematicScene *scene) :
    m_propertyName(propName), m_staticText(m_propertyName), m_edited(false)
{
    m_staticText.append(" = ");
    if(m_staticText.startsWith("label", Qt::CaseInsensitive)) {
        m_staticText = "";
    }
    setTextInteractionFlags(Qt::TextEditorInteraction);
    setFlags(ItemIsMovable | ItemIsFocusable);

    //caculate the pos of static text initially.
    calculatePos();
    setAcceptsHoverEvents(false);
    if(scene) {
        scene->addItem(this);
    }
    m_edited = false;
}

//! \brief Returns the bounds of the item.
QRectF PropertyItem::boundingRect() const
{
    QRectF bounds = QGraphicsTextItem::boundingRect();
    bounds.setLeft(m_staticPos.x());
    return bounds;
}

//! \brief Returns the shape of the item.
QPainterPath PropertyItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

/*!
 * \brief Set font and calculate the static position again.
 * \note This hides the base implementation as base method isn't virtual!
 */
void PropertyItem::setFont(const QFont& f)
{
    QGraphicsTextItem::setFont(f);
    calculatePos();
}

/*!
 * \brief Updates visual display of property value.
 * \note This method is key method to alter the visual text of property. Call
 * it wherever the property changes.
 */
void PropertyItem::updateValue()
{
    if(!component()) {
        qDebug() << "PropertyItem::updateValue() : Component is null!";
        return;
    }
    QString newValue = component()->property(m_propertyName).toString();
    if(newValue.isEmpty()) {
        newValue = " ";
    }
    setPlainText(newValue);
}

//! \brief Event filter used to avoid interfence of shortcut events while edit.
bool PropertyItem::eventFilter(QObject* object, QEvent* event)
{
    bool condition = (event->type() == QEvent::Shortcut ||
            event->type() == QEvent::ShortcutOverride) &&
        !object->inherits("QGraphicsView");
    if(condition == true) {
        event->accept();
        return true;
    }
    return false;
}

/*!
 * \brief Draws the the text item to painter.
 *
 * The static part of text is drawn explicitly and the remaining part
 * is drawn by the base \a QGraphicsTextItem
 */
void PropertyItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *o,
        QWidget *widget)
{
    //Draw static part of text.
    painter->setFont(font());
    painter->setBrush(Qt::NoBrush);
    //FIXME: Hardcoded color.
    painter->setPen(QPen(Qt::black,0));
    painter->drawText(m_staticPos, m_staticText);

    // Remove const ness of option.
    QStyleOptionGraphicsItem *option = const_cast<QStyleOptionGraphicsItem*>(o);

    // Save the values before changing.
    QStyle::State savedState = option->state;
    QRectF savedExposedRect = option->exposedRect;

    if(option->exposedRect.left() < 0) {
        option->exposedRect.setLeft(0);
    }
    // Clear the selection and focus flags to prevent the focus rect being drawn
    option->state &= ~QStyle::State_Selected;
    option->state &= ~QStyle::State_HasFocus;

    // Paint the remaining part of text using base method.
    QGraphicsTextItem::paint(painter, option, widget);

    // Restore changed values.
    option->state = savedState;
    option->exposedRect = savedExposedRect;

    //Now manually draw the focus rect.
    if(option->state & QStyle::State_HasFocus) {
        const qreal pad = 1.0 / 2;

        const qreal penWidth = 0; // cosmetic pen

        const QColor fgcolor = option->palette.windowText().color();
        const QColor bgcolor(fgcolor.red()   > 127 ? 0 : 255,
                fgcolor.green() > 127 ? 0 : 255,
                fgcolor.blue()  > 127 ? 0 : 255);

        painter->setPen(QPen(bgcolor, penWidth, Qt::SolidLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(QGraphicsTextItem::boundingRect().
                adjusted(pad, pad, -pad, -pad));

        painter->setPen(QPen(fgcolor, penWidth, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(QGraphicsTextItem::boundingRect().
                adjusted(pad, pad, -pad, -pad));
    }
}

/*!
 * \brief Alter the default reaction to event.
 *
 * This takes care of sending only required event to the
 * parent group rather than the default implementation which sends all the
 * events. This is a bit a hacky kind of implementation.
 */
bool PropertyItem::sceneEvent(QEvent *event)
{
    if(!group()) {
        return QGraphicsTextItem::sceneEvent(event);
    }

    //Eat some unnecessary events
    switch (event->type()) {
        case QEvent::GraphicsSceneDragEnter:
        case QEvent::GraphicsSceneDragMove:
        case QEvent::GraphicsSceneDragLeave:
        case QEvent::GraphicsSceneDrop:
        case QEvent::GraphicsSceneHoverEnter:
        case QEvent::GraphicsSceneHoverMove:
        case QEvent::GraphicsSceneHoverLeave:
        case QEvent::InputMethod:
        case QEvent::GraphicsSceneWheel:
            event->ignore();
            return true;
        default: ;
    };

    return QGraphicsTextItem::sceneEvent(event);
}

//! \brief Unselects the other selected items on mouse press.
void PropertyItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(scene()) {
        // Unselect other items once focus is received
        foreach(QGraphicsItem *item, scene()->selectedItems()) {
            if(item != this) {
                item->setSelected(false);
            }
        }
    }
    QGraphicsTextItem::mousePressEvent(event);
}

void PropertyItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsTextItem::mouseMoveEvent(event);
}

void PropertyItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsTextItem::mouseReleaseEvent(event);
}

void PropertyItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsTextItem::mouseDoubleClickEvent(event);
}

//! \brief Installs an event filter after calling base method.
void PropertyItem::focusInEvent(QFocusEvent *event)
{
    qApp->installEventFilter(this);
    m_edited = false;
    connect(document(), SIGNAL(contentsChanged()), SLOT(textChanged()));

    QGraphicsTextItem::focusInEvent(event);
}

/*!
 * \brief Focus out event handler.
 *
 * This updates the geometry of parameter group, validates
 * the text and also clears the selection.
 */
void PropertyItem::focusOutEvent(QFocusEvent *event)
{
    if(m_edited) {
        updateValue();
    }

    qApp->removeEventFilter(this);

    // Clear the text selection
    QTextCursor c = textCursor();
    c.clearSelection();
    setTextCursor(c);
    disconnect(document(), SIGNAL(contentsChanged()), this,
            SLOT(textChanged()));

    QGraphicsTextItem::focusOutEvent(event);
}

//! \brief Clears focus if escape key pressed. Otherwise calls base method.
void PropertyItem::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Escape) {
        clearFocus();
        return;
    }
    QVariant oldProperty(component()->property(m_propertyName));
    if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        if(m_edited &&
                component()->setProperty(m_propertyName, QVariant(toPlainText()))) {
            updateGroupGeometry();
            m_edited = false;
            QVariant newProperty(component()->property(m_propertyName));
            PropertiesGroup *parentGroup = static_cast<PropertiesGroup*>(group());
            Q_ASSERT(parentGroup);
            SchematicScene *schScene = parentGroup->schematicScene();
            schScene->undoStack()->push(new PropertyChangeCmd(m_propertyName,
                        newProperty, oldProperty, component()));
        }
        clearFocus();
        return;
    }

    m_edited = true;
    QGraphicsTextItem::keyPressEvent(e);
}

//! \brief Calculates the position of name part of property.
void PropertyItem::calculatePos()
{
    static const int magicNumberAdjust = 2;
    QFontMetricsF fm(font());
    QPointF temp = QPointF(-fm.width(m_staticText), fm.ascent() +
            magicNumberAdjust);
    prepareGeometryChange();
    m_staticPos = temp;
}

/*!
 * \brief Checks whether the given \a event can be sent to parent group or not
 * The method checks whether the current mouse position is in
 * the static part of area and if so returns true in case of mouse press.
 * Otherwise it checks for buttonDownPos for the same check.
 */
bool PropertyItem::isSendable(QGraphicsSceneMouseEvent *event) const
{
    if(event->type() == QEvent::GraphicsSceneMousePress) {
        return event->pos().x() < 0.0;
    }
    return event->buttonDownPos(Qt::LeftButton).x() < 0.0;
}

/*!
 * \brief Updates the geometry the parent group.
 * It first queries for the PropertiesGroup type and then
 * forcefully updates its geometry.
 */
void PropertyItem::updateGroupGeometry() const
{
    PropertiesGroup *propGroup = static_cast<PropertiesGroup*>(group());
    if(propGroup) {
        propGroup->forceUpdate();
    }
}

//! \brief Slot to keep track of change of data.
void PropertyItem::textChanged()
{
    m_edited = true;
    disconnect(document(), SIGNAL(contentsChanged()), this, SLOT(textChanged()));
}
