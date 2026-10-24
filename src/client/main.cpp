#include <QDebug>
#include <qguiapplication.h>
#include <qlogging.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <qobject.h>
#include <qqmlapplicationengine.h>
#include <nlohmann/json.hpp>

int main(int argc,char**argv){
    qDebug()<<"running...";
    QGuiApplication app(argc,argv);
    QQmlApplicationEngine engine;
    QObject::connect(&engine,
        &QQmlApplicationEngine::objectCreationFailed,
      [&app](){
        qFatal()<<"QQmlApplicationEngine::objectCreationFailed emited";
        app.exit(-1);
      });
    engine.load("./main.qml");
    return app.exec();
}