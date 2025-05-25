#include "mainwindow.h"
#include <QApplication>
#include <QtGlobal>   // For qputenv
#include <QByteArray> // For QByteArray
#include <QDebug>
#include <QGuiApplication> // For platformName()

int main(int argc, char *argv[]) {
    // For high DPI displays, if needed (Qt 6 usually handles this well)
    // QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    // QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // Try native Wayland first for Qt6. Only force xcb if Wayland QVK fails.
    // qputenv("QT_QPA_PLATFORM", QByteArray("xcb"));
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    QApplication app(argc, argv);

    qDebug() << "Application is using QPA platform:" << QGuiApplication::platformName();
    qDebug() << "IM Module should be:" << qgetenv("QT_IM_MODULE").constData();

    MainWindow w;
    w.showFullScreen(); // Show main window in full screen

    return app.exec();
}
