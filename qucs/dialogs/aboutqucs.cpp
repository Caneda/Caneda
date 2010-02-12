/***************************************************************************
 * Copyright 2006-2009 Xavier Guerrin                                      *
 * Copyright 2009 Pablo Daniel Pareja Obregon                              *
 * This file was part of QElectroTech and modified by Pablo Daniel Pareja  *
 * Obregon to be included in Qucs.                                         *
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

#include "aboutqucs.h"

#include "qucs-tools/global.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QTabWidget>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>

/*!
 * Constructor
 * @param QWidget *parent The parent of the dialog.
 */
AboutQUCS::AboutQUCS(QWidget *parent) : QDialog(parent)
{
    // Title, size, behavior...
    setWindowTitle(tr("About Qucs", "window title"));
    setMinimumWidth(680);
    setMinimumHeight(350);
    setModal(true);

    QTabWidget *tabs = new QTabWidget(this);
    tabs->addTab(aboutTab(),        tr("A&bout",             "tab title"));
    tabs->addTab(authorsTab(),      tr("A&uthors",           "tab title"));
    tabs->addTab(translatorsTab(),  tr("&Translators",       "tab title"));
    tabs->addTab(contributorsTab(), tr("&Contributions",     "tab title"));
    tabs->addTab(licenseTab(),      tr("&Licence Agreement", "tab title"));

    // Button to close the dialog box
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(accept()));

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addWidget(title());
    vlayout->addWidget(tabs);
    vlayout->addWidget(buttons);
    setLayout(vlayout);
}

//! Destructor
AboutQUCS::~AboutQUCS()
{
}

//!  @return The title Qucs with its icon
QWidget *AboutQUCS::title() const
{
    QWidget *icon_and_title = new QWidget();
    // icon
    QLabel *icon = new QLabel();
    icon->setPixmap(QPixmap(Qucs::bitmapDirectory() + "qucs.png"));
    // label "Qucs"
    QLabel *title = new QLabel("<span style=\"font-weight:0;font-size:16pt;\">Qucs v"
            + Qucs::version + "</span>");
    title->setTextFormat(Qt::RichText);
    // All in a grid
    QGridLayout *grid_layout = new QGridLayout();
    grid_layout->addWidget(icon, 0, 0);
    grid_layout->addWidget(title, 0, 1);
    grid_layout->setColumnStretch(0, 1);
    grid_layout->setColumnStretch(1, 100);
    icon_and_title->setLayout(grid_layout);
    return icon_and_title;
}

//! @return The widget contained by the tab "About"
QWidget *AboutQUCS::aboutTab() const
{
    QLabel *about = new QLabel(
            tr("Qucs - Quite Universal Circuit Simulator") +
            "<br>" +
            tr("An application for eletric schematics editing and simulation.") +
            "<br><br>" +
            tr("\251 2008-2010 Qucs developer team") +
            "<br><br>"
            "<a href=\"http://qucs.sourceforge.net/\">"
            "http://qucs.sourceforge.net/</a>"
            );
    about->setAlignment(Qt::AlignCenter);
    about->setOpenExternalLinks(true);
    about->setTextFormat(Qt::RichText);
    return about;
}

//! @return The widget contained by the tab "Authors"
QWidget *AboutQUCS::authorsTab() const
{
    QLabel *authors = new QLabel();
    addAuthor(authors, "Stefan Jahn", "stefan@lkcc.org", tr("Original idea"));
    addAuthor(authors, "Bastien Roucaries", "roucaries.bastien@gmail.com",
            tr("Programming"));
    addAuthor(authors, "Gopala Krishna", "krishna.ggk@gmail.com", tr("Programming"));
    addAuthor(authors, "Pablo Daniel Pareja Obregon", "parejaobregon@gmail.com",
            tr("Programming"));
    authors->setAlignment(Qt::AlignCenter);
    authors->setOpenExternalLinks(true);
    authors->setTextFormat(Qt::RichText);
    return authors;
}

//! @return The widget contained by the tab "Translators"
QWidget *AboutQUCS::translatorsTab() const
{
    QLabel *translators = new QLabel();

    addAuthor(translators, "Pablo Daniel Pareja Obregon", "parejaobregon@gmail.com",
            tr("Spanish translation"));

    translators->setAlignment(Qt::AlignCenter);
    translators->setOpenExternalLinks(true);
    translators->setTextFormat(Qt::RichText);
    return translators;
}

//! @return The widget contained by the tab "Contributions"
QWidget *AboutQUCS::contributorsTab() const
{
    QLabel *contributors = new QLabel();

    // addAuthor(contributors, "Name Lastname", "mail@domain.com", tr("Whatever"));

    contributors->setAlignment(Qt::AlignCenter);
    contributors->setOpenExternalLinks(true);
    contributors->setTextFormat(Qt::RichText);
    return contributors;
}

//! @return The widget contained by the tab "Licence Agreement"
QWidget *AboutQUCS::licenseTab() const
{
    QWidget *license = new QWidget();
    // label
    QLabel *title_license =
        new QLabel(tr("This program is licensed under the GNU/GPL v2."));

    // Text of the GNU/GPL v2 in a scrollable non-editable text box


    QFile *file = new QFile(Qucs::baseDir + "COPYING");
    QString text;
    // Verification that the file exists
    if(!file->exists()) {
        text = QString(QObject::tr("The text file that contains the GNU/GPL is not found."));
    }
    else if(!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        text = QString(QObject::tr("The text file that contains the GNU/GPL exists but could not be opened."));
    }
    else {
        QTextStream in(file);
        text = QString("");
        while(!in.atEnd()) {
            text += in.readLine() + QChar('\n');
        }
        file->close();
    }

    QTextEdit *text_license = new QTextEdit();
    text_license->setPlainText(text);
    text_license->setReadOnly(true);

    QVBoxLayout *license_layout = new QVBoxLayout();
    license_layout->addWidget(title_license);
    license_layout->addWidget(text_license);
    license->setLayout(license_layout);
    return license;
}

/*!
 * Adding someone to the list of authors
 * @param QLabel *label  Label which will add the person
 * @param QString &name  Name of the person
 * @param QString &email Email address of the person
 * @param QString &work  Function/work done by the person
 */
void AboutQUCS::addAuthor(QLabel *label, const QString &name, const QString &email,
        const QString &work) const
{
    QString new_text = label->text();

    QString author_template =
        "<span style=\"text-decoration: underline;\">%1</span> : %2 &lt;<a href=\"mailto:%3\">%3</a>&gt;<br><br>";

    // adds the title of the person
    new_text += author_template.arg(work).arg(name).arg(email);
    label->setText(new_text);
}
