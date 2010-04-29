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

#include "mainwindowbase.h"

#include "caneda-tools/global.h"
#include "tabs.h"
#include "xmlutilities/transformers.h"
#include "xmlutilities/validators.h"

#include <QDockWidget>
#include <QToolButton>
#include <QWheelEvent>

namespace Caneda
{

    //! Constructor
    MainWindowBase::MainWindowBase(QWidget *parent) : QMainWindow(parent)
    {
        setupTabWidget();
        setCentralWidget(m_tabWidget);
    }

    //! Destructor
    MainWindowBase::~MainWindowBase()
    {
    }

    void MainWindowBase::addChildWidget(QWidget *widget)
    {
        static_cast<QTabWidget*>(m_tabWidget)->addTab(widget,
                widget->windowIcon(), widget->windowTitle());
        if(m_tabWidget->count() == 1) {
            emit currentWidgetChanged(widget, 0);
        }
    }

    void MainWindowBase::removeChildWidget(QWidget *widget, bool deleteWidget)
    {
        int index = m_tabWidget->indexOf(widget);
        if(index >= 0) {
            QWidget *w = m_tabWidget->widget(index);

            if(w->close()) {
                emit closedWidget(w);
                if(deleteWidget) {
                    w->deleteLater();
                }
            }
            else {
                return;
            }

            m_tabWidget->removeTab(index);
        }
    }

    void MainWindowBase::addAsDockWidget(QWidget *w, const QString& title,
            Qt::DockWidgetArea area)
    {
        QDockWidget *dw = new QDockWidget(title);
        dw->setWidget(w);
        addDockWidget(area, dw);
    }

    TabWidget* MainWindowBase::tabWidget() const
    {
        return m_tabWidget;
    }

    QWidget* MainWindowBase::currentWidget() const
    {
        return m_tabWidget->currentWidget();
    }

    void MainWindowBase::closeTab(int index)
    {
        removeChildWidget(m_tabWidget->widget(index), true);
    }

    void MainWindowBase::closeAllTabs()
    {
        while(tabWidget()->count() > 0) {
            removeChildWidget(m_tabWidget->widget(m_tabWidget->currentIndex()), true);
        }
    }

    void MainWindowBase::setupTabWidget()
    {
        m_tabWidget = new TabWidget(this);
        m_tabWidget->setFocusPolicy(Qt::NoFocus);
        m_tabWidget->setTabsClosable(true);
        m_tabWidget->setMovable(true);

        connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(emitWidgetChanged(int)));

        m_lastCurrentWidget = m_tabWidget->currentWidget();
    }

    void MainWindowBase::emitWidgetChanged(int index)
    {
        QWidget *current = m_tabWidget->widget(index);
        int lastIndex = m_tabWidget->indexOf(m_lastCurrentWidget);
        QWidget *prev = m_tabWidget->widget(lastIndex);

        emit currentWidgetChanged(current, prev);
        m_lastCurrentWidget = current;

        if (current) {
            current->setFocus();
        }
    }

} // namespace Caneda
