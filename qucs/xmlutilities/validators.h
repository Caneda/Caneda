// Copyright (C) 2008 bastien ROUCARIES <bastien.roucaries+qucs@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef VALIDATORS_H
#define VALIDATORS_H

#include "QRelaxNGvalidator.h"

#include <QString>

namespace Qucs {

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
        };

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

}

#endif
