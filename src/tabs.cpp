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

#include "tabs.h"

#include "actionmanager.h"
#include "documentviewmanager.h"
#include "global.h"
#include "idocument.h"
#include "iview.h"
#include "mainwindow.h"

#include <QDebug>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabBar>
#include <QToolBar>
#include <QWheelEvent>
#include <QUndoGroup>

namespace Caneda
{
    ViewContainer::ViewContainer(IView *view, QWidget *parent) :
        QWidget(parent),
        m_view(0),
        m_toolBar(0)
    {
        QVBoxLayout *layout = new QVBoxLayout;
        setLayout(layout);
        setContentsMargins(0, 0, 0, 0);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        setView(view);
    }


    ViewContainer::~ViewContainer()
    {
        // Don't let QWidget destructor destroy toolbar as the view might still exist.
        if (m_toolBar) {
            layout()->removeWidget(m_toolBar);
            m_toolBar->setParent(0);
        }
    }

    IView* ViewContainer::view() const
    {
        return m_view;
    }

    void ViewContainer::setView(IView *view)
    {
        QLayout *layout = this->layout();

        if (m_view) {
            QWidget *widget = m_view->toWidget();
            layout->removeWidget(widget);
            widget->setParent(0);

            disconnect(m_view, SIGNAL(focussedIn(IView*)), this,
                    SLOT(onViewFocusChange(IView*)));
            setToolBar(0);
        }

        m_view = view;

        if (m_view) {
            QWidget *widget = m_view->toWidget();
            widget->setParent(this);
            layout->addWidget(widget);

            connect(m_view, SIGNAL(focussedIn(IView*)), this,
                    SLOT(onViewFocusChange(IView*)));

            setToolBar(m_view->toolBar());
        }
    }

    void ViewContainer::setToolBar(QToolBar *toolbar)
    {
        QVBoxLayout *lay = qobject_cast<QVBoxLayout*>(layout());
        if (m_toolBar) {
            m_toolBar->setParent(0);
            lay->removeWidget(m_toolBar);
        }

        m_toolBar = toolbar;

        if (m_toolBar) {
            m_toolBar->setIconSize(QSize(16, 16));
            m_toolBar->setParent(this);
            lay->insertWidget(0, m_toolBar);
        }
    }

    void ViewContainer::onViewFocusChange(IView *view)
    {
        Q_UNUSED(view);
        //TODO: Uncomment this line after fixing ViewContainer::paintEvent
        //update();
    }

    void ViewContainer::paintEvent(QPaintEvent *event)
    {
        QWidget::paintEvent(event);
        bool hasFocus = m_view && m_view->toWidget()->hasFocus();
        Q_UNUSED(hasFocus);
        //TODO: Draw some focus helper.
    }

