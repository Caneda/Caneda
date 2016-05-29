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

namespace Caneda
{
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
