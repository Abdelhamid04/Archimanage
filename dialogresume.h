#ifndef DIALOGRESUME_H
#define DIALOGRESUME_H

#include <QDialog>

namespace Ui {
class DialogResume;
}

class DialogResume : public QDialog
{
    Q_OBJECT

public:
    explicit DialogResume(QWidget *parent = nullptr);
    ~DialogResume();
    void setResumeText(const QString &texte);
private:
    Ui::DialogResume *ui;
};

#endif // DIALOGRESUME_H
