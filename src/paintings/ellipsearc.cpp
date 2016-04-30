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

#include "ellipsearc.h"

#include "settings.h"
#include "styledialog.h"
#include "xmlutilities.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace Caneda
{
    /*!
     * \brief Constructs an elliptic arc item.
     *
     * \param rect The ellipse rect of arc (local coords).
     * \param startAngle Starting angle of arc.
     * \param spanAngle Span angle of arc.
     * \param scene GraphicsScene to which this item should be added.
     */
    EllipseArc::EllipseArc(QRectF rect, int startAngle, int spanAngle,
            GraphicsScene *scene) :
        Painting(scene),
        m_startAngle(startAngle),
        m_spanAngle(spanAngle)
    {
        setEllipse(rect);
        setResizeHandles(Caneda::TopLeftHandle | Caneda::BottomRightHandle |
                Caneda::TopRightHandle| Caneda::BottomLeftHandle);
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
        if(option->state & QStyle::State_Selected) {
            Settings *settings = Settings::instance();
            painter->setPen(QPen(settings->currentValue("gui/selectionColor").value<QColor>(),
                                 pen().width()));
        }
        else {
            painter->setPen(pen());
        }

        painter->setBrush(Qt::NoBrush);

        painter->drawArc(ellipse(), 16 * m_startAngle, 16 * m_spanAngle);

        // Call base method to draw resize handles.
        Painting::paint(painter, option, w);
    }

    //! \copydoc GraphicsItem::copy()
    EllipseArc* EllipseArc::copy(GraphicsScene *scene) const
    {
        EllipseArc *ellipseArc = new EllipseArc(ellipse(), m_startAngle, m_spanAngle, scene);
        Painting::copyDataTo(ellipseArc);
        return ellipseArc;
    }

    //! \brief Save's data to xml referred by \a writer.
    void EllipseArc::saveData(Caneda::XmlWriter *writer) const
    {
        writer->writeStartElement("painting");
        writer->writeAttribute("name", "ellipseArc");

        writer->writeRectAttribute(ellipse(), QLatin1String("ellipse"));
        writer->writePointAttribute(pos(), "pos");
        writer->writeTransformAttribute(sceneTransform());

        writer->writeEmptyElement("properties");
        writer->writeAttribute("startAngle", QString::number(m_startAngle));
        writer->writeAttribute("spanAngle", QString::number(m_spanAngle));

        writer->writePen(pen());

        writer->writeEndElement(); // </painting>
    }

    //! \brief Loads data from xml represented by \a reader.
    void EllipseArc::loadData(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "painting");
        Q_ASSERT(reader->attributes().value("name") == "ellipseArc");

        setEllipse(reader->readRectAttribute(QLatin1String("ellipse")));
        setPos(reader->readPointAttribute("pos"));
        setTransform(reader->readTransformAttribute("transform"));

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement()) {

                if(reader->name() == "properties") {
                    bool ok1, ok2;

                    setStartAngle(reader->attributes().value("startAngle").toString().toInt(&ok1));
                    setSpanAngle(reader->attributes().value("spanAngle").toString().toInt(&ok2));

                    if(!ok1 || !ok2) {
                        reader->raiseError(QObject::tr("Invalid arc attributes"));
                        break;
                    }

                    reader->readUnknownElement();  // Read till end tag
                }

                else if(reader->name() == "pen") {
                    setPen(reader->readPen());
                }

                else {
                    reader->readUnknownElement();
                }
            }
        }
    }

    int EllipseArc::launchPropertiesDialog()
    {
        StyleDialog dialog(this);
        return dialog.exec();
    }

} // namespace Caneda
