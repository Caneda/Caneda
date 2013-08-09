/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2013 by Pablo Daniel Pareja Obregon                       *
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

#ifndef SIDEBAR_BROWSER_H
#define SIDEBAR_BROWSER_H

#include <QTreeView>

namespace Caneda
{
    // Forward declarations
    class CLineEdit;
    class FilterProxyModel;
    class SidebarModel;

    class TreeView : public QTreeView
    {
        Q_OBJECT

    public:
        TreeView(QWidget *parent = 0);

        void startDrag(Qt::DropActions supportedActions);

    signals:
        void invalidAreaClicked(const QModelIndex &index);

        protected:
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);

        bool invalidPressed;
    };

    /*!
     * \brief This class implements the sidebar dockwidget with components to
     * be inserted in graphic documents.
     *
     * This class implements the sidebar dockwidget corresponding to the
     * LayoutContext, SchematicContext, and SymbolContext classes. It allows
     * previously generated components to be inserted in these type of
     * documents.
     *
     * The components depend on the final context class. In the LayoutContext,
     * for example, components are layout layers; in the SchematicContext,
     * components are electronic components; and in the SymbolContext,
     * components are painting items. All these components are inserted into
     * the currently opened document upon user double click.
     *
     * \sa LayoutContext, SchematicContext, SymbolContext, SidebarTextBrowser
     */
    class SidebarBrowser : public QWidget
    {
        Q_OBJECT

    public:
        SidebarBrowser(QWidget *parent = 0);
        ~SidebarBrowser();

        void plugLibrary(QString libraryName, QString category);
        void unPlugLibrary(QString libraryName, QString category);

        void plugItem(QString itemName, const QPixmap& itemPixmap, QString category);
        void plugItems(const QList<QPair<QString, QPixmap> > &items, QString category);

        QString currentComponent();

    signals:
        void itemClicked(const QString& item, const QString& category);
        void itemDoubleClicked(const QString& item, const QString& category);

    private Q_SLOTS:
        void filterTextChanged();
        void slotOnClicked(const QModelIndex& index);
        void slotOnDoubleClicked(const QModelIndex& index);

    private:
        SidebarModel *m_model;
        FilterProxyModel *m_proxyModel;
        TreeView *m_treeView;

        CLineEdit *m_filterEdit;

        QString m_currentComponent;
    };

} // namespace Caneda

#endif //SIDEBAR_BROWSER_H
