/***************************************************************************
 * Copyright (C) 2014 by Pablo Daniel Pareja Obregon                       *
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

#ifndef SIDEBAR_WEB_BROWSER_H
#define SIDEBAR_WEB_BROWSER_H

#include <QTreeView>

// Forward declarations
class QFileSystemModel;

namespace Caneda
{
    // Forward declarations
    class CLineEdit;
    class FileFilterProxyModel;

    /*!
     * \brief This class implements the sidebar dockwidget with templates to be
     * inserted in text documents.
     *
     * This class implements the sidebar dockwidget corresponding to the
     * TextContext class. It allows previously generated templates to be
     * inserted in text documents.
     *
     * The templates may correspond to spice, vhdl, verilog, or any other type
     * of code structures. These structures or templates are inserted into the
     * currently opened document upon user double click.
     *
     * \sa TextContext, SidebarBrowser
     */
    class SidebarWebBrowser : public QWidget
    {
        Q_OBJECT

    public:
        SidebarWebBrowser(QWidget *parent = 0);
        ~SidebarWebBrowser();

    private Q_SLOTS:
        void filterTextChanged();
        void slotOnDoubleClicked(const QModelIndex& index);

    private:
        QFileSystemModel *m_fileModel;
        FileFilterProxyModel *m_proxyModel;
        QTreeView *m_treeView;

        CLineEdit *m_filterEdit;
    };

} // namespace Caneda

#endif //SIDEBAR_WEB_BROWSER_H
