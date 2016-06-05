/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2013-2016 by Pablo Daniel Pareja Obregon                  *
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

#include "statehandler.h"

#include "actionmanager.h"
#include "graphicsscene.h"
#include "graphicsview.h"
#include "library.h"
#include "painting.h"
#include "portsymbol.h"
#include "wire.h"
#include "xmlutilities.h"

#include <QApplication>
#include <QClipboard>

namespace Caneda
{
    static bool areItemsEquivalent(GraphicsItem *a, GraphicsItem *b)
    {
        if (!a || !b) {
            return false;
        }
        if (a->type() != b->type()) {
            return false;
        }

        if (a->type() == GraphicsItem::ComponentType) {
            Component *ac = canedaitem_cast<Component*>(a);
            Component *bc = canedaitem_cast<Component*>(b);

            return ac->library() == bc->library() &&
                ac->name() == bc->name();
        }

        /*!
         * \todo Implement for other kinds of comparison required to compare
         * insertibles and toolbarInsertibles private members.
         */
        return false;
    }

    //! \brief Constructor.
    StateHandler::StateHandler(QObject *parent) : QObject(parent)
    {
        mouseAction = Caneda::Normal;
        paintingDrawItem = 0;
        focussedWidget = 0;
    }

    //! \copydoc MainWindow::instance()
    StateHandler* StateHandler::instance()
    {
        static StateHandler *instance = 0;
        if (!instance) {
            instance = new StateHandler();
        }
        return instance;
    }

    //! \brief Destructor.
    StateHandler::~StateHandler()
    {
        delete focussedWidget;
        delete paintingDrawItem;
        clearInsertibles();
    }

    void StateHandler::registerWidget(GraphicsView *widget)
    {
        GraphicsScene *scene = widget->graphicsScene();
        if (!scene) {
            qWarning() << Q_FUNC_INFO << "Widget doesn't have an associated scene";
            return;
        }
        if (!widgets.contains(widget)) {
            widgets << widget;
            connect(widget, SIGNAL(destroyed(QObject*)), SLOT(objectDestroyed(QObject*)));
            connect(widget, SIGNAL(focussedIn(GraphicsView*)), SLOT(updateFocussedWidget(GraphicsView*)));
        }

        if (!scenes.contains(scene)) {
            scenes << scene;
            connect(scene, SIGNAL(destroyed(QObject*)), SLOT(objectDestroyed(QObject*)));
        }
    }

