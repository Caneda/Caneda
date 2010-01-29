// Copyright(C) 2008 ROUCARIES Bastien <roucaries.bastien+qucs@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//**************************************************************************
//

#include "QRelaxNGvalidator.h"

#include <QFile>
#include <QDebug>
#include <QString>

extern "C" {
#include <libxml/parser.h>
#include <libxml/relaxng.h>
}

#include <iostream>

namespace Qucs {

    /*!
     * \brief Build a validator from a schematic file
     * \param array[in]: array for reading the whole xml file
     * \todo Remove error handling and add some slot and signal
     */
    QRelaxNGvalidator::QRelaxNGvalidator(const QByteArray &array)
    {
        xmlRelaxNGParserCtxtPtr pctxt;

        /* Load a schematic file */
        pctxt = xmlRelaxNGNewMemParserCtxt(array.constData(),array.size());
        if(!pctxt) {
            goto couldnotload;
        }

        this->finish(pctxt);
        return;

couldnotload:
        this->raiseError("Could not load from memory");
        return;
    }


    //! \brief Construct an empty object
    void QRelaxNGvalidator::bless()
    {
        this->rng = 0;
        this->vctxt = 0;
        this->filename = "";
        this->errorflag = false;
        this->Error = "";
    }


    /*!
     * \brief Build a validator from a schematic file
     * \param name[in]: file name of schema
     * \todo Check if QDir::tonative is needed
     * \todo Remove error handling and add some slot and signal
     */
    QRelaxNGvalidator::QRelaxNGvalidator(const QString & filename)
    {
        xmlRelaxNGParserCtxtPtr pctxt;
        QByteArray bname;

        this->bless();
        this->filename = filename;

        //  Parhaps a to native is needed here
        bname = QFile::encodeName(filename);

        //  Load a schematic file
        pctxt = xmlRelaxNGNewParserCtxt(bname.constData());
        if(pctxt == NULL) {
            goto couldnotload;
        }

        this->finish(pctxt);
        return;

couldnotload:
        this->raiseError("Could not load file " + this->filename) ;
        return;
    }

    /*!
     * \brief Private constructor from a libxml pointer
     *
     * Do the real configuration from a libxml pointer
     * \param pctxt[in]: lib xml document pointer
     */
    void QRelaxNGvalidator::finish(const void * pctxt)
    {
        //  Parse schema
        this->rng = xmlRelaxNGParse((xmlRelaxNGParserCtxtPtr)pctxt);
        xmlRelaxNGFreeParserCtxt((xmlRelaxNGParserCtxtPtr)pctxt);
        if(!rng) {
            goto schemaerror;
        }

        //  Create a validation context
        vctxt = xmlRelaxNGNewValidCtxt((xmlRelaxNGPtr)this->rng);
        if(!vctxt) {
            goto contexterror;
        }

        /* Ask the validation parser to return error */
        xmlRelaxNGSetValidErrors((xmlRelaxNGValidCtxtPtr)this->vctxt,
                (xmlRelaxNGValidityErrorFunc) fprintf,
                (xmlRelaxNGValidityWarningFunc) fprintf,
                stderr);
        return;

schemaerror:
        this->raiseError(QString("Schema ") + this->filename + QString(" has an error"));
        return;
contexterror:
        this->raiseError(QString("Could not create validation context for ")
                + this->filename + QString("schema"));
        xmlRelaxNGFree((xmlRelaxNGPtr)this->rng);
        return;
    }


    /*!
     * \brief validate a xml document
     * \param[in]: doc xml document pointer from libxml
     * \return: true if document is valid
     */
    bool QRelaxNGvalidator::validate(const void * doc) const {
        int ret;

        //  return 0 if valid
        ret = xmlRelaxNGValidateDoc((xmlRelaxNGValidCtxtPtr)this->vctxt,
               (xmlDocPtr)doc);

        return !ret;
    }

    /*!
     * \brief Raise an error
     * \param error: error String
     */
    void QRelaxNGvalidator::raiseError(const QString& error)
    {
        this->Error += error;
        this->errorflag = true;
    }



    /*!
     * \brief Default destructor
     *
     * Free libxml object
     */
    QRelaxNGvalidator::~QRelaxNGvalidator()
    {
        xmlRelaxNGFreeValidCtxt((xmlRelaxNGValidCtxtPtr)this->vctxt);
        xmlRelaxNGFree((xmlRelaxNGPtr)this->rng);
    }

} // namespace qucs
