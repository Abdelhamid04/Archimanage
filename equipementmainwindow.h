#ifndef EQUIPEMENTMAINWINDOW_H
#define EQUIPEMENTMAINWINDOW_H

#include <QWidget>
#include "employe.h"

class EquipementMainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit EquipementMainWindow(const employe &e, QWidget *parent = nullptr);

private:
    employe m_employeConnecte;
};

#endif // EQUIPEMENTMAINWINDOW_H
