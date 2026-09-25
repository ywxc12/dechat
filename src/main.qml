import QtQuick 
import QtQuick.Controls

ApplicationWindow{
    id:mainWindow
    title:qsTr("dechat")
    visible:true
    width:640
    height:480
    Canvas{
        anchors.fill:parent
        onPaint:{
            var cvs=getContext("cvs")
        }
    }
}
/*
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
*/