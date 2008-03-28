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
#include "qucs-tools/global.h"

#include "xmlutilities/validators.h"

#include <QtGui/QTabBar>
#include <QtGui/QDockWidget>
#include <QtGui/QToolButton>
#include <QtGui/QWheelEvent>

class TabBarPrivate : public QTabBar
{
   public:
      TabBarPrivate(QWidget *parent=0l) : QTabBar(parent)
      {
         setDrawBase(true);
      }
      ~TabBarPrivate() {}

   protected:
      void wheelEvent( QWheelEvent *ev )
      {
         if ( count() > 1 ) {
            int current = currentIndex();
            if ( ev->delta() < 0 )
               current = (current + 1) % count();
            else {
               current--;
               if ( current < 0 )
                  current = count() - 1;
            }
            setCurrentIndex( current );
         }
      }
};

TabWidgetPrivate:: TabWidgetPrivate(QWidget *parent) : QTabWidget(parent)
{
   setTabBar(new TabBarPrivate(this));
}

MainWindowBase::MainWindowBase(QWidget *parent) : QMainWindow(parent)
{
   setupTabWidget();
   setCentralWidget(m_tabWidget);
}

MainWindowBase::~MainWindowBase()
{
}

void MainWindowBase::addChildWidget(QWidget *widget)
{
   m_tabWidget->addTab(widget, widget->windowIcon(), widget->windowTitle());
   if(m_tabWidget->count() == 1) {
      emit currentWidgetChanged(widget, 0);
   }
   if(m_tabCloseButton->isHidden())
      m_tabCloseButton->show();
}

void MainWindowBase::removeChildWidget(QWidget *widget, bool deleteWidget)
{
   int index = m_tabWidget->indexOf(widget);
   if ( index >= 0 ) {
      QWidget *w = m_tabWidget->widget(index);

      if(w->close()) {
         emit closedWidget(w);
         if(deleteWidget)
            w->deleteLater();
      }
      else {
         return;
      }

      m_tabWidget->removeTab(index);
   }

   if( m_tabWidget->count() == 0 )
      m_tabCloseButton->hide();
}

void MainWindowBase::addAsDockWidget(QWidget *w, const QString& title, Qt::DockWidgetArea area)
{
   QDockWidget *dw = new QDockWidget(title);
   dw->setWidget(w);
   addDockWidget(area, dw);
}

void MainWindowBase::closeCurrentTab()
{
   int index = m_tabWidget->currentIndex();
   if (index >= 0)
      removeChildWidget(m_tabWidget->widget(index), true);
}

void MainWindowBase::setupTabWidget()
{
   m_tabWidget = new TabWidgetPrivate(this);
   m_tabWidget->setFocusPolicy(Qt::NoFocus);
   m_tabCloseButton = new QToolButton(m_tabWidget);
   m_tabCloseButton->setIcon(QIcon(Qucs::bitmapDirectory() + "close_tab.png"));
   m_tabCloseButton->adjustSize();
   m_tabCloseButton->hide();
   m_tabWidget->setCornerWidget(m_tabCloseButton, Qt::TopRightCorner);

   connect(m_tabCloseButton, SIGNAL(clicked()), this, SLOT(closeCurrentTab()));
   connect(m_tabWidget, SIGNAL(currentChanged ( int)), this, SLOT(emitWidgetChanged( int )));

   m_lastCurrentWidget = m_tabWidget->currentWidget();
}

void MainWindowBase::emitWidgetChanged(int index)
{
   QWidget *current = m_tabWidget->widget(index);
   int lastIndex = m_tabWidget->indexOf(m_lastCurrentWidget);
   QWidget *prev = m_tabWidget->widget(lastIndex);

   emit currentWidgetChanged(current, prev);
   m_lastCurrentWidget = current;
}
