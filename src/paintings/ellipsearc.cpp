/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "ellipsearc.h"

#include "styledialog.h"
#include "xmlutilities.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * \brief Constructs an elliptic arc item.
     * \param rect The ellipse rect of arc (local coords).
     * \param startAngle Starting angle of arc.
     * \param spanAngle Span angle of arc.
     * \param scene CGraphicsScene to which this item should be added.
     */
    EllipseArc::EllipseArc(QRectF rect, int startAngle, int spanAngle,
            CGraphicsScene *scene) :
        Painting(scene),
        m_startAngle(startAngle),
        m_spanAngle(spanAngle)
    {
        setEllipse(rect);
        setResizeHandles(Caneda::TopLeftHandle | Caneda::BottomRightHandle |
                Caneda::TopRightHandle| Caneda::BottomLeftHandle);
    }

    //! \brief Destructor.
    EllipseArc::~EllipseArc()
    {
    }

    //! \copydoc Painting::shapeForRect()
    QPainterPath EllipseArc::shapeForRect(const QRectF &rect) const
    {
        QPainterPath path;
        path.arcMoveTo(rect, m_startAngle);
        path.arcTo(rect, m_startAngle, m_spanAngle);
        return path;
    }

    //! \brief Set's this item's arc start angle to \a angle.
    void EllipseArc::setStartAngle(int angle)
    {
        m_startAngle = angle;
        update();
    }

    //! \brief Set's this item's arc span angle to \a angle.
    void EllipseArc::setSpanAngle(int angle)
    {
        m_spanAngle = angle;
        update();
    }

    //! \brief Draw's elliptic arc represented by this item.
    void EllipseArc::paint(QPainter *painter,
            const QStyleOptionGraphicsItem *option,
            QWidget *w)
    {
        painter->setBrush(Qt::NoBrush);
        if(option->state & QStyle::State_Selected) {
            QPen _pen(pen());

            _pen.setColor(Qt::darkGray);
            _pen.setWidth(pen().width() + 5);

            painter->setPen(_pen);
            painter->drawArc(ellipse(), 16 * m_startAngle, 16 * m_spanAngle);

            _pen.setColor(Qt::white);
            _pen.setWidth(pen().width());
            painter->setPen(_pen);
        }
        else {
            painter->setPen(pen());
        }
        painter->drawArc(ellipse(), 16 * m_startAngle, 16 * m_spanAngle);

        //call base method to draw resize handles.
        Painting::paint(painter, option, w);
    }

    //! \brief Returns a copy of EllipseArc painting item.
    EllipseArc* EllipseArc::copy(CGraphicsScene *scene) const
    {
        EllipseArc *arc = new EllipseArc(ellipse(), m_startAngle, m_spanAngle, scene);
        Painting::copyDataTo(arc);
        return arc;
    }

    //! \brief Save's data to xml referred by \a writer.
    void EllipseArc::saveData(Caneda::XmlWriter *writer) const
    {
        writer->writeStartElement("painting");
        writer->writeAttribute("name", "ellipseArc");

        writer->writeEmptyElement("properties");
        writer->writeRectAttribute(ellipse(), QLatin1String("ellipse"));
        writer->writeAttribute("startAngle", QString::number(m_startAngle));
        writer->writeAttribute("spanAngle", QString::number(m_spanAngle));
        writer->writePointAttribute(pos(), "pos");

        writer->writePen(pen());
        writer->writeTransform(transform());

        writer->writeEndElement(); // </painting>
    }

    //! \brief Loads data from xml represented by \a reader.
    void EllipseArc::loadData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "painting");
        Q_ASSERT(reader->attributes().value("name") == "ellipseArc");

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "properties") {
                    QRectF ellipse = reader->readRectAttribute(QLatin1String("ellipse"));
                    setEllipse(ellipse);

                    bool ok1, ok2;

                    setStartAngle(reader->attributes().value("startAngle").toString().toInt(&ok1));
                    setSpanAngle(reader->attributes().value("spanAngle").toString().toInt(&ok2));

                    if(!ok1 || !ok2) {
                        reader->raiseError(QObject::tr("Invalid arc attributes"));
                        break;
                    }

                    QPointF pos = reader->readPointAttribute("pos");
                    setPos(pos);

                    reader->readUnknownElement(); //read till end tag
                }

                else if(reader->name() == "pen") {
                    setPen(reader->readPen());
                }

                else if(reader->name() == "transform") {
                    setTransform(reader->readTransform());
                }

                else {
                    reader->readUnknownElement();
                }
            }
        }
    }

    int EllipseArc::launchPropertyDialog(Caneda::UndoOption opt)
    {
        StyleDialog dialog(this, opt);
        return dialog.exec();
    }

} // namespace Caneda
