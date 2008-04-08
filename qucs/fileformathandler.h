/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#ifndef __FILEFORMATHANDLER_H
#define __FILEFORMATHANDLER_H

#include <QtCore/QString>

class SchematicView;

/** This class is used to save and load files.
 * Using this base class we can support any fileformat */
class FileFormatHandler
{
   public:
      FileFormatHandler(SchematicView *view=0);
      virtual ~FileFormatHandler() {}

      virtual QString saveText() = 0;

      /** Loads the document. If non-negative is returned
        * the operation is successful. Negative return
        * value indicated failure */
      virtual bool loadFromText(const QString& text) = 0;

      SchematicView* view() const { return m_view; }
      void setView(SchematicView *view) { m_view = view; }

      static FileFormatHandler* handlerFromSuffix(const QString& extension,
                                                     SchematicView *view = 0);

   protected:
      SchematicView *m_view;

};

#endif //__FILEFORMATHANDLER_H
