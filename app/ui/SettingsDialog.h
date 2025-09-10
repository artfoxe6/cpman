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
    void refreshStorageStats();

signals:
    void hotkeyChanged(QKeySequence);
    void autoPasteChanged(bool);
    void pasteDelayChanged(int);
    void preloadChanged(int);
    // days: older-than days; usageSkipGreaterThan: keep items with usage_count > this (set -1 to disable)
    void cleanupRequested(int days, int usageSkipGreaterThan);
    void windowSizeChanged(QSize);
    void useCurrentWindowSizeRequested();

private:
    Settings* m_settings;
    QSpinBox* m_spW = nullptr;
    QSpinBox* m_spH = nullptr;
    // For storage stats refresh
    class QLabel* m_lblDb = nullptr;
    class QLabel* m_lblMedia = nullptr;
};
