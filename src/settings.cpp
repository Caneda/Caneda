/***************************************************************************
 * Copyright (C) 2010 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "settings.h"

#include "global.h"

#include <QColor>
#include <QFont>
#include <QKeySequence>
#include <QSettings>
#include <QSize>
#include <QVariant>

namespace Caneda
{
    //! \brief Constructor.
    Settings::Settings(QObject *parent) : QObject(parent)
    {
        QStringList libraries;
        libraries << Caneda::libDirectory() + "components/active";
        libraries << Caneda::libDirectory() + "components/cmos";
        libraries << Caneda::libDirectory() + "components/miscellaneous";
        libraries << Caneda::libDirectory() + "components/models";
        libraries << Caneda::libDirectory() + "components/passive";
        libraries << Caneda::libDirectory() + "components/semiconductor";
        libraries << Caneda::libDirectory() + "components/simulations";
        libraries << Caneda::libDirectory() + "components/sources";
        libraries << Caneda::libDirectory() + "components/transmission";

        defaultSettings["libraries/schematic"] = QVariant(QStringList(libraries));
        defaultSettings["libraries/hdl"] = QVariant(QStringList(Caneda::libDirectory() + "hdl"));

        defaultSettings["gui/showMenuBar"] = QVariant(bool(true));
        defaultSettings["gui/showToolBar"] = QVariant(bool(true));
        defaultSettings["gui/showStatusBar"] = QVariant(bool(true));
        defaultSettings["gui/showSideBarBrowser"] = QVariant(bool(true));
        defaultSettings["gui/showFolderBrowser"] = QVariant(bool(true));
        defaultSettings["gui/showFullScreen"] = QVariant(bool(false));

        defaultSettings["gui/gridVisible"] = QVariant(bool(true));
        defaultSettings["gui/foregroundColor"] = QVariant(QColor(Qt::darkGray));
        defaultSettings["gui/backgroundColor"] = QVariant(QColor(Qt::white));
        defaultSettings["gui/simulationBackgroundColor"] = QVariant(QColor(Qt::white));
        defaultSettings["gui/lineColor"] = QVariant(QColor(Qt::blue));
        defaultSettings["gui/selectionColor"] = QVariant(QColor(255, 128, 0)); // Dark orange
        defaultSettings["gui/lineWidth"] = QVariant(int(1));

        defaultSettings["gui/hdl/keyword"]= QVariant(QVariant(QColor(Qt::black)));
        defaultSettings["gui/hdl/type"]= QVariant(QVariant(QColor(Qt::blue)));
        defaultSettings["gui/hdl/attribute"]= QVariant(QVariant(QColor(Qt::darkCyan)));
        defaultSettings["gui/hdl/block"] = QVariant(QColor(Qt::darkBlue));
        defaultSettings["gui/hdl/class"] = QVariant(QColor(Qt::darkMagenta));
        defaultSettings["gui/hdl/data"]= QVariant(QVariant(QColor(Qt::darkGreen)));
        defaultSettings["gui/hdl/comment"] = QVariant(QColor(Qt::red));
        defaultSettings["gui/hdl/system"] = QVariant(QColor(Qt::darkYellow));

        defaultSettings["gui/layout/metal1"] = QVariant(QColor(Qt::blue));
        defaultSettings["gui/layout/metal2"] = QVariant(QColor(Qt::gray));
        defaultSettings["gui/layout/poly1"] = QVariant(QColor(Qt::red));
        defaultSettings["gui/layout/poly2"] = QVariant(QColor(Qt::darkRed));
        defaultSettings["gui/layout/active"] = QVariant(QColor(Qt::green));
        defaultSettings["gui/layout/contact"] = QVariant(QColor(Qt::black));
        defaultSettings["gui/layout/nwell"] = QVariant(QColor(Qt::darkYellow));
        defaultSettings["gui/layout/pwell"] = QVariant(QColor(Qt::darkCyan));

        defaultSettings["sim/simulationEngine"] = QVariant(QString("ngspice"));  //! \todo In the future this could be replaced by an enum, to avoid problems
        defaultSettings["sim/simulationCommand"] = QVariant(QString("ngspice -b -r %filename.raw %filename.net"));
        defaultSettings["sim/outputFormat"] = QVariant(QString("binary"));  //! \todo In the future this could be replaced by an enum, to avoid problems

        defaultSettings["shortcuts/fileNew"] = QVariant(QKeySequence(QKeySequence::New));
        defaultSettings["shortcuts/fileOpen"] = QVariant(QKeySequence(QKeySequence::Open));
        defaultSettings["shortcuts/fileSave"] = QVariant(QKeySequence(QKeySequence::Save));
        defaultSettings["shortcuts/fileSaveAs"] = QVariant(QKeySequence(QKeySequence::SaveAs));
        defaultSettings["shortcuts/fileClose"] = QVariant(QKeySequence(QKeySequence::Close));
        defaultSettings["shortcuts/filePrint"] = QVariant(QKeySequence(QKeySequence::Print));
        defaultSettings["shortcuts/fileExportImage"] = QVariant(QKeySequence(tr("Ctrl+E")));
        defaultSettings["shortcuts/fileQuit"] = QVariant(QKeySequence(QKeySequence::Quit));

        defaultSettings["shortcuts/editUndo"] = QVariant(QKeySequence(QKeySequence::Undo));
        defaultSettings["shortcuts/editRedo"] = QVariant(QKeySequence(QKeySequence::Redo));
        defaultSettings["shortcuts/editCut"] = QVariant(QKeySequence(QKeySequence::Cut));
        defaultSettings["shortcuts/editCopy"] = QVariant(QKeySequence(QKeySequence::Copy));
        defaultSettings["shortcuts/editPaste"] = QVariant(QKeySequence(QKeySequence::Paste));
        defaultSettings["shortcuts/selectAll"] = QVariant(QKeySequence(QKeySequence::SelectAll));
        defaultSettings["shortcuts/editFind"] = QVariant(QKeySequence(QKeySequence::Find));

        defaultSettings["shortcuts/select"] = QVariant(QKeySequence(tr("Esc")));
        defaultSettings["shortcuts/editDelete"] = QVariant(QKeySequence(QKeySequence::Delete));
        defaultSettings["shortcuts/editRotate"] = QVariant(QKeySequence(tr("R")));
        defaultSettings["shortcuts/editMirrorX"] = QVariant(QKeySequence(tr("V")));
        defaultSettings["shortcuts/editMirrorY"] = QVariant(QKeySequence(tr("H")));
        defaultSettings["shortcuts/insertWire"] = QVariant(QKeySequence(tr("W")));

        defaultSettings["shortcuts/zoomFitInBest"] = QVariant(QKeySequence(tr("0")));
        defaultSettings["shortcuts/zoomOriginal"] = QVariant(QKeySequence(tr("1")));
        defaultSettings["shortcuts/zoomIn"] = QVariant(QKeySequence(tr("+")));
        defaultSettings["shortcuts/zoomOut"] = QVariant(QKeySequence(tr("-")));
        defaultSettings["shortcuts/splitHorizontal"] = QVariant(QKeySequence(tr("Ctrl+Shift+L")));
        defaultSettings["shortcuts/splitVertical"] = QVariant(QKeySequence(tr("Ctrl+Shift+T")));
        defaultSettings["shortcuts/splitClose"] = QVariant(QKeySequence(tr("Ctrl+Shift+R")));

        defaultSettings["shortcuts/openSchematic"] = QVariant(QKeySequence(tr("F2")));
        defaultSettings["shortcuts/openSymbol"] = QVariant(QKeySequence(tr("F3")));
        defaultSettings["shortcuts/openLayout"] = QVariant(QKeySequence(tr("F4")));
        defaultSettings["shortcuts/simulate"] = QVariant(QKeySequence(QKeySequence::Refresh));
        defaultSettings["shortcuts/openSimulation"] = QVariant(QKeySequence(tr("F6")));
        defaultSettings["shortcuts/openLog"] = QVariant(QKeySequence(tr("F7")));
        defaultSettings["shortcuts/openNetlist"] = QVariant(QKeySequence(tr("F8")));

        defaultSettings["shortcuts/filterItems"] = QVariant(QKeySequence(tr("C")));
        defaultSettings["shortcuts/enterHierarchy"] = QVariant(QKeySequence(tr("Ctrl+I")));
        defaultSettings["shortcuts/exitHierarchy"] = QVariant(QKeySequence(tr("Ctrl+Shift+I")));
        defaultSettings["shortcuts/invokeRunner"] = QVariant(QKeySequence(tr("Ctrl+Space")));

        defaultSettings["shortcuts/showMenuBar"] = QVariant(QKeySequence(tr("Ctrl+M")));
        defaultSettings["shortcuts/showWidgets"] = QVariant(QKeySequence(tr("Ctrl+Up")));
        defaultSettings["shortcuts/showSideBarBrowser"] = QVariant(QKeySequence(tr("T")));
        defaultSettings["shortcuts/showFolderBrowser"] = QVariant(QKeySequence(tr("F")));
        defaultSettings["shortcuts/showFullScreen"] = QVariant(QKeySequence(tr("Ctrl+Shift+F")));
        defaultSettings["shortcuts/settings"] = QVariant(QKeySequence(QKeySequence::Preferences));
        defaultSettings["shortcuts/helpIndex"] = QVariant(QKeySequence(QKeySequence::HelpContents));

        currentSettings = defaultSettings;
    }

    //! \copydoc MainWindow::instance()
    Settings* Settings::instance()
    {
        static Settings *instance = 0;
        if (!instance) {
            instance = new Settings();
        }
        return instance;
    }

    /*!
     * \brief Get the current value of the selected setting.
     *
     * \param key Setting to look for.
     * \return QVariant value of the setting.
     *
     * \sa setCurrentValue
     */
    QVariant Settings::currentValue(const QString& key) const
    {
        if (!currentSettings[key].isValid()) {
            return defaultSettings[key];
        }
        return currentSettings[key];
    }

    /*!
     * \brief Get the default value of the selected setting.
     *
     * \param key Setting to look for.
     * \return QVariant default value of the setting.
     */
    QVariant Settings::defaultValue(const QString& key) const
    {
        return defaultSettings[key];
    }

    /*!
     * \brief Set a new value for the selected setting.
     *
     * \param key Setting to change its value.
     * \param value New value of the setting.
     *
     * \sa currentValue
     */
    void Settings::setCurrentValue(const QString& key, const QVariant& value)
    {
        currentSettings[key] = value.isValid() ? value : defaultSettings[key];
    }

    /*!
     * \brief Load stored settings values.
     *
     * \return True on success, false otherwise.
     */
    bool Settings::load()
    {
        QSettings settings;

        QStringList childKeys = settings.allKeys();
        foreach (const QString& childKey, childKeys) {
            setCurrentValue(childKey, settings.value(childKey));
        }

        return true;
    }

    /*!
     * \brief Store settings values.
     *
     * \return True on success, false otherwise.
     */
    bool Settings::save()
    {
        QSettings settings;

        QStringList childKeys = currentSettings.keys();
        foreach (const QString& childKey, childKeys) {
            settings.setValue(childKey, currentValue(childKey));
        }
    }

} // namespace Caneda
