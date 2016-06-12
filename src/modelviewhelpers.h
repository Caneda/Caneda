/***************************************************************************
 * Copyright (C) 2016 by Pablo Daniel Pareja Obregon                       *
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

#ifndef MODEL_VIEW_HELPERS_H
#define MODEL_VIEW_HELPERS_H

#include <QFileIconProvider>
#include <QSortFilterProxyModel>

namespace Caneda
{
    /*!
     * \brief The FilterProxyModel class helps in filtering an items model in a
     * tree like structure, using the text of a QLineEdit.
     *
     * This class is used to be able to filter the items present in any model
     * column (categories are in the first columns of the tree, and the items
     * are in succesive columns). The QSortFilterProxyModel doesn't allow
     * multicolumn filtering, hence it must be subclassed for those cases where
     * it's needed.
     */
    class FilterProxyModel : public QSortFilterProxyModel
    {
    public:
        explicit FilterProxyModel(QObject *parent = 0);

        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

        //! \brief Method to prevent from becoming rootless while filtering
        void setSourceRoot(const QModelIndex &sourceRoot) { m_sourceRoot = sourceRoot; }

      private:
        QModelIndex m_sourceRoot;
    };

    /*!
     * \brief The FileFilterProxyModel class helps in filtering a file model in
     * a tree like structure, using the text of a QLineEdit.
     *
     * The main difference of the class with respect to the FilterProxyModel is
     * that it keeps the folders in the model while filtering, allowing for a
     * different user interface. It is mainly used in the SidebarTextBrowser
     * class for displaying (filtering) available sidebar items.
     *
     * \sa SidebarTextBrowser, FilterProxyModel
     */
    class FileFilterProxyModel : public QSortFilterProxyModel
    {
        Q_OBJECT

    public:
        explicit FileFilterProxyModel(QObject *parent = 0);

        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    };

    /*!
     * \brief Reimplementation of QFileIconProvider to allow for custom icons
     * in view widgets with a model derived from QFileSystemModel.
     *
     * This class reimplements a simple QFileIconProvider that uses custom
     * icons for Caneda's files mimetypes.
     *
     * \sa QFileSystemModel, QFileIconProvider, QuickOpen, FolderBrowser
     */
    class IconProvider: public QFileIconProvider
    {
    public:
        explicit IconProvider();

        virtual QIcon icon(IconType type) const;
        virtual QIcon icon(const QFileInfo & info) const;
    };

} // namespace Caneda

#endif //MODEL_VIEW_HELPERS_H
