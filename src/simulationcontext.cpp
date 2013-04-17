/***************************************************************************
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

#include "simulationcontext.h"

#include "simulationdocument.h"
#include "singletonowner.h"

#include <QFileInfo>
#include <QStringList>

namespace Caneda
{
    SimulationContext::SimulationContext(QObject *parent) : IContext(parent)
    {
    }

    SimulationContext* SimulationContext::instance()
    {
        static SimulationContext *context = 0;
        if (!context) {
            context = new SimulationContext(SingletonOwner::instance());
        }
        return context;
    }

    SimulationContext::~SimulationContext()
    {
    }

    void SimulationContext::init()
    {
    }

    bool SimulationContext::canOpen(const QFileInfo &info) const
    {
        QStringList supportedSuffixes;
        supportedSuffixes << "xdat";
        supportedSuffixes << "raw";

        foreach (const QString &suffix, supportedSuffixes) {
            if (suffix == info.suffix()) {
                return true;
            }
        }

        return false;
    }

    QStringList SimulationContext::fileNameFilters() const
    {
        QStringList nameFilters;
        nameFilters << QObject::tr("Data display (*.xdat)")+" (*.xdat);;";
        nameFilters << QObject::tr("Raw waveform data (*.raw)")+" (*.raw);;";

        return nameFilters;
    }

    IDocument* SimulationContext::newDocument()
    {
        return new SimulationDocument;
    }

    IDocument* SimulationContext::open(const QString &fileName,
            QString *errorMessage)
    {
        SimulationDocument *document = new SimulationDocument();
        document->setFileName(fileName);

        if (!document->load(errorMessage)) {
            delete document;
            document = 0;
        }

        return document;
    }

    void SimulationContext::addNormalAction(Action *action)
    {
        m_normalActions << action;
    }

    void SimulationContext::addMouseAction(Action *action)
    {
        m_mouseActions << action;
    }

    void SimulationContext::exportCsv()
    {
        //! \todo Implement this
    }

} // namespace Caneda
