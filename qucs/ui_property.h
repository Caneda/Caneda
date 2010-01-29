/********************************************************************************
** Form generated from reading ui file 'property.ui'
**
** Created: Wed Mar 12 20:13:24 2008
**      by: Qt User Interface Compiler version 4.3.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_PROPERTY_H
#define UI_PROPERTY_H

#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>

class Ui_PropertyDialogBase
{
public:
    QVBoxLayout *vboxLayout;
    QLabel *label;
    QTableView *tableView;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *PropertyDialogBase)
    {
    if(PropertyDialogBase->objectName().isEmpty())
        PropertyDialogBase->setObjectName(QString::fromUtf8("PropertyDialogBase"));
    QSize size(592, 311);
    size = size.expandedTo(PropertyDialogBase->minimumSizeHint());
    PropertyDialogBase->resize(size);
    vboxLayout = new QVBoxLayout(PropertyDialogBase);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    label = new QLabel(PropertyDialogBase);
    label->setObjectName(QString::fromUtf8("label"));

    vboxLayout->addWidget(label);

    tableView = new QTableView(PropertyDialogBase);
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

    buttonBox = new QDialogButtonBox(PropertyDialogBase);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);

    vboxLayout->addWidget(buttonBox);


    retranslateUi(PropertyDialogBase);
    QObject::connect(buttonBox, SIGNAL(accepted()), PropertyDialogBase, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), PropertyDialogBase, SLOT(reject()));

    QMetaObject::connectSlotsByName(PropertyDialogBase);
    } // setupUi

    void retranslateUi(QDialog *PropertyDialogBase)
    {
    PropertyDialogBase->setWindowTitle(QApplication::translate("PropertyDialogBase", "Edit Component properties", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("PropertyDialogBase", "Tip: Check the properties to be shown on schematic.", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(PropertyDialogBase);
    } // retranslateUi

};

namespace Ui {
    class PropertyDialogBase: public Ui_PropertyDialogBase {};
} // namespace Ui

#endif // UI_PROPERTY_H
