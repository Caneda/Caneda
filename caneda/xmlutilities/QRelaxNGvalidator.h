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

#ifndef QRELAXNGVALIDATOR_H
#define QRELAXNGVALIDATOR_H

#include <QString>

namespace Caneda
{
    //  forward declaration
    class QXmlStreamReaderExt;

    /*!
     * \brief Validator class
     * This class validate a document from a given relax NG schema
     */
    class QRelaxNGvalidator {
    public:
        //  construct from a byte array
        QRelaxNGvalidator(const QString & filename);
        //  construct from a filename
        QRelaxNGvalidator(const QByteArray &array);

        //  destructor
        ~QRelaxNGvalidator();

        //  friend class
        friend class QXmlStreamReaderExt;

        //! Return error status
        bool hasError() const { return this->errorflag; };

        //! Return error string
        QString ErrorString() const { return this->Error; };

    private:
        void * rng;
        void * vctxt;

        //! \brief Cached filename
        QString filename;

        void bless();

        //  finish construction
        void finish(const void * pctxt);

        void raiseError(const QString &error);
        //!  error flag
        bool errorflag;
        //!  error string
        QString Error;
    protected:
        //  validate document
        bool validate(const void * doc) const;
    };

} // namespace Caneda

#endif //QRELAXNGVALIDATOR_H
