#include "SingleInstance.h"

#include <QCoreApplication>
#include <QFile>

SingleInstance::SingleInstance(const QString& key, QObject* parent)
    : QObject(parent), m_key(key) {}

bool SingleInstance::ensure() {
    // Remove stale socket
    QLocalSocket socket;
    socket.connectToServer(m_key);
    if (socket.waitForConnected(100)) {
        // another instance exists
        return false;
    } else {
        // setup server
        QLocalServer::removeServer(m_key);
        if (!m_server.listen(m_key)) {
            return false;
        }
        connect(&m_server, &QLocalServer::newConnection, this, &SingleInstance::onNewConnection);
        return true;
    }
}

void SingleInstance::notifyShow() {
    QLocalSocket sock;
    sock.connectToServer(m_key);
    if (sock.waitForConnected(200)) {
        sock.write("ShowPopup", 9);
        sock.flush();
        sock.waitForBytesWritten(100);
        sock.disconnectFromServer();
    }
}

void SingleInstance::onNewConnection() {
    if (auto* s = m_server.nextPendingConnection()) {
        s->waitForReadyRead(50);
        const QByteArray msg = s->readAll();
        if (msg == "ShowPopup") emit showRequested();
        s->close();
        s->deleteLater();
    }
}

