#include "helpdialog.h"
#include <QtGui/QTextEdit>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>

HelpDialog::HelpDialog(const QString& cap,QWidget *p) : QDialog(p)
{
  setWindowTitle(cap);
  QVBoxLayout *l = new QVBoxLayout(this);
  edit = new QTextEdit();
  l->addWidget(edit);
  QHBoxLayout *h = new QHBoxLayout();
  l->addLayout(h);
  h->addStretch(5);
  QPushButton *pb = new QPushButton(tr("Dismiss"));
  pb->setFocus();
  h->addWidget(pb);
  h->addStretch(5);
  connect(pb,SIGNAL(clicked()),this,SLOT(accept()));
}

void HelpDialog::setText(const QString &text)
{
  edit->setPlainText(text);
}
