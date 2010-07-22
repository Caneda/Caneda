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

#ifndef TRANSFORMERS_H
#define TRANSFORMERS_H

#include "QXsltTransformer.h"

#include <QString>

namespace Caneda
{
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

} // namespace Caneda

#endif //TRANSFORMERS_H
