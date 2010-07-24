/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "schematicstatehandler.h"

#include "actionmanager.h"
#include "global.h"
#include "library.h"
#include "schematicscene.h"
#include "schematicwidget.h"
#include "settings.h"
#include "singletonowner.h"
#include "undocommands.h"

#include "paintings/painting.h"

#include "xmlutilities/xmlutilities.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QPointer>
#include <QSet>

namespace Caneda
{
    struct SchematicStateHandlerPrivate
    {
        SchematicStateHandlerPrivate() {
            mouseAction = SchematicScene::Normal;
            paintingDrawItem = 0;

        }

        ~SchematicStateHandlerPrivate() {
            delete paintingDrawItem;
            clearInsertibles();
        }

        void updateToolbarInsertibles() {
            LibraryLoader *loader = LibraryLoader::instance();
            toolbarInsertibles.insert("insGround",
                    loader->newComponent("Ground", 0, "Passive"));
            toolbarInsertibles.insert("insPort",
                    loader->newComponent("Port", 0, "Passive"));
        }

        void clearInsertibles() {
            foreach (SchematicItem* item, insertibles) {
                if (item->scene()) {
                    item->scene()->removeItem(item);
                }
                delete item;
            }
            insertibles.clear();
        }

        SchematicScene::MouseAction mouseAction;
        QList<SchematicItem*> insertibles;
        Painting *paintingDrawItem;

        QSet<SchematicScene*> scenes;
        QSet<SchematicWidget*> widgets;

        QPointer<SchematicWidget> focussedWidget;
        QHash<QString, SchematicItem*> toolbarInsertibles;
    };

    static bool areItemsEquivalent(SchematicItem *a, SchematicItem *b)
    {
        if (!a || !b) {
            return false;
        }
        if (a->type() != b->type()) {
            return false;
        }

        if (a->isComponent()) {
            Component *ac = canedaitem_cast<Component*>(a);
            Component *bc = canedaitem_cast<Component*>(b);

            return ac->library() == bc->library() &&
                ac->name() == bc->name();
        }

        // Implement for other kinds of comparison required to compare
        // insertibles and toolbarInsertibles of
        // SchematicStateHandlerPrivate class.
        return false;
    }

    SchematicStateHandler::SchematicStateHandler(QObject *parent) : QObject(parent)
    {
        d = new SchematicStateHandlerPrivate;

        LibraryLoader *loader = LibraryLoader::instance();
        connect(loader, SIGNAL(basicLibrariesLoaded()), this, SLOT(slotUpdateToolbarInsertibles()));
    }

    SchematicStateHandler* SchematicStateHandler::instance()
    {
        static SchematicStateHandler *instance = 0;
        if (!instance) {
            instance = new SchematicStateHandler(SingletonOwner::instance());
        }
        return instance;
    }

    SchematicStateHandler::~SchematicStateHandler()
    {
        delete d;
    }

    void SchematicStateHandler::registerWidget(SchematicWidget *widget)
    {
        SchematicScene *scene = widget->schematicScene();
        if (!scene) {
            qWarning() << Q_FUNC_INFO << "Widget doesn't have an associated scene";
            return;
        }
        if (!d->widgets.contains(widget)) {
            d->widgets << widget;
            connect(widget, SIGNAL(destroyed(QObject*)), SLOT(slotOnObjectDestroyed(QObject*)));
            connect(widget, SIGNAL(focussedIn(SchematicWidget*)),
                    SLOT(slotUpdateFocussedWidget(SchematicWidget*)));
        }

        if (!d->scenes.contains(scene)) {
            d->scenes << scene;
            connect(scene, SIGNAL(destroyed(QObject*)), SLOT(slotOnObjectDestroyed(QObject*)));
            connect(scene, SIGNAL(rotateInvokedWhileInserting()),
                    SLOT(slotRotateInsertibles()));
            connect(scene, SIGNAL(mirrorInvokedWhileInserting()),
                    SLOT(slotMirrorInsertibles()));
        }
    }

    void SchematicStateHandler::unregisterWidget(SchematicWidget *widget)
    {
        if (!widget) {
            return;
        }
        if (d->widgets.contains(widget)) {
            d->widgets.remove(widget);
            disconnect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(slotOnObjectDestroyed(QObject*)));
            disconnect(widget, SIGNAL(focussedIn(SchematicWidget*)), this,
                    SLOT(slotUpdateFocussedWidget(SchematicWidget*)));
        }

