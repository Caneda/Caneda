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

#ifndef CANEDA_TABCONTENT_H
#define CANEDA_TABCONTENT_H

#include <QTabWidget>

namespace Caneda
{
    // Forward declarations.
    class IDocument;
    class IView;
    class Manager;

    class Tab: public QWidget
    {
    Q_OBJECT
    public:
        Tab(IView *view, QWidget *parent = 0);
        ~Tab();

        IView* activeView() const;

        QString tabText() const;
        QIcon tabIcon() const;

        void closeView(IView *view);
        void splitView(IView *view, IView *newView,
                Qt::Orientation splitOrientation);

    public Q_SLOTS:
        void onViewFocussedIn(IView *view);
        void onDocumentChanged(IDocument *document);
        void onStatusBarMessage(const QString &message);

    Q_SIGNALS:
        void tabInfoChanged(Tab *tab);
        void statusBarMessage(Tab *tab, const QString &message);

    protected:
        void closeEvent(QCloseEvent *event);

    private:
        void addView(IView *view);
        QIcon modifiedIcon() const;
        QIcon unmodifiedIcon() const;
        void updateTitle();

        QList<IView*> m_views;

        friend class Manager;
    };

    class TabWidget : public QTabWidget
    {
        Q_OBJECT
    public:
        TabWidget(QWidget *parent = 0);
        QList<Tab*> tabs() const;

        Tab* tabForView(IView *view) const;

        void addTab(Tab *tab);
        void insertTab(int index, Tab *tab);

        Tab* currentTab() const;
        void setCurrentTab(Tab *tab);

        void highlightView(IView *view);
        void closeView(IView *view);

    Q_SIGNALS:
        void statusBarMessage(const QString &message);

    private Q_SLOTS:
        void updateTabInfo(Tab *tab);
        void onStatusBarMessage(Tab *tab, const QString &message);
        void onTabCloseRequested(int index);
    };

} // namespace Caneda

#endif // CANEDA_TABCONTENT_H
