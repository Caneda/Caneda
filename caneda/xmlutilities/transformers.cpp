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

#include "transformers.h"

#include "QRelaxNGvalidator.h"

#define NULL_PTR (0)

namespace Qucs {

    static const char componentsvgpath[] = "xslt/componentsvg.xsl";

    //! \brief load all transformer from a root path
    bool transformers::load(const QString& path)
    {
        this->componentsvgtransformer = new QXsltTransformer(path + componentsvgpath);
        if(this->componentsvgtransformer == NULL_PTR) {
            return false;
        }
        return true;
    }

    //! \brief Default transformers
    transformers * transformersdefaultInstance = NULL_PTR;

    //! \brief Return default transformers
    transformers * transformers::defaultInstance()
    {
        if(transformersdefaultInstance == NULL_PTR) {
            transformersdefaultInstance = new transformers();
        }
        return transformersdefaultInstance;
    }

}