    Tab::Tab(IView *view, QWidget *parent) : QWidget(parent)
    {
        addView(view);

        QHBoxLayout *layout = new QHBoxLayout(this);

        QSplitter *splitter = new QSplitter();
        splitter->setContentsMargins(0, 0, 0, 0);
        splitter->addWidget(new ViewContainer(view));

        layout->addWidget(splitter);
        setContentsMargins(0, 0, 0, 0);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    Tab::~Tab()
    {
    }

    IView* Tab::activeView() const
    {
        return m_views.isEmpty() ? 0 : m_views.first();
    }

    QString Tab::tabText() const
    {
        IView *view = activeView();
        QString title;
        if (view) {
            title = view->document()->fileName();
        }

        if (title.isEmpty()) {
            title = tr("Untitled");
        } else {
            title = QFileInfo(title).fileName();
        }

        return title;
    }

    QIcon Tab::tabIcon() const
    {
        IView *view = activeView();
        if (view) {
            return view->document()->isModified() ? modifiedIcon() : unmodifiedIcon();
        }

        return QIcon();
    }

    void Tab::splitView(IView *view, IView *newView,
            Qt::Orientation splitOrientation)
    {
        QWidget *asWidget = view->toWidget();
        ViewContainer *parentContainer = qobject_cast<ViewContainer*>(asWidget->parentWidget());
        QSplitter *parentSplitter = qobject_cast<QSplitter*>(parentContainer->parentWidget());

        if (parentSplitter->orientation() != splitOrientation &&
                parentSplitter->count() == 1) {
            parentSplitter->setOrientation(splitOrientation);
        }

        if (parentSplitter->orientation() == splitOrientation) {
            parentSplitter->addWidget(new ViewContainer(newView));
        } else {
            int index = parentSplitter->indexOf(parentContainer);
            parentContainer->setParent(0);

            QSplitter *newSplitter = new QSplitter(splitOrientation);
            newSplitter->setContentsMargins(0, 0, 0, 0);
            newSplitter->addWidget(parentContainer);
            newSplitter->addWidget(new ViewContainer(newView));
            parentSplitter->insertWidget(index, newSplitter);
        }

        addView(newView);
        newView->toWidget()->setFocus();
    }

    void Tab::closeView(IView *view)
    {
        if (!m_views.contains(view)) {
            qDebug() << Q_FUNC_INFO << "View " << (void*)view << "doesn't exist"
                << "in this tab";
            return;
        }

        QWidget *asWidget = view->toWidget();
        ViewContainer *parentContainer = qobject_cast<ViewContainer*>(asWidget->parentWidget());
        QSplitter *parentSplitter = qobject_cast<QSplitter*>(parentContainer->parentWidget());

        parentContainer->setParent(0);
        parentContainer->setView(0);
        parentContainer->deleteLater();
        m_views.removeAll(view);
        // Remember, we do not delete the view itself here. It is handled in
        // DocumentViewManager.

        bool removeThisTab = false;

        // Get rid of unnecessary splitters recursively.
        while (1) {
            if (parentSplitter->count() > 0) break;

            QWidget *ancestor = parentSplitter->parentWidget();

            if (static_cast<QWidget*>(this) == ancestor) {
                removeThisTab = true;
                break;
            }

            parentSplitter->setParent(0);
            parentSplitter->deleteLater();

            parentSplitter = qobject_cast<QSplitter*>(ancestor);
        }

        if (removeThisTab) {
            QStackedWidget *stackedWidget = qobject_cast<QStackedWidget*>(parentWidget());

            TabWidget *tabWidget = qobject_cast<TabWidget*>(stackedWidget->parentWidget());
            tabWidget->removeTab(tabWidget->indexOf(this));
            deleteLater();
        }
    }

    void Tab::replaceView(IView *oldView, IView *newView)
    {
        if (!m_views.contains(oldView)) {
            qDebug() << Q_FUNC_INFO << "View " << (void*)oldView << "doesn't exist"
                << "in this tab";
            return;
        }

        QWidget *asWidget = oldView->toWidget();
        ViewContainer *parentContainer = qobject_cast<ViewContainer*>(asWidget->parentWidget());
        m_views.removeAll(oldView);

        parentContainer->setView(newView);

        addView(newView);
        newView->toWidget()->setFocus();
    }

    void Tab::onViewFocussedIn(IView *view)
    {
        // Apply LRU algo by pushing the least recently used
        // view to the front of the list.
        int i = m_views.indexOf(view);
        if (i >= 0 && i < m_views.size()) {
            m_views.takeAt(i);
            m_views.insert(0, view);
        }

        emit tabInfoChanged(this);
    }

    void Tab::onDocumentChanged(IDocument *document)
    {
        emit tabInfoChanged(this);
    }

    void Tab::onStatusBarMessage(const QString &message)
    {
        emit statusBarMessage(this, message);
    }

    void Tab::closeEvent(QCloseEvent *event)
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        QList<IDocument*> documents;
        QSet<IDocument*> processedDocuments;
        foreach (IView *view, m_views) {
            IDocument *document = view->document();

            if (processedDocuments.contains(document)) {
                continue;
            }

            processedDocuments << document;

            documents << document;
        }

        if (!documents.isEmpty()) {
            bool status = manager->saveDocuments(documents);
            if (!status) {
                event->ignore();
                return;
            }
        }

        const bool askForSave = false;
        while (1) {
            if (m_views.isEmpty()) break;

            IView *view = m_views.first();
            manager->closeView(view, askForSave);
        }

        event->accept();
    }

    void Tab::addView(IView *view)
    {
        if (m_views.contains(view)) {
            qDebug() << Q_FUNC_INFO << "View " << (void*)view
                << " is already added.";
            return;
        }

        m_views.insert(0, view);

        connect(view, SIGNAL(focussedIn(IView*)), this,
                SLOT(onViewFocussedIn(IView*)));
        connect(view->document(), SIGNAL(documentChanged(IDocument*)),
                this, SLOT(onDocumentChanged(IDocument*)));
        connect(view, SIGNAL(statusBarMessage(const QString &)), this,
                SLOT(onStatusBarMessage(const QString &)));

        emit tabInfoChanged(this);
    }

    QIcon Tab::modifiedIcon() const
    {
        static QIcon modifiedIcon;
        if (modifiedIcon.isNull()) {
            modifiedIcon = QIcon(Caneda::bitmapDirectory() + "modified.png");
        }
        return modifiedIcon;
    }

    QIcon Tab::unmodifiedIcon() const
    {
        static QIcon unmodifiedIcon;
        if (unmodifiedIcon.isNull()) {
            unmodifiedIcon = QIcon(Caneda::bitmapDirectory() + "unmodified.png");
        }
        return unmodifiedIcon;
    }

    class TabBarPrivate : public QTabBar
    {
    public:
        TabBarPrivate(QWidget *parent=0l) : QTabBar(parent)
        {
            setDrawBase(true);
        }
        ~TabBarPrivate() {}

