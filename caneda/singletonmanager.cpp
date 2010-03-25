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

#include "singletonmanager.h"

#include "actionmanager.h"
#include "library.h"
#include "schematicstatehandler.h"
#include "svgitem.h"

SingletonManager::SingletonManager(QObject *parent)
    : QObject(parent),
    m_actionManager(0),
    m_libraryLoader(0),
    m_schematicStateHandler(0),
    m_svgPainter(0)
{
}

SingletonManager::~SingletonManager()
{
}

ActionManager* SingletonManager::actionManager()
{
    if (!m_actionManager) {
        m_actionManager = new ActionManager(this);
    }
    return m_actionManager;
}

LibraryLoader* SingletonManager::libraryLoader()
{
    if (!m_libraryLoader) {
        m_libraryLoader = new LibraryLoader(this);
    }
    return m_libraryLoader;
}

SchematicStateHandler* SingletonManager::schematicStateHandler()
{
    if (!m_schematicStateHandler) {
        m_schematicStateHandler = new SchematicStateHandler(this);
    }
    return m_schematicStateHandler;
}

SvgPainter* SingletonManager::svgPainter()
{
    if (!m_svgPainter) {
        m_svgPainter = new SvgPainter(this);
    }
    return m_svgPainter;
}

SingletonManager* SingletonManager::instance()
{
    static SingletonManager* sm = new SingletonManager();
    return sm;
}
