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

#include "icontext.h"

namespace Caneda
{
    /*!
     * \class IContext
     *
     * This class provides an interface for a context which is used by IDocument
     * and IView. This class also provides objects like toolbar, statusbar etc which
     * is specific to particular context.
     *
     * The context class can also be used to host functionalites shared by all
     * views and documents of same type.
     *
     * \see IDocument, IView
     */

    IContext::IContext(QObject *parent) : QObject(parent)
    {

    }

    IContext::~IContext()
    {

    }

    void IContext::init()
    {

    }

    QToolBar* IContext::toolBar()
    {
        return 0;
    }

    QWidget* IContext::statusBarWidget()
    {
        return 0;
    }

    QWidget* IContext::sideBarWidget(Caneda::SideBarRole role)
    {
        return 0;
    }


} // namespace Caneda
