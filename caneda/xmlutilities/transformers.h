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

#ifndef TRANSFORMERS_H
#define TRANSFORMERS_H

#include "QXsltTransformer.h"

#include <QString>

namespace Qucs {
    //! \brief This class preload all the xml validator used for parsing xml file
    class transformers
    {
    public:
        //! \brief schema used for components validation
        QXsltTransformer * componentsvg() {
            return this->componentsvgtransformer;
        }

        //! \brief Default constructor
        transformers() {
            componentsvgtransformer = NULL;
        }

        //! \brief Default destructor
        ~transformers() {
            delete this->componentsvgtransformer;
        }

        bool load(const QString& path);
        static transformers * defaultInstance();

    private:
        //! \brief schema used for components svg
        QXsltTransformer * componentsvgtransformer;
    };
}

#endif
