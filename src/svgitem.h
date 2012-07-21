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

#ifndef SVGITEM_H
#define SVGITEM_H

#include <QHash>
#include <QSvgRenderer>

namespace Caneda
{
    /*!
     * \brief This class packs some information needed by svg items which are
     *        shared by many items.
     */
    class SvgItemData
    {
    public:
        SvgItemData(const QByteArray& _content);
        QRectF boundingRect() const;

    private:
        friend class SvgPainter;

        QByteArray content; //!< Represents raw svg content.
        QSvgRenderer renderer; //!< Represents svg renderer which renders svg.
    };

    /*!
     * \brief A class used to take care of rendering svg.
     *
     * This class renders a svg, given the svg id. The svg to be rendered should
     * be first registered with the instance of this class. This class also
     * supports modifying style of svg using css.
     *
     * \sa registerSvg()
     */
    struct SvgPainter : public QObject
    {
    public:
        static SvgPainter* instance();
        ~SvgPainter();

        void registerSvg(const QString& svg_id, const QByteArray& content);

        /*!
         * Returns whether the svg corresponding to \svg_id is registered
         * or not with \a registerSvg.
         */
        bool isSvgRegistered(const QString& svg_id) const {
            return m_dataHash.contains(svg_id);
        }

        QSvgRenderer *rendererFor(const QString& svg_id) const;
        QRectF boundingRect(const QString& svg_id) const;

        void paint(QPainter *painter, const QString& svg_id);
        SvgItemData* svgData(const QString& svg_id) const;

        QByteArray svgContent(const QString& svg_id) const;

    private:
        SvgPainter(QObject *parent = 0);

        QHash<QString, SvgItemData*> m_dataHash; //!< Holds svg data in a hash table.
    };

} // namespace Caneda

#endif //SVGITEM_H
