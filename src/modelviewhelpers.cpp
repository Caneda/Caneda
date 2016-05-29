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

#include "modelviewhelpers.h"

#include "global.h"
#include "icontext.h"

#include <QFileSystemModel>

namespace Caneda
{
    /*************************************************************************
     *                          FilterProxyModel                             *
     *************************************************************************/
    //! \brief Constructor.
    FilterProxyModel::FilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
    {
    }

    /*!
     * \brief Returns true if the item should be included in the model
     * (filtered); false otherwise.
     *
     * This method must be reimplemented from QSortFilterProxyModel to be able
     * to perform multicolumn filtering. It returns true if the item in the row
     * indicated by the given sourceRow and sourceParent should be included in
     * the model; otherwise returns false.
     *
     * Special care must also be taken to avoid filtering the parent or root
     * item.
     */
    bool FilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);

        // Make sure the root is always accepted, or we become rootless
        if(m_sourceRoot.isValid() && index0 == m_sourceRoot) {
            return true;
        }

        // Do bottom to top filtering
        if(sourceModel()->hasChildren(index0)) {
            for(int i=0; i < sourceModel()->rowCount(index0); ++i) {
                if(filterAcceptsRow(i, index0)) {
                    return true;
                }
            }
        }

        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }

    /*************************************************************************
     *                         FileFilterProxyModel                          *
     *************************************************************************/
    //! \brief Constructor.
    FileFilterProxyModel::FileFilterProxyModel(QObject *parent) :
        QSortFilterProxyModel(parent)
    {
    }

    /*!
     * \brief Returns true if the item should be included in the model
     * (filtered); false otherwise.
     *
     * This method must be reimplemented from QSortFilterProxyModel to be able
     * to perform multicolumn filtering. It returns true if the item in the row
     * indicated by the given sourceRow and sourceParent should be included in
     * the model; otherwise returns false.
     */
    bool FileFilterProxyModel::filterAcceptsRow(int sourceRow,
                                                const QModelIndex &sourceParent) const
    {
        QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
        QFileSystemModel *fileModel = static_cast<QFileSystemModel*>(sourceModel());

        if(fileModel != NULL && fileModel->isDir(index0)) {
            return true;
        }

        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }

    /*************************************************************************
     *                            IconProvider                               *
     *************************************************************************/
    //! \brief Constructor.
    IconProvider::IconProvider() : QFileIconProvider()
    {
    }

    //! \brief Returns an icon set for the given type.
    QIcon IconProvider::icon(QFileIconProvider::IconType type) const
    {
        if(type == QFileIconProvider::Folder) {
            return Caneda::icon("folder");
        }
        else {
            return Caneda::icon("unknown");
        }
    }

    //! \brief Returns an icon for the file described by info.
    QIcon IconProvider::icon(const QFileInfo &info) const
    {
        if(info.isDir()) {
            return icon(QFileIconProvider::Folder);
        }
        else {
            IContext *context = SchematicContext::instance();
            if(context->supportedSuffixes().contains(info.suffix())) {
                return Caneda::icon("application-x-caneda-schematic");
            }

            context = SymbolContext::instance();
            if(context->supportedSuffixes().contains(info.suffix())) {
                return Caneda::icon("application-x-caneda-symbol");
            }

            context = LayoutContext::instance();
            if(context->supportedSuffixes().contains(info.suffix())) {
                return Caneda::icon("application-x-caneda-layout");
            }

            context = SimulationContext::instance();
            if(context->supportedSuffixes().contains(info.suffix())) {
                return Caneda::icon("application-x-spice-simulation-raw");
            }

            context = TextContext::instance();
            if(context->supportedSuffixes().contains(info.suffix())) {
                return Caneda::icon("text-plain");
            }

            return icon(QFileIconProvider::File);
        }

        return QIcon();
    }

} // namespace Caneda
