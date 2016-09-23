﻿// Qt lib import
#include <QCoreApplication>
#include <QtTest>
#include <QtConcurrent>

// JQNetwork lib improt
#include <JQNetworkPackage>
#include <JQNetworkConnect>
#include <JQNetworkServer>
#include <JQNetworkClient>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QtConcurrent::run( []()
    {
        // 创建一个服务端
        const quint16 &&listenPort = 34543;
        auto server = JQNetworkServer::createServerByListenPort( listenPort );

        // 设置接收到数据包后的回调
        server->setOnPackageReceivedCallback( [](const auto &connect, const auto &package)
        {
            qDebug() << "Server: onPackageReceived:" << package->payloadData() << package->randomFlag();

            // 返回一个数据，需要指定 randomFlag 以告知客户端
            connect->replyPayloadData( "Pong", package->randomFlag() );
        } );

        // 初始化服务端
        if ( !server->begin() )
        {
            qDebug() << "Server: begin fail";
            return;
        }

        // 进入事件循环
        QEventLoop eventLoop;
        eventLoop.exec();
    } );

    QtConcurrent::run( []()
    {
        // 创建一个客户端
        auto client = JQNetworkClient::createClient();

        // 创建PingPong测试
        QTimer timer;
        QObject::connect( &timer, &QTimer::timeout, [ client ]()
        {
            // 发送数据，返回0表示失败，其余数表示发送成功
            const auto &&randomFlag = client->sendPayloadData( "127.0.0.1", 34543, "Ping", { } );
            qDebug() << "Client: send randomFlag:" << randomFlag;
        } );

        // 设置连接到服务端后的回调
        client->setOnConnectToHostSucceedCallback( [ &timer ](JQNetworkConnectPointer, const QString &hostName, const quint16 &port)
        {
            qDebug() << "Client: onConnectToHostSucceedCallback:" << hostName << port;
            QMetaObject::invokeMethod( &timer, "start", Q_ARG( int, 1000 ) );
        } );

        // 初始化客户端
        if ( !client->begin() )
        {
            qDebug() << "Client: begin fail";
            return;
        }

        // 创建一个连接，指向某个服务端（创建只需一次，若没连接成功内部会自动维护重连，断开后也会自动重连）
        client->createConnect( "127.0.0.1", 34543 );

        // 进入事件循环
        QEventLoop eventLoop;
        eventLoop.exec();
    } );

    return a.exec();
}
