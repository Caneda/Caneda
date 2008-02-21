/********************************************************************************
** Form generated from reading ui file 'filldialog.ui'
**
** Created: Sun Feb 17 17:11:15 2008
**      by: Qt User Interface Compiler version 4.3.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_FILLDIALOG_H
#define UI_FILLDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>

class Ui_Dialog
{
public:
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QVBoxLayout *vboxLayout1;
    QGroupBox *previewGroupBox;
    QSpacerItem *spacerItem;
    QVBoxLayout *vboxLayout2;
    QGroupBox *arrowGroupBox;
    QGridLayout *gridLayout;
    QLabel *label_5;
    QComboBox *arrowStyleComboBox;
    QLabel *label_7;
    QSpinBox *arrowWidthSpinBox;
    QLabel *label_6;
    QSpinBox *arrowHeightSpinBox;
    QGroupBox *lineGroupBox;
    QGridLayout *gridLayout1;
    QLabel *label;
    QSpinBox *lineWidthSpinBox;
    QLabel *label_2;
    QToolButton *lineColorButton;
    QLabel *label_3;
    QComboBox *lineStyleComboBox;
    QGroupBox *fillGroupBox;
    QGridLayout *gridLayout2;
    QLabel *label_4;
    QToolButton *fillColorButton;
    QLabel *label_8;
    QComboBox *fillStyleComboBox;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *Dialog)
    {
    if (Dialog->objectName().isEmpty())
        Dialog->setObjectName(QString::fromUtf8("Dialog"));
    QSize size(412, 419);
    size = size.expandedTo(Dialog->minimumSizeHint());
    Dialog->resize(size);
    vboxLayout = new QVBoxLayout(Dialog);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setSpacing(3);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    vboxLayout1->setContentsMargins(-1, 5, 2, 2);
    previewGroupBox = new QGroupBox(Dialog);
    previewGroupBox->setObjectName(QString::fromUtf8("previewGroupBox"));
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(previewGroupBox->sizePolicy().hasHeightForWidth());
    previewGroupBox->setSizePolicy(sizePolicy);
    previewGroupBox->setMinimumSize(QSize(150, 150));
    previewGroupBox->setBaseSize(QSize(150, 150));

    vboxLayout1->addWidget(previewGroupBox);

    spacerItem = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

    vboxLayout1->addItem(spacerItem);


    hboxLayout->addLayout(vboxLayout1);

    vboxLayout2 = new QVBoxLayout();
#ifndef Q_OS_MAC
    vboxLayout2->setSpacing(-1);
#endif
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    vboxLayout2->setContentsMargins(2, 5, -1, -1);
    arrowGroupBox = new QGroupBox(Dialog);
    arrowGroupBox->setObjectName(QString::fromUtf8("arrowGroupBox"));
    QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(arrowGroupBox->sizePolicy().hasHeightForWidth());
    arrowGroupBox->setSizePolicy(sizePolicy1);
    gridLayout = new QGridLayout(arrowGroupBox);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    label_5 = new QLabel(arrowGroupBox);
    label_5->setObjectName(QString::fromUtf8("label_5"));

    gridLayout->addWidget(label_5, 0, 0, 1, 1);

    arrowStyleComboBox = new QComboBox(arrowGroupBox);
    arrowStyleComboBox->setObjectName(QString::fromUtf8("arrowStyleComboBox"));

    gridLayout->addWidget(arrowStyleComboBox, 0, 1, 1, 1);

    label_7 = new QLabel(arrowGroupBox);
    label_7->setObjectName(QString::fromUtf8("label_7"));

    gridLayout->addWidget(label_7, 1, 0, 1, 1);

    arrowWidthSpinBox = new QSpinBox(arrowGroupBox);
    arrowWidthSpinBox->setObjectName(QString::fromUtf8("arrowWidthSpinBox"));

    gridLayout->addWidget(arrowWidthSpinBox, 1, 1, 1, 1);

    label_6 = new QLabel(arrowGroupBox);
    label_6->setObjectName(QString::fromUtf8("label_6"));

    gridLayout->addWidget(label_6, 2, 0, 1, 1);

    arrowHeightSpinBox = new QSpinBox(arrowGroupBox);
    arrowHeightSpinBox->setObjectName(QString::fromUtf8("arrowHeightSpinBox"));

    gridLayout->addWidget(arrowHeightSpinBox, 2, 1, 1, 1);


    vboxLayout2->addWidget(arrowGroupBox);

    lineGroupBox = new QGroupBox(Dialog);
    lineGroupBox->setObjectName(QString::fromUtf8("lineGroupBox"));
    sizePolicy1.setHeightForWidth(lineGroupBox->sizePolicy().hasHeightForWidth());
    lineGroupBox->setSizePolicy(sizePolicy1);
    gridLayout1 = new QGridLayout(lineGroupBox);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    label = new QLabel(lineGroupBox);
    label->setObjectName(QString::fromUtf8("label"));

    gridLayout1->addWidget(label, 0, 0, 1, 2);

    lineWidthSpinBox = new QSpinBox(lineGroupBox);
    lineWidthSpinBox->setObjectName(QString::fromUtf8("lineWidthSpinBox"));
    lineWidthSpinBox->setMaximum(20);

    gridLayout1->addWidget(lineWidthSpinBox, 0, 2, 1, 2);

    label_2 = new QLabel(lineGroupBox);
    label_2->setObjectName(QString::fromUtf8("label_2"));

    gridLayout1->addWidget(label_2, 1, 0, 1, 3);

    lineColorButton = new QToolButton(lineGroupBox);
    lineColorButton->setObjectName(QString::fromUtf8("lineColorButton"));
    lineColorButton->setAutoFillBackground(true);

    gridLayout1->addWidget(lineColorButton, 1, 3, 1, 1);

    label_3 = new QLabel(lineGroupBox);
    label_3->setObjectName(QString::fromUtf8("label_3"));

    gridLayout1->addWidget(label_3, 2, 0, 1, 1);

    lineStyleComboBox = new QComboBox(lineGroupBox);
    lineStyleComboBox->setObjectName(QString::fromUtf8("lineStyleComboBox"));

    gridLayout1->addWidget(lineStyleComboBox, 2, 1, 1, 3);


    vboxLayout2->addWidget(lineGroupBox);

    fillGroupBox = new QGroupBox(Dialog);
    fillGroupBox->setObjectName(QString::fromUtf8("fillGroupBox"));
    sizePolicy1.setHeightForWidth(fillGroupBox->sizePolicy().hasHeightForWidth());
    fillGroupBox->setSizePolicy(sizePolicy1);
    gridLayout2 = new QGridLayout(fillGroupBox);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    label_4 = new QLabel(fillGroupBox);
    label_4->setObjectName(QString::fromUtf8("label_4"));

    gridLayout2->addWidget(label_4, 0, 0, 1, 2);

    fillColorButton = new QToolButton(fillGroupBox);
    fillColorButton->setObjectName(QString::fromUtf8("fillColorButton"));
    fillColorButton->setAutoFillBackground(true);

    gridLayout2->addWidget(fillColorButton, 0, 2, 1, 1);

    label_8 = new QLabel(fillGroupBox);
    label_8->setObjectName(QString::fromUtf8("label_8"));

    gridLayout2->addWidget(label_8, 1, 0, 1, 1);

    fillStyleComboBox = new QComboBox(fillGroupBox);
    fillStyleComboBox->setObjectName(QString::fromUtf8("fillStyleComboBox"));

    gridLayout2->addWidget(fillStyleComboBox, 1, 1, 1, 2);


    vboxLayout2->addWidget(fillGroupBox);


    hboxLayout->addLayout(vboxLayout2);


    vboxLayout->addLayout(hboxLayout);

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
    previewGroupBox->setTitle(QApplication::translate("Dialog", "Preview", 0, QApplication::UnicodeUTF8));
    arrowGroupBox->setTitle(QApplication::translate("Dialog", "&Arrow property", 0, QApplication::UnicodeUTF8));
    label_5->setText(QApplication::translate("Dialog", "Head style", 0, QApplication::UnicodeUTF8));
    arrowStyleComboBox->clear();
    arrowStyleComboBox->insertItems(0, QStringList()
     << QApplication::translate("Dialog", "two lines", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "filled", 0, QApplication::UnicodeUTF8)
    );
    label_7->setText(QApplication::translate("Dialog", "Head width", 0, QApplication::UnicodeUTF8));
    label_6->setText(QApplication::translate("Dialog", "Head height", 0, QApplication::UnicodeUTF8));
    lineGroupBox->setTitle(QApplication::translate("Dialog", "&Line Style", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("Dialog", "Width", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("Dialog", "Color", 0, QApplication::UnicodeUTF8));
    lineColorButton->setText(QString());
    label_3->setText(QApplication::translate("Dialog", "Style", 0, QApplication::UnicodeUTF8));
    lineStyleComboBox->clear();
    lineStyleComboBox->insertItems(0, QStringList()
     << QApplication::translate("Dialog", "no line", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "solid line", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "dash line", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "dot line", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "dash dot line", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "dash dot dot line", 0, QApplication::UnicodeUTF8)
    );
    fillGroupBox->setTitle(QApplication::translate("Dialog", "&Filling Style", 0, QApplication::UnicodeUTF8));
    label_4->setText(QApplication::translate("Dialog", "Color", 0, QApplication::UnicodeUTF8));
    fillColorButton->setText(QString());
    label_8->setText(QApplication::translate("Dialog", "Style", 0, QApplication::UnicodeUTF8));
    fillStyleComboBox->clear();
    fillStyleComboBox->insertItems(0, QStringList()
     << QApplication::translate("Dialog", "no fill", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "solid", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "dense1 (densest)", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "dense2", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "dense3", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "dense4", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "dense5", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "dense6", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "dense 7 (least dense)", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "horizontal line", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "vertical line", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "crossed lines", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "hatched backward", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "hatched forward", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("Dialog", "diagonal crossed", 0, QApplication::UnicodeUTF8)
    );
    Q_UNUSED(Dialog);
    } // retranslateUi

};

namespace Ui {
    class Dialog: public Ui_Dialog {};
} // namespace Ui

#endif // UI_FILLDIALOG_H
