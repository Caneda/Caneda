/***************************************************************************
 * Copyright 2009 Pablo Daniel Pareja Obregon                              *
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

#ifndef SETTINGS_PAGES_H
#define SETTINGS_PAGES_H

#include <QtGui>

class QucsMainWindow;

/**
        This abstract class contains methods that all pages
        configuration must implement.
*/
class SettingsPage : public QWidget {
	Q_OBJECT

	public:
        SettingsPage(QucsMainWindow *parent);
        virtual ~SettingsPage();

        /// Applies the configuration input by the user in the configuration page
	virtual void applyConf() = 0;
	virtual QString title() const = 0;
	virtual QIcon icon() const = 0;

        void setBackgroundColor(QPushButton *, QColor);
        QColor getBackgroundColor(QPushButton *);
        void setForegroundColor(QPushButton *, QColor);
        QColor getForegroundColor(QPushButton *);
};

/**
        This class represents the general configuration page.
*/
class GeneralConfigurationPage : public SettingsPage {
	Q_OBJECT

        public:
        GeneralConfigurationPage(QucsMainWindow *parent = 0);
	virtual ~GeneralConfigurationPage();

        private slots:
//            void slotDefaultValues();
            void slotEditSuffix(QTableWidgetItem*);
            void slotAdd();
            void slotRemove();
            void slotFontDialog();
            void slotBGColorDialog();
	
	public:
	void applyConf();
	QString title() const;
	QIcon icon() const;
	
	public:
        QucsMainWindow *App;

	QLabel *title_label_;
	QFrame *horiz_line_;

        QTableWidget *listSuffix;
        QLineEdit *editUndoNum, *editEditor, *inputSuffix, *inputProgram;
        QFont font;
        QPushButton *buttonFont, *buttonBackground;
        QComboBox *comboLanguage;
};

/**
        This class represents the simulation configuration page.
*/
class SimulationConfigurationPage : public SettingsPage {
        Q_OBJECT

        public:
        SimulationConfigurationPage(QucsMainWindow *parent = 0);
        virtual ~SimulationConfigurationPage();

        public:
        void applyConf();
        QString title() const;
        QIcon icon() const;

        public:
        QLabel *title_label_;
        QFrame *horiz_line_;
};

/**
        This class represents the vhdl configuration page.
*/
class VhdlConfigurationPage : public SettingsPage {
        Q_OBJECT

        public:
        VhdlConfigurationPage(QucsMainWindow *parent = 0);
        virtual ~VhdlConfigurationPage();

        private slots:
//            void slotDefaultValues();
            void slotColorComment();
            void slotColorString();
            void slotColorInteger();
            void slotColorReal();
            void slotColorCharacter();
            void slotColorDataType();
            void slotColorAttributes();

        public:
        void applyConf();
        QString title() const;
        QIcon icon() const;

        public:
        QucsMainWindow *App;

        QLabel *title_label_;
        QFrame *horiz_line_;

        QPushButton *commentButton, *stringButton, *integerButton,
                    *realButton, *characterButton, *dataButton, *attributeButton;
};

#endif
