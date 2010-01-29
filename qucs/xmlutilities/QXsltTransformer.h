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
 * \file QXsltTransformer.h
 * \author ROUCARIES Bastien
 * \brief Implement xslt transformer engine
 */

#ifndef QXSLTTRANSFORMER_H
#define QXSLTTRANSFORMER_H

#include <QHash>
#include <QString>

namespace Qucs {

    // forward declaration
    class QXmlStreamReaderExt;

    /*!
     * \brief Xslt transformer class
     *
     * This class implement the xslt transformer engine
     */
    class QXsltTransformer
    {
    public:
        //! default constructor
        QXsltTransformer () { this->init(); };
        //! construct from a byte array
        QXsltTransformer (const QString & filename);
        //! construct from a filename
        QXsltTransformer (const QByteArray &array);

        //! destructor
        ~QXsltTransformer();

        //! insert a hash list of parameter
        bool config(const QHash<QString, QString> & hash);

        //  friend class
        friend class QXmlStreamReaderExt;

        //! Return error status
        bool hasError() const { return this->errorflag; };

        //! Return error string
        QString ErrorString() const { return this->Error; };

    private:
        //!  pointer to in memory parsed script
        void * xslt;

        //! \brief Cached filename
        QString filename;

        //! \brief pointer to an array of param/value
        char **charparam;


        //! \brief Construct an empty object
        void init();

        //  finish construction
        void finish(const void * pctxt);

        //  Create an empty list
        void emptylist(void);

        //  clean the full list
        void clean();

        //  insert pair argument/value
        bool insert(const QString &key, const QString &value,
                const unsigned int pos);


        void raiseError(const QString &error);
        //! \brief Error String
        QString Error;
        //! \brief Set in case of error
        bool errorflag;

    protected:
        //  validate document
        void * transform(const void * doc) const;
    };

} // namespace Qucs
#endif

