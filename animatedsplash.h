#ifndef ANIMATEDSPLASH_H
#define ANIMATEDSPLASH_H

#include <QWidget>
#include <QMovie>
#include <QLabel>

class AnimatedSplash : public QWidget
{
    Q_OBJECT

public:
    explicit AnimatedSplash(QWidget *parent = nullptr);
    void startAnimation();
    void stopAnimation();


private:
    void moveToCenter();
    QLabel *label;
    QMovie *movie;
};

#endif // ANIMATEDSPLASH_H
