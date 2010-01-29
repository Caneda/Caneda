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
// along with this program; ifnot, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "validators.h"

#include "QRelaxNGvalidator.h"

namespace Qucs {

    static const char componentspath[] = "schema/qucscomponents.rng";
    static const char librarypath[] = "schema/qucslibrary.rng";

    /*!
     * \brief load all validators from a schema path
     * \note NULL is safe and in case of out of memory you could safely continue
     * \todo implement library
     */
    bool validators::load(const QString& path)
    {
        this->componentsschema = new QRelaxNGvalidator(path + componentspath);
        if(this->componentsschema == NULL) {
            return false;
        }
        //this->componentslibrary = new QRelaxNGvalidator(path + librarypath);
        return true;
    }

    //! \brief Default validators
    validators * validatorsdefaultInstance = NULL;

    //! \brief Return default validators
    validators * validators::defaultInstance() {
        if(validatorsdefaultInstance == NULL) {
            validatorsdefaultInstance = new validators();
        }
        return validatorsdefaultInstance;
    }

}
