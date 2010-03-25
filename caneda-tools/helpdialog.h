#include <QtGui/QDialog>

class QTextEdit;

class HelpDialog : public QDialog
{
 public:
  HelpDialog(const QString &cap,QWidget *p=0l);
  ~HelpDialog() {}
  void setText(const QString &text);

 private:
  QTextEdit *edit;
};
