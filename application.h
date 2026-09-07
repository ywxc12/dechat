#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include<boost/beast.hpp>
#include<boost/asio.hpp>
#include<boost/asio/ssl.hpp>
class Application : public QObject
{
    Q_OBJECT
public:
    explicit Application(QObject *parent = nullptr);

signals:
};

#endif // APPLICATION_H
