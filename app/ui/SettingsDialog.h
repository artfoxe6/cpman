#pragma once

#include <QDialog>

class Settings;
class QKeySequenceEdit;
class QSpinBox;
class QCheckBox;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(Settings* settings, QWidget* parent = nullptr);

signals:
    void hotkeyChanged(QKeySequence);
    void autoPasteChanged(bool);
    void pasteDelayChanged(int);
    void preloadChanged(int);
    void pausedChanged(bool);
    void cleanupRequested(int days);

private:
    Settings* m_settings;
};
