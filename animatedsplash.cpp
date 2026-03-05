#include "animatedsplash.h"
#include <QVBoxLayout>
#include <QTimer>
#include <QScreen> // Ajouter pour accéder à la géométrie de l'écran
#include <QGuiApplication> // Ajouter pour QGuiApplication

AnimatedSplash::AnimatedSplash(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
    setAttribute(Qt::WA_TranslucentBackground);

    label = new QLabel(this);
    movie = new QMovie(":/loader.gif");
    label->setMovie(movie);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label, 0, Qt::AlignCenter);
    setLayout(layout);

    // Ajuster la taille selon la taille du GIF
    resize(movie->frameRect().size());

    // Centrer la fenêtre
    moveToCenter();
}

void AnimatedSplash::moveToCenter()
{
    // Obtenir la géométrie de l'écran principal
    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();

    // Calculer la position centrale
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;

    // Déplacer la fenêtre
    move(x, y);
}

void AnimatedSplash::startAnimation() {
    movie->start();
    show();
}

void AnimatedSplash::stopAnimation() {
    movie->stop();
    close();
}