    protected:
        void wheelEvent(QWheelEvent *ev)
        {
            if(count() > 1) {
                int current = currentIndex();
                if(ev->delta() < 0) {
                    current = (current + 1) % count();
                }
                else {
                    current--;
                    if(current < 0 ) {
                        current = count() - 1;
                    }
                }
                setCurrentIndex(current);
            }
        }
    };

    TabWidget::TabWidget(QWidget *parent) : QTabWidget(parent)
    {
        setTabBar(new TabBarPrivate(this));
        connect(this, SIGNAL(currentChanged(int)), this,
                SLOT(updateTabInfo()));
        connect(this, SIGNAL(tabCloseRequested(int)), this,
                SLOT(onTabCloseRequested(int)));
    }

    QList<Tab*> TabWidget::tabs() const
    {
        QList<Tab*> tabs;
        for (int i = 0; i < count(); ++i) {
            Tab *tab = qobject_cast<Tab*>(widget(i));
            if (!tab) {
                qDebug() << Q_FUNC_INFO << "Consists of tab that isn't a Tab";
            }
            tabs << tab;
        }
        return tabs;
    }

    void TabWidget::addTab(Tab *tab)
    {
        insertTab(-1, tab);
    }

    void TabWidget::insertTab(int index, Tab *tab)
    {
        QTabWidget::insertTab(index, tab, tab->tabIcon(), tab->tabText());
        connect(tab, SIGNAL(tabInfoChanged(Tab*)), this, SLOT(updateTabInfo()));
        connect(tab, SIGNAL(statusBarMessage(Tab*, const QString&)), this,
                SLOT(onStatusBarMessage(Tab*, const QString&)));


        IView *view = tab->activeView();
        if (!view) {
            return;
        }

        IDocument *document = view->document();
        if (!document) {
            return;
        }

        MainWindow *mw = MainWindow::instance();
        mw->m_undoGroup->addStack(document->undoStack());
    }

    Tab* TabWidget::currentTab() const
    {
        return qobject_cast<Tab*>(currentWidget());
    }

    void TabWidget::setCurrentTab(Tab *tab)
    {
        setCurrentWidget(tab);
    }

    void TabWidget::highlightView(IView *view)
    {
        QWidget *asWidget = view->toWidget();
        if (!asWidget) return;

        QWidget *parentWidget = asWidget->parentWidget();
        Tab *parentTab = 0;

        while (parentWidget) {
            parentTab = qobject_cast<Tab*>(parentWidget);
            if (parentTab) break;

            parentWidget = parentWidget->parentWidget();
        }

        if (!parentTab) return;

        setCurrentTab(parentTab);
        asWidget->setFocus();
    }

    void TabWidget::closeView(IView *view)
    {
        QWidget *asWidget = view->toWidget();
        if (!asWidget) return;

        QWidget *parentWidget = asWidget->parentWidget();
        Tab *parentTab = 0;

        while (parentWidget) {
            parentTab = qobject_cast<Tab*>(parentWidget);
            if (parentTab) break;

            parentWidget = parentWidget->parentWidget();
        }

        if (!parentTab) return;

        parentTab->closeView(view);
    }

    void TabWidget::replaceView(IView *oldView, IView *newView)
    {
        Tab *tab = tabForView(oldView);
        tab->replaceView(oldView, newView);
    }

    Tab* TabWidget::tabForView(IView *view) const
    {
        QWidget *widget = view->toWidget();

        Tab *tab = 0;
        while (1) {
            if (!widget) {
                break;
            }

            tab = qobject_cast<Tab*>(widget->parentWidget());
            if (tab) {
                break;
            }

            widget = widget->parentWidget();
        }

        return tab;
    }

    void TabWidget::updateTabInfo()
    {
        int index = currentIndex();
        if (index < 0 || index >= count()) {
            return;
        }

        Tab *tab = currentTab();

        setTabIcon(index, tab->tabIcon());
        setTabText(index, tab->tabText());

        MainWindow *mw = MainWindow::instance();
        mw->updateTitle();

        IView *view = tab->activeView();
        if (!view) {
            return;
        }

        IDocument *document = view->document();
        if (!document) {
            return;
        }

        mw->action("editCut")->setEnabled(document->canCut());
        mw->action("editCopy")->setEnabled(document->canCopy());
        mw->action("editPaste")->setEnabled(document->canPaste());
        mw->action("editUndo")->setEnabled(document->canUndo());
        mw->action("editRedo")->setEnabled(document->canRedo());

        mw->m_undoGroup->setActiveStack(document->undoStack());
    }

    void TabWidget::onStatusBarMessage(Tab *tab, const QString &message)
    {
        int index = indexOf(tab);
        if (index == currentIndex()) {
            emit statusBarMessage(message);
        }
    }

    void TabWidget::onTabCloseRequested(int index)
    {
        QWidget *w = widget(index);
        w->close();
    }

} // namespace Caneda
