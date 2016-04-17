/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2010-2016 by Pablo Daniel Pareja Obregon                  *
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

#ifndef GLOBAL_H
#define GLOBAL_H

#include <config.h>

#include <QDebug>
#include <QGraphicsItem>
#include <QPainter>

// Forward declarations
class QGraphicsItem;
class QIcon;

namespace Caneda
{
    QString baseDirectory();
    QString binaryDirectory();
    QString bitmapDirectory();
    QString langDirectory();
    QString libDirectory();

    QString version();
    QString versionString();

    QIcon icon(const QString& iconName);

    QString localePrefix();

    bool checkVersion(const QString& line);

    inline QString boolToString(bool boolean) {
        return boolean ? QString("true") : QString("false");
    }

    inline QString realToString(qreal val) {
        return QString::number(val,'f',2);
    }

    //! \brief Helper method to return sign of given integer.
    inline int sign(int value)
    {
        return value >= 0 ? +1 : -1;
    }

    //! \brief Short function for qsort sort by abscissa
    static inline bool pointCmpFunction_X(const QGraphicsItem *lhs, const QGraphicsItem  *rhs)
    {
        return lhs->pos().x() < rhs->pos().x();
    }

    //!Short function for qsort sort by abscissa
    static inline bool pointCmpFunction_Y(const QGraphicsItem *lhs, const QGraphicsItem  *rhs)
    {
        return lhs->pos().y() < rhs->pos().y();
    }

    //! \brief Default grid spacing
    static const uint DefaultGridSpace = 10;
    QPointF smartNearingGridPoint(const QPointF &pos);

    QString latexToUnicode(const QString& input);
    QString unicodeToLatex(QString unicode);

    //! \brief Possible mouse actions
    enum MouseAction {
        Wiring,             // Wire action
        Deleting,           // Delete
        Rotating,           // Rotate
        MirroringX,         // Mirror X
        MirroringY,         // Mirror Y
        ZoomingAreaEvent,   // Zoom an area
        PaintingDrawEvent,  // Painting item's drawing (like Ellipse, Rectangle)
        InsertingItems,     // Inserting an item
        Normal              // Normal action (ie select)
    };

    struct ZoomRange
    {
        const qreal min;
        const qreal max;

        ZoomRange(qreal _min = 0., qreal _max = 1.0) :
                min(_min), max(_max)
        {
            if (max < min) {
                qWarning() << Q_FUNC_INFO << "Invalid range" << min << max;
            }
        }

        bool contains(qreal value) const {
            return value >= min && value <= max;
        }
    };

    //! \brief This enum determines the rotation direction.
    enum AngleDirection {
        Clockwise,
        AntiClockwise
    };

    //! \brief Render hints
    static const QPainter::RenderHints DefaulRenderHints = QPainter::Antialiasing | QPainter::SmoothPixmapTransform;

} // namespace Caneda

#endif //GLOBAL_H
