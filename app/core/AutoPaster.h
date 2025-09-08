#pragma once

#include <QObject>
#include <QTimer>

class AutoPaster : public QObject {
    Q_OBJECT
public:
    explicit AutoPaster(QObject* parent = nullptr);
    void schedulePaste(int delayMs = 1000);

private slots:
    void doPaste();
};

