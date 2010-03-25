// Copyright (C) 2008 ROUCARIES Bastien <roucaries.bastien+qucs@gmail.com>
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
//**************************************************************************
//

/*!
 * \file QXmlStreamReaderExt.cpp
 * \author ROUCARIES Bastien
 * \brief Implement xml parser extending QXmlStreamReader class
 */


#include "QXmlStreamReaderExt.h"

#include <QDebug>
#include <QFile>

extern "C" {
#include <libxml/parser.h>
#include <libxml/relaxng.h>
#include <libxml/xinclude.h>
}

#include <iostream>


namespace Qucs {
    /*!
     * \brief Construct a XML reader from a filename
     * \param name[in]: Name of the file
     * \param schema[in]: Schema in order to control file (NULL is no schema)
     * \param xslt[in]: xslt file
     * \todo Check ifwe need tonative
     */
#if 0
    QXmlStreamReaderExt::QXmlStreamReaderExt ( const QString &name,
            const QRelaxNGvalidator * schema,
            const QXsltTransformer *xslt,
            bool usexinclude):
        QXmlStreamReader()
    {
        xmlDocPtr doc;
        QByteArray bname;

        this->bless();

        /* Parhaps a to native is needed here */
        bname = QFile::encodeName (name);

        doc = xmlParseFile(bname.constData());
        if(doc == NULL) {
            goto cannotparse;
}

        this->finalize(doc, schema, xslt, usexinclude);
        return;

cannotparse:
        qDebug() << "Can not parse";
        raiseError ("Can not parse");
        return;
    }
#endif

    /*!
     * \brief Construct a XML reader from a filename
     * \param name[in]: Name of the file
     * \param schema[in]: Schema in order to control file (NULL is no schema)
     * \param xslt[in]: xslt engine pointer for transformation (NULL is no xslt)
     * \param usexinclude[in]: use xinclude substitution
     */
    QXmlStreamReaderExt::QXmlStreamReaderExt(const QByteArray &array,
            const QRelaxNGvalidator * schema,
            const QXsltTransformer * xslt,
            bool usexinclude):
        QXmlStreamReader()
    {
        xmlDocPtr doc;

        this->bless();

        doc =  xmlParseMemory(array.constData(), array.size());
        if(doc == NULL) {
            goto cannotparse;
        }

        this->finalize(doc, schema, xslt, usexinclude);
        return;

cannotparse:
        qDebug() << "Can not parse";
        raiseError ("Can not parse");
        return;
    }




    /*!
     * \brief Finalize building of QXmlStreamReaderExt
     *
     * Factorize code. This code validate document and
     * transfer control to qt
     * \param docvoid[in]: xmlDocPtr doc of xml open
     * document
     * \param schema[in]: schema used could be null
     * \param xslt[in]: xslt transform to be used could be null
     * \note We use void cast in order to avoid exporting libxml
     * in .h
     */
    void QXmlStreamReaderExt::finalize(const void * docvoid,
            const QRelaxNGvalidator * schema,
            const QXsltTransformer *xslt,
            bool usexinclude)
    {
        xmlDocPtr doc = (xmlDocPtr) docvoid;
        xmlDocPtr xslteddoc = NULL;
        int size;

        /* apply xinclude */
        if(usexinclude) {
            if(xmlXIncludeProcess(doc) < 0) {
                goto errorxinclude;
            }
        }

        /* control schema */
        if(schema) {
            if(schema->hasError()) {
                this->raiseError(schema->ErrorString());
                goto notvalid;
            }
            /* validate */
            if(schema->validate(doc) == false) {
                goto notvalid;
            }
        }

        /* apply xslt */
        if(xslt != NULL) {
            if(xslt->hasError()) {
                this->raiseError(xslt->ErrorString());
                goto outxslterror;
            }
            xslteddoc = (xmlDocPtr) xslt->transform(doc);
            if(xslteddoc == NULL) {
                goto outxslterror;
            }
        }
        else {
            /* null transform */
            xslteddoc = doc;
        }

        /* save document */
        xmlDocDumpMemory(xslteddoc, (xmlChar **)&xmlout, &size);

        /* free */
        if(xslt != NULL) {
            xmlFreeDoc(xslteddoc);
        }
        xmlFreeDoc(doc);

        if(size < 0) {
            goto writeerror;
        }

        /* pass document to qt avoid copying data */
        this->data = QByteArray::fromRawData((char *) xmlout, size);
        this->addData(this->data);
        return;

errorxinclude:
        xmlFreeDoc(doc);
        qDebug() << "Xinclude error";
        raiseError ("Xinclude error");
notvalid:
        xmlFreeDoc(doc);
        qDebug() << "Could not validate";
        raiseError ("Could not validate");
        return;
outxslterror:
        qDebug() << "xslt error";
        raiseError ("xslt error");
writeerror:
        qDebug() << "Could not save to memory";
        raiseError ("Could not save to memory");
        return;
    }

    //! \brief Destructor
    QXmlStreamReaderExt::~QXmlStreamReaderExt()
    {
        xmlFree(xmlout);
    }

} // namespace Qucs
