/***************************************************************************
 * Copyright 2010 Pablo Daniel Pareja Obregon                              *
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

#ifndef GITMANAGER_H
#define GITMANAGER_H

#include <QObject>

namespace Caneda
{
    class GitManager : public QObject
    {
        Q_OBJECT;

    public:
        GitManager();
        ~GitManager() {}

    private Q_SLOTS:
        void init();
        void status();
        void commit();
        void history();
        void revert();
    };

} // namespace Caneda

#endif //GITMANAGER_H