    void StateHandler::unregisterWidget(GraphicsView *widget)
    {
        if (!widget) {
            return;
        }
        if (widgets.contains(widget)) {
            widgets.remove(widget);
            disconnect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
            disconnect(widget, SIGNAL(focussedIn(GraphicsView*)), this, SLOT(updateFocussedWidget(GraphicsView*)));
        }

        GraphicsScene *scene = widget->graphicsScene();
        if (scene && scenes.contains(scene)) {
            scenes.remove(scene);
            disconnect(scene, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
        }
    }

    //! \brief Toggles the normal select action on.
    void StateHandler::setNormalAction()
    {
        performToggleAction("select", true);
    }

    /*!
     * \brief Toogles the action perfomed.
     *
     * This method toggles the action corresponding to the sender, invoking the
     * performToggleAction(const QString&, bool) method, to takes care of
     * preserving the mutual exclusiveness off the checkable actions.
     *
     * While performToggleAction(const QString&, bool) is a general method
     * this method allows the direct connection to the toggled(bool) signal of
     * a QAction object.
     */
    void StateHandler::performToggleAction(bool on)
    {
        QAction *action = qobject_cast<QAction*>(sender());
        if(action) {
            performToggleAction(action->objectName(), on);
        }
    }

    /*!
     * \brief Toogles the action perfomed.
     *
     * This method toggles the action and calls the function pointed by
     * \a func if on is true. This method takes care to preserve the mutual
     * exclusiveness off the checkable actions.
     */
    void StateHandler::performToggleAction(const QString& actionName, bool on)
    {
        typedef void (GraphicsScene::*pActionFunc) (QList<GraphicsItem*>&);

        ActionManager *am = ActionManager::instance();

        QAction *action = am->actionForName(actionName);
        Caneda::MouseAction ma = am->mouseActionForAction(action);
        pActionFunc func = 0;

        if (actionName == "editDelete") {
            func = &GraphicsScene::deleteItems;
        }
        else if (actionName == "editRotate") {
            func = &GraphicsScene::rotateItems;
        }
        else if (actionName == "editMirrorX") {
            func = &GraphicsScene::mirrorXItems;
        }
        else if (actionName == "editMirrorY") {
            func = &GraphicsScene::mirrorYItems;
        }

        QList<QAction*> mouseActions = ActionManager::instance()->mouseActions();

        //toggling off any action switches normal select action "on"
        if(!on) {
            // Normal action can't be turned off through UI by clicking
            // the selct action again.
            setNormalAction();
            return;
        }

        //else part
        GraphicsScene *scene = 0;
        if (focussedWidget) {
            scene = focussedWidget->graphicsScene();
        }
        QList<QGraphicsItem*> selectedItems;
        if (scene) {
            selectedItems = scene->selectedItems();
        }

        do {
            if(!selectedItems.isEmpty() && func != 0) {
                QList<GraphicsItem*> funcable = filterItems<GraphicsItem>(selectedItems);

                if(funcable.isEmpty()) {
                    break;
                }

                (scene->*func)(funcable);

                // Turn off this action
                performToggleAction(action->objectName(), false);
                return;
            }
        } while(false); //For break

        // Just ensure all other action's are off.
        foreach(QAction *act, mouseActions) {
            if(act != action) {
                act->blockSignals(true);
                act->setChecked(false);
                act->blockSignals(false);
            }
        }

        QHash<QString, GraphicsItem*>::const_iterator it =
            toolbarInsertibles.begin();
        while (it != toolbarInsertibles.end()) {
            QAction *act = am->actionForName(it.key());
            act->blockSignals(true);
            act->setChecked(false);
            act->blockSignals(false);
            ++it;
        }

        if (actionName == "insertItem" && insertibles.size() == 1) {
            for (it = toolbarInsertibles.begin();
                    it != toolbarInsertibles.end(); ++it) {
                if (areItemsEquivalent(it.value(), insertibles.first())) {
                    QAction *act = am->actionForName(it.key());
                    act->blockSignals(true);
                    act->setChecked(true);
                    act->blockSignals(false);
                }
            }
        }

        // Ensure current action is on visibly
        action->blockSignals(true);
        action->setChecked(true);
        action->blockSignals(false);

        mouseAction = ma;

        foreach (GraphicsView *widget, widgets) {
            applyState(widget);
        }
    }

    /*!
     * \brief Insert an item based on its name and category.
     *
     * Get a component or painting based on the name and category. The
     * painting is processed in a special hardcoded way (with no libraries
     * involved). Some other "miscellaneous" items are hardcoded too. On the
     * other hand, components are loaded from existing libraries.
     *
     * \param item Item's name
     * \param category Item's category
     */
    void StateHandler::sidebarItemClicked(const QString& item,
            const QString& category)
    {
        // Clear old item first
        clearInsertibles();

        // Get a component or painting based on the name and category.
        GraphicsItem *qItem = 0;
        if(category == "Paint Tools" || category == "Layout Tools") {
            qItem = Painting::fromName(item);
        }
        else if(category == QObject::tr("Miscellaneous")) {
            // This must be repeated for each type of miscellaneous item,
            // for example ground, port symbols, etc.
            if(item == QObject::tr("Ground")) {
                qItem = new PortSymbol();
                PortSymbol *portSymbol = static_cast<PortSymbol*>(qItem);
                portSymbol->setLabel("Ground");
            }
            if(item == QObject::tr("Port Symbol")) {
                qItem = new PortSymbol();
            }
        }

        // If the item was not found in the fixed libraries, search for the
        // item in the dinamic loaded libraries ("Components" category).
        if(!qItem) {
            ComponentDataPtr data = LibraryManager::instance()->componentData(item, category);
            if(data.constData()) {
                qItem = new Component();
                Component *comp = static_cast<Component*>(qItem);
                comp->setComponentData(data);
            }
        }

        // Check if the item was successfully found and created
        if(qItem) {
            if(category == "Paint Tools" || category == "Layout Tools") {
                paintingDrawItem = static_cast<Painting*>(qItem);
                paintingDrawItem->setPaintingRect(QRectF(0, 0, 0, 0));
                performToggleAction("paintingDraw", true);
            }
            else {
                insertibles << qItem;
                performToggleAction("insertItem", true);
            }
        }
        else {
            setNormalAction();
        }
    }

    void StateHandler::insertToolbarComponent(const QString& sender, bool on)
    {
        GraphicsItem *item = toolbarInsertibles[sender];
        if (!on || !item) {
            setNormalAction();
            return;
        }

        clearInsertibles();
        insertibles << item->copy();
        performToggleAction("insertItem", true);
    }

    void StateHandler::handlePaste()
    {
        QClipboard *clipboard =  QApplication::clipboard();
        const QString text = clipboard->text();

        Caneda::XmlReader reader(text.toUtf8());

        while(!reader.atEnd()) {
            reader.readNext();

            if(reader.isStartElement() && reader.name() == "caneda") {
                break;
            }
        }

        if(reader.hasError() || !(reader.isStartElement() && reader.name() == "caneda")) {
            return;
        }

        if(!Caneda::checkVersion(reader.attributes().value("version").toString())) {
            return;
        }

        QList<GraphicsItem*> _items;
        while(!reader.atEnd()) {
            reader.readNext();

            if(reader.isEndElement()) {
                break;
            }

            if(reader.isStartElement()) {
                GraphicsItem *readItem = 0;
                if(reader.name() == "component") {
                    readItem = new Component();
                    readItem->loadData(&reader);
                }
                else if(reader.name() == "wire") {
                    readItem = new Wire(QPointF(10,10), QPointF(50,50));
                    readItem->loadData(&reader);
                }
                else if(reader.name() == "painting") {
                    QString name = reader.attributes().value("name").toString();
                    readItem = Painting::fromName(name);
                    readItem->loadData(&reader);
                }
                else if(reader.name() == "port") {
                    readItem = new PortSymbol();
                    readItem->loadData(&reader);
                }

                if(readItem) {
                    _items << readItem;
                }
            }
        }

        if (!_items.isEmpty()) {
            clearInsertibles();
            insertibles = _items;
            performToggleAction("insertItem", true);
        }
    }

    void StateHandler::objectDestroyed(QObject *object)
    {
        /*!
         * \todo HACK: Using static cast to convert QObject pointers to scene
         * and widget respectively. This might result in invalid pointers, but
         * the main purpose why we need them is just to remove the same from
         * the lists. Using of these pointers to access any method or variable
         * will result in ugly crash!!!
         */
        GraphicsScene *scene = static_cast<GraphicsScene*>(object);
        GraphicsView *widget = static_cast<GraphicsView*>(object);

        scenes.remove(scene);
        widgets.remove(widget);
    }

    void StateHandler::updateFocussedWidget(GraphicsView *widget)
    {
        focussedWidget = widget;
    }

    void StateHandler::applyCursor(GraphicsView *widget)
    {
        QCursor cursor;

        switch (mouseAction) {
            case Caneda::Wiring:
                cursor.setShape(Qt::CrossCursor);
                break;

            case Caneda::Deleting:
                cursor = QCursor(Caneda::icon("draw-eraser").pixmap(20));
                break;

            case Caneda::Rotating:
                cursor = QCursor(Caneda::icon("object-rotate-right").pixmap(20));
                break;

            case Caneda::MirroringX:
                cursor.setShape(Qt::SizeVerCursor);
                break;

            case Caneda::MirroringY:
                cursor.setShape(Qt::SizeHorCursor);
                break;

            case Caneda::ZoomingAreaEvent:
                cursor = QCursor(Caneda::icon("zoom-in").pixmap(20));
                break;

            case Caneda::PaintingDrawEvent:
                cursor.setShape(Qt::CrossCursor);
                break;

            case Caneda::InsertingItems:
                cursor.setShape(Qt::ClosedHandCursor);
                break;

            default:
                cursor.setShape(Qt::ArrowCursor);
        }

        widget->setCursor(cursor);
    }

    void StateHandler::applyState(GraphicsView *widget)
    {
        applyCursor(widget);

        GraphicsScene *scene = widget->graphicsScene();
        if (!scene) {
            return;
        }

        scene->setMouseAction(mouseAction);
        if (mouseAction == Caneda::InsertingItems) {
            if (!insertibles.isEmpty()) {
                QList<GraphicsItem*> copy;
                foreach (GraphicsItem *it, insertibles) {
                    copy << it->copy();
                }
                scene->beginInsertingItems(copy);
            }
        }
        else if (mouseAction == Caneda::PaintingDrawEvent) {
            if (paintingDrawItem) {
                Painting *copy = paintingDrawItem->copy();
                scene->beginPaintingDraw(copy);
            }
        }
    }

    void StateHandler::clearInsertibles()
    {
        foreach (GraphicsItem* item, insertibles) {
            if(item->scene()) {
                item->scene()->removeItem(item);
            }
            delete item;
        }

        insertibles.clear();
    }

} // namespace Caneda
