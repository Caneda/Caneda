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

namespace Caneda
{
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
