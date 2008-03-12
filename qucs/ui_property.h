/********************************************************************************
** Form generated from reading ui file 'property.ui'
**
** Created: Wed Mar 12 15:58:11 2008
**      by: Qt User Interface Compiler version 4.3.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_PROPERTY_H
#define UI_PROPERTY_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QLabel>
#include <QtGui/QTableView>
#include <QtGui/QVBoxLayout>

class Ui_Dialog
{
public:
    QVBoxLayout *vboxLayout;
    QLabel *label;
    QTableView *tableView;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *Dialog)
    {
    if (Dialog->objectName().isEmpty())
        Dialog->setObjectName(QString::fromUtf8("Dialog"));
    QSize size(592, 311);
    size = size.expandedTo(Dialog->minimumSizeHint());
    Dialog->resize(size);
    vboxLayout = new QVBoxLayout(Dialog);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    label = new QLabel(Dialog);
    label->setObjectName(QString::fromUtf8("label"));

    vboxLayout->addWidget(label);

    tableView = new QTableView(Dialog);
    tableView->setObjectName(QString::fromUtf8("tableView"));
    tableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    tableView->setAlternatingRowColors(true);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    tableView->setWordWrap(false);
    tableView->setCornerButtonEnabled(false);

    vboxLayout->addWidget(tableView);

    buttonBox = new QDialogButtonBox(Dialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);

    vboxLayout->addWidget(buttonBox);


    retranslateUi(Dialog);
    QObject::connect(buttonBox, SIGNAL(accepted()), Dialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), Dialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
    Dialog->setWindowTitle(QApplication::translate("Dialog", "Dialog", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("Dialog", "Tip: Check the properties to be shown on schematic.", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(Dialog);
    } // retranslateUi

};

namespace Ui {
    class Dialog: public Ui_Dialog {};
} // namespace Ui

#endif // UI_PROPERTY_H
