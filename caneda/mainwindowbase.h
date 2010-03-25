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

#ifndef MAINWINDOWBASE_H
#define MAINWINDOWBASE_H

#include <QMainWindow>
#include <QTabWidget>

class QToolButton;

class TabWidgetPrivate : public QTabWidget
{
public:
    TabWidgetPrivate(QWidget *parent = 0);
};

class MainWindowBase : public QMainWindow
{
    Q_OBJECT;
public:
    MainWindowBase(QWidget *parent = 0);
    ~MainWindowBase();

    void addChildWidget(QWidget *widget);
    void removeChildWidget(QWidget *widget, bool deleteWidget = false);

    void addAsDockWidget(QWidget *w, const QString& title = "",
            Qt::DockWidgetArea area = Qt::LeftDockWidgetArea);

    QTabWidget* tabWidget() const { return m_tabWidget; }
    QWidget* currentWidget() const { return m_tabWidget->currentWidget(); }

signals:
    void currentWidgetChanged(QWidget *current, QWidget *prev);
    void closedWidget(QWidget *widget);

public Q_SLOTS:
    void closeTab(int index);

private Q_SLOTS:
    void emitWidgetChanged(int index);

private:
    void setupTabWidget();

    TabWidgetPrivate *m_tabWidget;
    QWidget *m_lastCurrentWidget;
};

#endif //MAINWINDOWBASE_H
