/***************************************************************************
 * Copyright (C) 2008 by Bastien Roucaries <bastien.roucaries@gmail.com>   *
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

#ifndef VALIDATORS_H
#define VALIDATORS_H

#include "QRelaxNGvalidator.h"

#include <QString>

namespace Caneda
{
    //! \brief This class preload all the xml validator used for parsing xml file
    class validators
    {
    public:
        //! \brief schema used for components validation
        QRelaxNGvalidator * components() {
            return this->componentsschema;
        }
        //! \brief schema used for library validation
        QRelaxNGvalidator * library() {
            return this->libraryschema;
        }

        //! \brief Default constructor
        validators() {
            componentsschema = NULL;
            libraryschema = NULL;
        }

        //! \brief Default destructor
        ~validators() {
            delete this->componentsschema;
            delete this->libraryschema;
        }

        bool load(const QString& path);
        static validators * defaultInstance();
    private:
        //! \brief schema used for components validation
        QRelaxNGvalidator * componentsschema;
        //! \brief schema used for library validation
        QRelaxNGvalidator * libraryschema;
    };

} // namespace Caneda

#endif //VALIDATORS_H
