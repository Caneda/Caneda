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

#ifndef QXMLSTREAMREADEREXT_H
#define QXMLSTREAMREADEREXT_H

#include "QRelaxNGvalidator.h"
#include "QXsltTransformer.h"

#include <QByteArray>
#include <QXmlStreamReader>

namespace Caneda
{
    class QXmlStreamReaderExt : public QXmlStreamReader
    {
    public:
        //! \brief Default constructor
        QXmlStreamReaderExt (): QXmlStreamReader()
        {
            this->bless();
        }
        /* construct from a file name */
        /*
           QXmlStreamReaderExt (const QString &name, const QRelaxNGvalidator * schema = NULL,
           const QXsltTransformer * xslt = NULL, bool usexinclude = true);
           */

        //! construct from memory
        QXmlStreamReaderExt (const QByteArray &array,
                const QRelaxNGvalidator * schema = NULL,
                const QXsltTransformer *xslt = NULL, bool usexinclude = true);
        virtual ~QXmlStreamReaderExt();

        //! \brief Get a const copy of transformed xml
        const char * constData() {
            return data.constData();
        }

    protected:
        //! \brief The xml file loaded in memory
        QByteArray data;
        //! \brief The xml file output (temporary)
        void * xmlout;

    private:
        //! \brief Default and safe initialisation
        void bless()
        {
            this->data = QByteArray();
            this->xmlout = NULL;
        };
        /* finalize construction */
        void finalize(const void *docvoid,
                const QRelaxNGvalidator * schema,
                const QXsltTransformer *xslt,
                bool usexinclude);
    };

} // namespace Caneda

#endif //QXMLSTREAMREADEREXT_H
