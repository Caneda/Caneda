/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2012-2013 by Pablo Daniel Pareja Obregon                  *
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

#include "arrow.h"

#include "settings.h"
#include "styledialog.h"
#include "xmlutilities.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <math.h>

namespace Caneda
{
    /*!
     * \brief Constructor.
     *
     * \param line The line used to represent the arrow's line part(local coords).
     * \param style The arrow's head style. See \a HeadStyle
     * \param headWidth The base width of triangle of arrow's head.
     * \param headHeight The height of triangle of arrow head.
     * \param scene The graphics scene to which this arrow is to be added.
     */
    Arrow::Arrow(const QLineF &line, HeadStyle style, qreal headWidth, qreal headHeight,
            CGraphicsScene *scene) :
        Painting(scene),
        m_headStyle(style),
        m_headWidth(headWidth),
        m_headHeight(headHeight)
    {
        setLine(line);
        setBrush(Qt::black);
        setResizeHandles(Caneda::TopLeftHandle | Caneda::BottomRightHandle);
    }

    //! \copydoc Painting::shapeForRect()
    QPainterPath Arrow::shapeForRect(const QRectF &rect) const
    {
        QPainterPath path;
        path.moveTo(rect.topLeft());
        path.lineTo(rect.bottomRight());

        path.addPolygon(m_head.translated(rect.topLeft()));
        path.closeSubpath();

        return path;
    }

    //! Draw's the arrow and arrow head based on \a style.
    void Arrow::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *w)
    {
        if(option->state & QStyle::State_Selected) {
            Settings *settings = Settings::instance();
            painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>(),
                                 pen().width()));

            painter->setBrush(Qt::NoBrush);
        }
        else {
            painter->setPen(pen());

            if(headStyle() == FilledArrow) {
                painter->setBrush(brush());
            }
            else {
                painter->setBrush(Qt::NoBrush);
            }
        }

        QLineF line = lineFromRect(paintingRect());
        painter->drawLine(line);
        drawHead(painter);

        // Call base method to draw resize handles.
        Painting::paint(painter, option, w);
    }

    //! \brief Returns a copy of this arrow.
    Arrow* Arrow::copy(CGraphicsScene *scene) const
    {
        Arrow *arrow = new Arrow(line(), headStyle(), headWidth(), headHeight(), scene);
        Painting::copyDataTo(arrow);
        return arrow;
    }

    //! \brief Save arrow data to xml referred by writer.
    void Arrow::saveData(Caneda::XmlWriter *writer) const
    {
        writer->writeStartElement("painting");
        writer->writeAttribute("name", "arrow");

        writer->writeLineAttribute(line());
        writer->writePointAttribute(pos(), "pos");
        writer->writeTransformAttribute(transform());

        writer->writeEmptyElement("properties");
        writer->writeAttribute("headStyle", QString::number(int(m_headStyle)));
        writer->writePointAttribute(QPointF(m_headWidth, m_headHeight), "headSize");

        writer->writePen(pen());
        writer->writeBrush(brush());

        writer->writeEndElement(); // </painting>
    }

    //! \brief Loads arrow from xml reffered by reader.
    void Arrow::loadData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "painting");
        Q_ASSERT(reader->attributes().value("name") == "arrow");

        QLineF line = reader->readLineAttribute("line");
        setPaintingRect(QRectF(line.p1(), line.p2()));
        setPos(reader->readPointAttribute("pos"));
        setTransform(reader->readTransformAttribute("transform"));

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {

                if(reader->name() == "properties") {
                    int style = reader->attributes().value("headStyle").toString().toInt();
                    setHeadStyle((HeadStyle)style);

                    QPointF headSize = reader->readPointAttribute("headSize");
                    setHeadWidth(headSize.x());
                    setHeadHeight(headSize.y());

                    reader->readUnknownElement();  // Read till end tag
                }

                else if(reader->name() == "pen") {
                    setPen(reader->readPen());
                }

                else if(reader->name() == "brush") {
                    setBrush(reader->readBrush());
                }

                else {
                    reader->readUnknownElement();
                }
            }
        }
    }

    //! \brief Sets arrow's head style to \a style.
    void Arrow::setHeadStyle(HeadStyle style)
    {
        m_headStyle = style;
        calcHeadPoints();
        update();
    }

    //! \brief Sets arrow's head width to \a width.
    void Arrow::setHeadWidth(qreal width)
    {
        m_headWidth = width;
        setLine(line());//recalc geometry
        calcHeadPoints();
        update();
    }

    //! \brief Sets arrow's head height to \a height.
    void Arrow::setHeadHeight(qreal height)
    {
        m_headHeight = height;
        setLine(line());//recalc geometry
        calcHeadPoints();
        update();
    }

    //! \brief Sets arrow's line to \a line.
    void Arrow::setLine(const QLineF& line)
    {
        setPaintingRect(QRectF(line.p1(), line.p2()));
    }

    //! \copydoc Painting::geometryChange()
    void Arrow::geometryChange()
    {
        calcHeadPoints();
    }

    //! \brief Calculates arrow's head polygon based on style, width and height.
    void Arrow::calcHeadPoints()
    {
        QRectF rect = paintingRect();

        qreal angle = (atan2(-rect.height(), rect.width()));
        angle = -270 + (angle * 180 / M_PI);

        QMatrix mapper;
        mapper.rotate(angle);

        //qreal lengthFromOrigin = QLineF(QPointF(), rect.bottomRight()).length();

        QPointF arrowTipPos = mapper.map(rect.bottomRight());
        QPointF bottomLeft(arrowTipPos.x() - m_headWidth/2, arrowTipPos.y() - m_headHeight);
        QPointF bottomRight(arrowTipPos.x() + m_headWidth/2, arrowTipPos.y() - m_headHeight);

        mapper = mapper.inverted();

        if(m_head.size() != 3) {
            m_head.resize(3);
        }
        m_head[0] = mapper.map(bottomLeft);
        m_head[1] = mapper.map(arrowTipPos);
        m_head[2] = mapper.map(bottomRight);
    }

    //! \brief Returns line representation of rect - topleft to bottom right.
    QLineF Arrow::lineFromRect(const QRectF& rect) const
    {
        return QLineF(rect.topLeft(), rect.bottomRight());
    }

    //! \brief Draws arrow head based on current head style.
    void Arrow::drawHead(QPainter *painter)
    {
        if(m_headStyle == FilledArrow) {
            painter->drawConvexPolygon(m_head);
        }
        else {
            painter->drawLine(m_head[0], m_head[1]);
            painter->drawLine(m_head[1], m_head[2]);
        }
    }

    int Arrow::launchPropertyDialog(Caneda::UndoOption opt)
    {
        StyleDialog dia(this, opt);
        return dia.exec();
    }

} // namespace Caneda