        SchematicScene *scene = widget->schematicScene();
        if (scene && d->scenes.contains(scene)) {
            d->scenes.remove(scene);
            disconnect(scene, SIGNAL(destroyed(QObject*)), this, SLOT(slotOnObjectDestroyed(QObject*)));
            disconnect(scene, SIGNAL(rotateInvokedWhileInserting()), this,
                    SLOT(slotRotateInsertibles()));
            disconnect(scene, SIGNAL(mirrorInvokedWhileInserting()), this,
                    SLOT(slotMirrorInsertibles()));
        }
    }

    void SchematicStateHandler::slotSidebarItemClicked(const QString& item,
            const QString& category)
    {
        if (category == "Paint Tools") {
            if (d->paintingDrawItem) {
                // Clear old item first.
                if (d->paintingDrawItem->scene()) {
                    d->paintingDrawItem->scene()->removeItem(d->paintingDrawItem);
                }
                delete d->paintingDrawItem;
            }

            d->paintingDrawItem = Painting::fromName(item);
            if (!d->paintingDrawItem) {
                slotSetNormalAction();
            } else {
                d->paintingDrawItem->setPaintingRect(QRectF(0, 0, 0, 0));
                slotPerformToggleAction("paintingDraw", true);
            }
        } else {
            d->clearInsertibles();
            LibraryLoader *libLoader = LibraryLoader::instance();
            SchematicItem *qItem = libLoader->newComponent(item, 0, category);
            if (!qItem) {
                slotSetNormalAction();
            } else {
                d->insertibles << qItem;
                slotPerformToggleAction("insertItem", true);
            }
        }
    }

    void SchematicStateHandler::slotHandlePaste()
    {
        const QString text = qApp->clipboard()->text();

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

        QList<SchematicItem*> _items;
        while(!reader.atEnd()) {
            reader.readNext();

            if(reader.isEndElement()) {
                break;
            }

            if(reader.isStartElement()) {
                SchematicItem *readItem = 0;
                if(reader.name() == "component") {
                    readItem = Component::loadComponentData(&reader, 0);
                }
                else if(reader.name() == "wire") {
                    readItem = Wire::loadWireData(&reader, 0);
                }
                else if(reader.name() == "painting")  {
                    readItem = Painting::loadPainting(&reader, 0);
                }

                if(readItem) {
                    _items << readItem;
                }
            }
        }

        if (_items.isEmpty() == false) {
            d->clearInsertibles();
            d->insertibles = _items;
            slotPerformToggleAction("insertItem", true);
        }
    }

    void SchematicStateHandler::slotRotateInsertibles()
    {
        if (d->mouseAction != SchematicScene::InsertingItems) {
            qDebug() << Q_FUNC_INFO << "Wrong mouse action mode!";
            return;
        }

        // Utilize code available in undo command :-P
        RotateItemsCmd cmd(d->insertibles, Caneda::Clockwise);
        cmd.redo();

        // Now start a fresh insertion
        slotPerformToggleAction("insertItem", true);
    }

    void SchematicStateHandler::slotMirrorInsertibles()
    {
        if (d->mouseAction != SchematicScene::InsertingItems) {
            qDebug() << Q_FUNC_INFO << "Wrong mouse action mode!";
            return;
        }

        // Utilize code available in undo command :-P
        MirrorItemsCmd cmd(d->insertibles, Qt::XAxis);
        cmd.redo();

        // Now start a fresh insertion
        slotPerformToggleAction("insertItem", true);
    }

    void SchematicStateHandler::slotInsertToolbarComponent(const QString& sender,
            bool on)
    {
        SchematicItem *item = d->toolbarInsertibles[sender];
        if (!on || !item) {
            slotSetNormalAction();
            return;
        }

        d->clearInsertibles();
        d->insertibles << item->copy();
        slotPerformToggleAction("insertItem", true);
    }

    void SchematicStateHandler::slotOnObjectDestroyed(QObject *object)
    {
        //HACK: Using static cast to convert QObject pointers to scene and widget
        //      respectively. This might result in invalid pointers, but the main
        //      purpose why we need them is just to remove the same from the lists.
        //
        //      Using of these pointers to access any method or variable will result
        //      in ugly crash!!
        SchematicScene *scene = static_cast<SchematicScene*>(object);
        SchematicWidget *widget = static_cast<SchematicWidget*>(object);


        d->scenes.remove(scene);
        d->widgets.remove(widget);
    }

    void SchematicStateHandler::slotUpdateFocussedWidget(SchematicWidget *widget)
    {
        d->focussedWidget = widget;
    }

    /*!
     * \brief Toogles the action perfomed.
     *
     * This method toggles the action and calls the function pointed by
     * \a func if on is true. This method takes care to preserve the mutual
     * exclusiveness off the checkable actions.
     */
    void SchematicStateHandler::slotPerformToggleAction(const QString& sender, bool on)
    {
        typedef void (SchematicScene::*pActionFunc) (QList<SchematicItem*>&, const Caneda::UndoOption);

        ActionManager *am = ActionManager::instance();

        Action *action = am->actionForName(sender);
        SchematicScene::MouseAction ma = am->mouseActionForAction(action);
        pActionFunc func = 0;

        if (sender == "editDelete") {
            func = &SchematicScene::deleteItems;
        } else if (sender == "editRotate") {
            func = &SchematicScene::rotateItems;
        } else if (sender == "editMirror") {
            func = &SchematicScene::mirrorXItems;
        } else if (sender == "editMirrorY") {
            func = &SchematicScene::mirrorYItems;
        } else if (sender == "editActivate") {
            func = &SchematicScene::toggleActiveStatus;
        }

        Action *norm = am->actionForName("select");

        QList<Action*> mouseActions = ActionManager::instance()->mouseActions();

        //toggling off any action switches normal select action "on"
        if(!on) {
            // Normal action can't be turned off through UI by clicking
            // the selct action again.
            slotSetNormalAction();
            return;
        }

        //else part
        SchematicScene *scene = 0;
        if (d->focussedWidget.isNull() == false) {
            scene = d->focussedWidget->schematicScene();
        }
        QList<QGraphicsItem*> selectedItems;
        if (scene) {
            selectedItems = scene->selectedItems();
        }

        do {
            if(!selectedItems.isEmpty() && func != 0) {
                QList<SchematicItem*> funcable = filterItems<SchematicItem>(selectedItems);

                if(funcable.isEmpty()) {
                    break;
                }

                (scene->*func)(funcable, Caneda::PushUndoCmd);

                // Turn off this action
                slotPerformToggleAction(action->objectName(), false);
                return;
            }
        } while(false); //For break

        // Just ensure all other action's are off.
        foreach(Action *act, mouseActions) {
            if(act != action) {
                act->blockSignals(true);
                act->setChecked(false);
                act->blockSignals(false);
            }
        }

        QHash<QString, SchematicItem*>::const_iterator it =
            d->toolbarInsertibles.begin();
        while (it != d->toolbarInsertibles.end()) {
            Action *act = am->actionForName(it.key());
            act->blockSignals(true);
            act->setChecked(false);
            act->blockSignals(false);
            ++it;
        }

        if (sender == "insertItem" && d->insertibles.size() == 1) {
            for (it = d->toolbarInsertibles.begin();
                    it != d->toolbarInsertibles.end(); ++it) {
                if (areItemsEquivalent(it.value(), d->insertibles.first())) {
                    Action *act = am->actionForName(it.key());
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

        d->mouseAction = ma;
        applyStateToAllWidgets();
    }

    void SchematicStateHandler::slotSetNormalAction()
    {
        slotPerformToggleAction("select", true);
    }

    void SchematicStateHandler::slotUpdateToolbarInsertibles()
    {
        d->updateToolbarInsertibles();
    }

    void SchematicStateHandler::applyCursor(SchematicWidget *widget)
    {
        static QString bitmapPath = Caneda::bitmapDirectory();
        QCursor cursor;

        switch (d->mouseAction) {
            case SchematicScene::Wiring:
                cursor.setShape(Qt::CrossCursor);
                break;

            case SchematicScene::Deleting:
                cursor = QCursor(QIcon::fromTheme("draw-eraser", QIcon(bitmapPath + "draw-eraser.png")).pixmap(20));
                break;

            case SchematicScene::Rotating:
                cursor = QCursor(QIcon::fromTheme("object-rotate-left", QIcon(bitmapPath + "object-rotate-left.png")).pixmap(20));
                break;

            case SchematicScene::MirroringX:
                cursor.setShape(Qt::SizeVerCursor);
                break;

            case SchematicScene::MirroringY:
                cursor.setShape(Qt::SizeHorCursor);
                break;

            case SchematicScene::ZoomingAreaEvent:
                cursor = QCursor(QIcon::fromTheme("zoom-in", QIcon(bitmapPath +  "zoom-in.png")).pixmap(20));
                break;

            case SchematicScene::PaintingDrawEvent:
                cursor.setShape(Qt::CrossCursor);
                break;

            case SchematicScene::InsertingItems:
                cursor.setShape(Qt::ClosedHandCursor);
                break;

            default:
                cursor.setShape(Qt::ArrowCursor);
        }

        widget->setCursor(cursor);
    }

    void SchematicStateHandler::applyState(SchematicWidget *widget)
    {
        applyCursor(widget);
        SchematicScene *scene = widget->schematicScene();
        if (!scene) {
            return;
        }

        scene->setMouseAction(d->mouseAction);
        if (d->mouseAction == SchematicScene::InsertingItems) {
            if (!d->insertibles.isEmpty()) {
                QList<SchematicItem*> copy;
                foreach (SchematicItem *it, d->insertibles) {
                    copy << it->copy(scene);
                }
                scene->beginInsertingItems(copy);
            }
        } else if (d->mouseAction == SchematicScene::PaintingDrawEvent) {
            if (d->paintingDrawItem) {
                Painting *copy = d->paintingDrawItem->copy();
                scene->beginPaintingDraw(copy);
            }
        }
    }

    void SchematicStateHandler::applyStateToAllWidgets()
    {
        foreach (SchematicWidget *widget, d->widgets) {
            applyState(widget);
        }
    }

} // namespace Caneda
