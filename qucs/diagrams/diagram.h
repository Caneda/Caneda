/***************************************************************************
                               diagram.h
                              -----------
    begin                : Thu Oct 2 2003
    copyright            : (C) 2003 by Michael Margraf
    email                : michael.margraf@alumni.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//EDIT by Gopala Krishna A: I'll port this little later. For now I need
//                          only definition of the class

#ifndef DIAGRAM_H
#define DIAGRAM_H

#include "item.h"

class Diagram : public QucsItem
{
public:
    enum {
        Type = QucsItem::DisplayType
    };

    Diagram(SchematicScene* scene = 0);
    ~Diagram();

    int type() const { return QucsItem::DisplayType; }
};

#endif
