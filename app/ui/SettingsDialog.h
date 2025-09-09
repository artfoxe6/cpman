#pragma once

#include <QDialog>
#include <QSize>

class Settings;
class QKeySequenceEdit;
class QSpinBox;
class QCheckBox;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(Settings* settings, QWidget* parent = nullptr);
    void setWindowSizeDisplay(const QSize& sz);

signals:
    void hotkeyChanged(QKeySequence);
    void autoPasteChanged(bool);
    void pasteDelayChanged(int);
    void preloadChanged(int);
    void cleanupRequested(int days);
    void windowSizeChanged(QSize);
    void useCurrentWindowSizeRequested();

private:
    Settings* m_settings;
    QSpinBox* m_spW = nullptr;
    QSpinBox* m_spH = nullptr;
};
