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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//**************************************************************************
//

/*!
 * \file QXsltTransformer.cpp
 * \author ROUCARIES Bastien
 * \brief Implement xslt transformer engine
 */

#include "QXsltTransformer.h"

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QHashIterator>
#include <QString>

extern "C" {
#include <libxml/parser.h>
#include <libxslt/transform.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
}

#include <cstdlib>
#include <cstring>

// NULL is 0L which isn't appropriate in C++. So use 0.
#define NULL_PTR (0)

namespace Qucs {

    static const char * nulllist[] = { NULL_PTR , NULL_PTR };


    /*!
     * \brief Build a xslt parser from a file
     * \param array[in]: array for reading the whole xml file
     * \todo Remove error handling and add some slot and signal
     */
    QXsltTransformer::QXsltTransformer (const QByteArray &array)
    {
        xmlDocPtr doc;

        this->init();

        doc = xmlParseMemory(array.constData(), array.size());
        if(doc == NULL_PTR) {
            goto cannotparse;
        }

        this->xslt = xsltParseStylesheetDoc(doc);
        xmlFreeDoc(doc);
        if(this->xslt == NULL_PTR) {
            goto couldnotload;
        }
        return;

cannotparse:
        this->raiseError("Could not parse from memory");
        return;
couldnotload:
        this->raiseError("Could not load from memory");
        return;
    }


    /*!
     * \brief Build a xslt file parser from a xslt file
     * \param name[in]: file name of schema
     * \todo Check if QDir::tonative is needed
     * \todo Remove error handling and add some slot and signal
     */
    QXsltTransformer::QXsltTransformer (const QString & filename)
    {
        QByteArray bname;

        this->init();
        this->filename = filename;

        /* Perhaps a to native is needed here */
        bname = QFile::encodeName (filename);

        /* Load a xslt file */
        this->xslt = xsltParseStylesheetFile((const xmlChar *) bname.constData());
        if(this->xslt == NULL_PTR) {
            goto couldnotload;
        }
        return;

couldnotload:
        this->raiseError("Could not load xslt file"+this->filename);
        return;
    }



    //! \brief Construct an empty object
    void QXsltTransformer::init() {
        this->xslt = NULL_PTR;
        this->filename = "";
        this->emptylist();
        this->Error = "";
        this->errorflag = false;
    }



    /*!
     * \brief Insert a pair argument/value in the stack for the transformer
     * \param key: key used
     * \param value: value of the argument
     * \param pos: where to insert in array
     * \return true on success false on error (out of memory)
     */
    bool QXsltTransformer::insert(const QString &key, const QString &value,
            const unsigned int pos)
    {
        QByteArray temp;
        int tempsize;

        // allocate first string
        temp = key.toUtf8();
        tempsize = temp.size();

        *(this->charparam + 2 * pos)  = (char *) malloc(tempsize);
        if(*(this->charparam + 2 * pos) == NULL_PTR) {
            goto outfailfirst;
        }
        memcpy(this->charparam[pos],temp.constData(),tempsize);

        // allocate second string
        temp = value.toUtf8();
        tempsize = temp.size();

        *(this->charparam + 2 * pos + 1) = (char *) malloc(tempsize);
        if(*(this->charparam + 2 * pos + 1)== NULL_PTR) {
            goto outfailsecond;
        }
        memcpy(this->charparam[2 * pos + 1],temp.constData(),tempsize);

        return true;

outfailsecond:
        free(this->charparam + 2 * pos);
outfailfirst:
        return false;
    }

    /*!
     * \brief Insert a list of parameter  in stack
     * xslt accept some paremeters use hash as parameter list
     */
    bool QXsltTransformer::config(const QHash<QString, QString> & hash)
    {
        int charparamsize;
        int i;
        QHashIterator<QString, QString> it(hash);

        this->clean();
        this->emptylist();

        /* alloc a char ** for the parameter list terminated by NULL_PTR, NULL_PTR*/
        charparamsize = 2 * (hash.size() + 1) * sizeof(char *);
        this->charparam = (char **) malloc(charparamsize);
        if(this->charparam == NULL_PTR) {
            goto outofmemorycharparam;
        }
        memset(this->charparam, 0, charparamsize);

        i = 0;
        while(it.hasNext()) {
            it.next();
            if(this->insert(it.key(),it.value(), i) == false) {
                goto outkeyvalue;
            }
            i += 1;
        }

        return true;

outkeyvalue:
        this->clean();
        /* restore empty list */
outofmemorycharparam:
        this->emptylist();
        return false;
    }


    //! \brief Clean up parameter/value list
    void QXsltTransformer::clean(void)
    {
        int i;

        // avoid error
        if(this->charparam == nulllist) {
            return;
        }

        for(i = 0 ; this->charparam[i] != NULL_PTR ; i++) {
            free(this->charparam[i]);
        }

        free(this->charparam);
        this->charparam = NULL_PTR;
    }

    //! \brief Create an empty list of parameter/value list
    void QXsltTransformer::emptylist(void)
    {
        // cast to non const is valid normally we do not allow to write to nulllist
        this->charparam = (char **)nulllist;
    }



    /*!
     * \brief validate a xml document
     * \param[in]: doc xml document pointer from libxml
     * \return: xmlDocPtr to transformed document
     * \attention the caller should free the returned value
     */
    void * QXsltTransformer::transform(const void * doc) const {

        return (void *) xsltApplyStylesheet((xsltStylesheetPtr) this->xslt,
                (xmlDocPtr) doc,
                (const char **) this->charparam);
    }

    /*!
     * \brief Raise an error
     * \param error: error String
     */
    void QXsltTransformer::raiseError(const QString& error)
    {
        this->Error += error;
        this->errorflag = true;
    }

    /*!
     * \brief Default destructor
     *
     * Free libxml object
     */
    QXsltTransformer::~QXsltTransformer()
    {
        xsltFreeStylesheet((xsltStylesheetPtr) this->xslt);
        this->clean();
    }

} //namespace Qucs
