#include "equipementmainwindow.h"

#include <QLabel>
#include <QVBoxLayout>

EquipementMainWindow::EquipementMainWindow(const employe &e, QWidget *parent)
    : QWidget(parent)
    , m_employeConnecte(e)
{
    auto *layout = new QVBoxLayout(this);
    auto *title = new QLabel("Gestion des equipements", this);
    auto *subtitle = new QLabel("Module temporaire: implementation principale manquante dans ce depot.", this);

    title->setStyleSheet("font-size: 18px; font-weight: 600;");
    subtitle->setWordWrap(true);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addStretch();
}
