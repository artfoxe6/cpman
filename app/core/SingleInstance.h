#pragma once

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>

class SingleInstance : public QObject {
    Q_OBJECT
public:
    explicit SingleInstance(const QString& key, QObject* parent = nullptr);
    bool ensure();
    void notifyShow();

signals:
    void showRequested();

private slots:
    void onNewConnection();

private:
    QString m_key;
    QLocalServer m_server;
};

