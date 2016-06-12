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
#include "documentviewmanager.h"
#include "graphicsscene.h"
#include "graphicsview.h"
#include "iview.h"
#include "library.h"
#include "painting.h"
#include "portsymbol.h"
#include "wire.h"
#include "xmlutilities.h"

#include <QApplication>
#include <QClipboard>

namespace Caneda
{
    //! \brief Constructor.
    StateHandler::StateHandler(QObject *parent) : QObject(parent)
    {
        mouseAction = Caneda::Normal;
        paintingDrawItem = 0;
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
        delete paintingDrawItem;
        clearInsertibles();
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
     * performToggleAction(const QString&, bool) method. This method takes care
     * of preserving the mutual exclusiveness of the checkable actions.
     *
     * While performToggleAction(const QString&, bool) is the method that
     * actually performs the toggle action, this method allows the direct
     * connection to the toggled(bool) signal of a QAction object.
     *
     * \sa performToggleAction(const QString&, bool)
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
     * This method toggles the selected action and calls its associated method
     * if on is true. This method also takes care of preserving the mutual
     * exclusiveness of the checkable actions.
     *
     * \sa performToggleAction(bool)
     */
    void StateHandler::performToggleAction(const QString& actionName, bool on)
    {
        // When turning off any action (other than select), the normal action
        // (select) must be turned on.
        if(!on) {
            setNormalAction();
            return;
        }

        // Set the current mouse action.
        ActionManager *am = ActionManager::instance();
        QAction *action = am->actionForName(actionName);
        mouseAction = am->mouseActionForAction(action);

        // Get the function associated to the selected action.
        typedef void (GraphicsScene::*pActionFunc) (QList<GraphicsItem*>&);
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

        // Get selected items.
        GraphicsView *view = 0;
        GraphicsScene *scene = 0;
        QList<QGraphicsItem*> selectedItems;

        DocumentViewManager *manager = DocumentViewManager::instance();
        IView *currentView = manager->currentView();

        if(currentView) {
            view = qobject_cast<GraphicsView*>(currentView->toWidget());
        }
        if(view) {
            scene = view->graphicsScene();
        }
        if(scene) {
            selectedItems = scene->selectedItems();
        }

        // If there is any selected item, apply the action to the selected
        // items, and deselect the action.
        if(!selectedItems.isEmpty() && func != 0) {
            QList<GraphicsItem*> funcable = filterItems<GraphicsItem>(selectedItems);

            if(!funcable.isEmpty()) {
                (scene->*func)(funcable);
                // Turn off this action
                performToggleAction(action->objectName(), false);
                return;
            }
        }

        // Turn off all other actions.
        QList<QAction*> mouseActions = ActionManager::instance()->mouseActions();

        foreach(QAction *act, mouseActions) {
            if(act != action) {
                act->blockSignals(true);
                act->setChecked(false);
                act->blockSignals(false);
            }
        }

        // Turn on this action.
        action->blockSignals(true);
        action->setChecked(true);
        action->blockSignals(false);

        // Set the state and cursor corresponding to this action in all views.
        foreach(IView *iview, manager->views()) {
            GraphicsView* view = qobject_cast<GraphicsView*>(iview->toWidget());
            if(view) {
                applyState(view);
            }
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
    void StateHandler::insertItem(const QString& item, const QString& category)
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

    /*!
     * \brief This method handles the paste action by reading the clipboard and
     * inserting (if found) the corresponding items.
     */
    void StateHandler::paste()
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

        QList<GraphicsItem*> items;
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
                    items << readItem;
                }
            }
        }

        if (!items.isEmpty()) {
            clearInsertibles();
            insertibles = items;
            performToggleAction("insertItem", true);
        }
    }

    //! \brief Apply the cursor of the current state to the given view.
    void StateHandler::applyCursor(GraphicsView *view)
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

        view->setCursor(cursor);
    }

    //! \brief Apply the current state to the given view.
    void StateHandler::applyState(GraphicsView *view)
    {
        applyCursor(view);

        GraphicsScene *scene = view->graphicsScene();
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

    //! \brief Clear the list of insertible items.
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
