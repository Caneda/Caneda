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

#include "documentviewmanager.h"
#include "iview.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTabBar>
#include <QWheelEvent>


namespace Caneda
{
    Tab::Tab(IView *view, QWidget *parent) : QWidget(parent)
    {
        addView(view);

        QHBoxLayout *layout = new QHBoxLayout(this);

        QSplitter *splitter = new QSplitter();
        splitter->addWidget(view->toWidget());

        layout->addWidget(splitter);
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
        return QString();
    }

    QIcon Tab::tabIcon() const
    {
        return QIcon();
    }

    void Tab::splitView(IView *view, IView *newView,
            Qt::Orientation splitOrientation)
    {
        QWidget *asWidget = view->toWidget();
        QSplitter *parentSplitter = qobject_cast<QSplitter*>(asWidget->parentWidget());

        Q_ASSERT_X(parentSplitter != 0, Q_FUNC_INFO,
                "Fix me: Parent widget of view not splitter");

        if (parentSplitter->orientation() != splitOrientation &&
                parentSplitter->count() == 1) {
            parentSplitter->setOrientation(splitOrientation);
        }

        if (parentSplitter->orientation() == splitOrientation) {
            parentSplitter->addWidget(newView->toWidget());
        } else {
            int index = parentSplitter->indexOf(asWidget);
            asWidget->setParent(0);

            QSplitter *newSplitter = new QSplitter(splitOrientation);
            newSplitter->addWidget(asWidget);
            newSplitter->addWidget(newView->toWidget());
            parentSplitter->insertWidget(index, newSplitter);
        }

        addView(newView);
    }

    void Tab::closeView(IView *view)
    {
        if (!m_views.contains(view)) {
            qDebug() << Q_FUNC_INFO << "View " << (void*)view << "doesn't exist"
                << "in this tab";
            return;
        }

        QWidget *asWidget = view->toWidget();
        QSplitter *parentSplitter = qobject_cast<QSplitter*>(asWidget->parentWidget());

        Q_ASSERT_X(parentSplitter != 0, Q_FUNC_INFO,
                "Fix me: Parent widget of view not splitter");

        asWidget->setParent(0);
        // Remember, we do not delete the view itself here. Its handled in
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

            Q_ASSERT_X(parentSplitter != 0, Q_FUNC_INFO,
                    "Fix me: Parent widget of splitter is neither Tab nor QSplitter");
        }

        if (removeThisTab) {
            TabWidget *tabWidget = qobject_cast<TabWidget*>(this);
            Q_ASSERT_X(tabWidget != 0, Q_FUNC_INFO,
                    "Tab's parent is not TabWidget");
            tabWidget->removeTab(tabWidget->indexOf(this));
            deleteLater();
        }
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
    }

    void Tab::closeEvent(QCloseEvent *event)
    {
        DocumentViewManager *manager = DocumentViewManager::instance();
        foreach (IView *view, m_views) {
            bool closed = manager->closeView(view);
            if (!closed) {
                event->ignore();
                return;
            }
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

        m_views.append(view);

        QObject *asObject = view->toWidget();
        connect(asObject, SIGNAL(focussedIn(IView*)), this,
                SLOT(onViewFocussedIn(IView*)));
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
        connect(tab, SIGNAL(tabInfoChanged(Tab*, QString, QIcon)), this,
                SLOT(updateTabInfo(Tab*, QString, QIcon)));
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


} // namespace Caneda
