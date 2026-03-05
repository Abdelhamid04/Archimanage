#-------------------------------------------------

#
# Project created by QtCreator 2018-10-26T21:45:23
#
#-------------------------------------------------


QT += core gui sql printsupport widgets charts #sahar
QT += network

CONFIG += c++17  # Obligatoire pour Qt 6

# Ligne CRUCIALE pour Qt 6
DEFINES += QT_CHARTS_LIB

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += charts
QT += axcontainer
QT += printsupport
QT += core gui network
QT += serialport

TARGET = Atelier_Connexion
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11


SOURCES += \
    animatedsplash.cpp \
    arduino.cpp \
    captchadialog.cpp \
    chatbotemploye.cpp \
    client.cpp \
    clientmainwindow.cpp \
    connection.cpp \
    designs.cpp \
    designsmainwindow.cpp \
    dialogresume.cpp \
    employe.cpp \
    employemainwindow.cpp \
    equipementmainwindow.cpp \
    login.cpp \
    main.cpp \
    mainwindow.cpp \
    motdepasseoublie.cpp \
    partenaire.cpp \
    partenairemainwindow.cpp \
    projet.cpp \
    projetmainwindow.cpp

HEADERS += \
    animatedsplash.h \
    arduino.h \
    captchadialog.h \
    chatbotemploye.h \
    client.h \
    clientmainwindow.h \
    connection.h \
    designs.h \
    designsmainwindow.h \
    dialogresume.h \
    employe.h \
    employemainwindow.h \
    equipementmainwindow.h \
    login.h \
    mainwindow.h \
    motdepasseoublie.h \
    partenaire.h \
    partenairemainwindow.h \
    projet.h \
    projetmainwindow.h


FORMS += \
        MotDePasseOublie.ui \
        captchaDialog.ui \
        clientmainwindow.ui \
        designsmainwindow.ui \
        dialogresume.ui \
        employemainwindow.ui \
        login.ui \
        mainwindow.ui \
        partenairemainwindow.ui \
        projetmainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    modele_defaillance.pkl \
    predict_surveillance.py \
    scaler.pkl

RESOURCES += \
    resources.qrc \
    resources.qrc
