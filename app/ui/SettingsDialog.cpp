#include "SettingsDialog.h"
#include "../core/Settings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QKeySequenceEdit>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QDirIterator>
#include <QStandardPaths>
#include <QComboBox>

SettingsDialog::SettingsDialog(Settings* settings, QWidget* parent)
    : QDialog(parent), m_settings(settings) {
    setWindowTitle(QStringLiteral("设置"));
    auto* v = new QVBoxLayout(this);

    // Hotkey
    auto* hotRow = new QHBoxLayout();
    hotRow->addWidget(new QLabel(QStringLiteral("全局快捷键:")));
    auto* keyEdit = new QKeySequenceEdit(m_settings->hotkey());
    hotRow->addWidget(keyEdit, 1);
    v->addLayout(hotRow);

    // Auto paste
    auto* apRow = new QHBoxLayout();
    auto* chkAuto = new QCheckBox(QStringLiteral("自动粘贴"));
    chkAuto->setChecked(m_settings->autoPaste());
    apRow->addWidget(chkAuto);
    apRow->addSpacing(12);
    apRow->addWidget(new QLabel(QStringLiteral("粘贴延时 (ms, 0-5000):")));
    auto* spDelay = new QSpinBox();
    spDelay->setRange(0, 5000);
    spDelay->setSingleStep(50);
    spDelay->setValue(m_settings->pasteDelayMs());
    spDelay->setEnabled(m_settings->autoPaste());
    apRow->addWidget(spDelay);
    v->addLayout(apRow);

    // Preload count
    auto* preRow = new QHBoxLayout();
    preRow->addWidget(new QLabel(QStringLiteral("预加载条数 (200-5000):")));
    auto* spPreload = new QSpinBox();
    spPreload->setRange(200, 5000);
    spPreload->setValue(m_settings->preloadCount());
    preRow->addWidget(spPreload);
    v->addLayout(preRow);

    // Pause
    auto* chkPause = new QCheckBox(QStringLiteral("暂停监听"));
    chkPause->setChecked(m_settings->paused());
    v->addWidget(chkPause);

    // Theme mode
    auto* themeRow = new QHBoxLayout();
    themeRow->addWidget(new QLabel(QStringLiteral("主题:")));
    auto* cbTheme = new QComboBox();
    cbTheme->addItem(QStringLiteral("系统"), "system");
    cbTheme->addItem(QStringLiteral("浅色"), "light");
    cbTheme->addItem(QStringLiteral("深色"), "dark");
    int themeIndex = cbTheme->findData(m_settings->themeMode());
    if (themeIndex >= 0) cbTheme->setCurrentIndex(themeIndex);
    themeRow->addWidget(cbTheme);
    v->addLayout(themeRow);

    // Storage stats
    auto* lblDb = new QLabel();
    auto* lblMedia = new QLabel();
    auto refreshStats = [&, this]() {
        // Compute sizes
        // DB path is determined by QStandardPaths inside Database; recompute here via QStandardPaths as well
        QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QFileInfo dbFi(base + "/clipboard.db");
        auto sizeToStr = [](qint64 s){
            const char* units[] = {"B","KB","MB","GB"}; int i=0; double d=s; while (d>1024 && i<3){d/=1024; ++i;} return QString::number(d,'f', (i==0?0:1)) + " " + units[i]; };
        lblDb->setText(QStringLiteral("数据库: %1").arg(dbFi.exists()? sizeToStr(dbFi.size()) : QStringLiteral("(不存在)")));
        // Media dir size
        QDir mediaDir(base + "/media");
        qint64 total=0; if (mediaDir.exists()) {
            QDirIterator it(mediaDir.absolutePath(), QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) { it.next(); total += it.fileInfo().size(); }
        }
        lblMedia->setText(QStringLiteral("媒体: %1").arg(sizeToStr(total)));
    };
    v->addWidget(lblDb);
    v->addWidget(lblMedia);
    refreshStats();

    // Cleanup controls
    auto* cleanRow = new QHBoxLayout();
    cleanRow->addWidget(new QLabel(QStringLiteral("清理 X 天前:")));
    auto* spDays = new QSpinBox(); spDays->setRange(1, 3650); spDays->setValue(30);
    auto* btnClean = new QPushButton(QStringLiteral("执行清理"));
    cleanRow->addWidget(spDays);
    cleanRow->addWidget(btnClean);
    v->addLayout(cleanRow);

    // removed: blacklist editor

    // Wayland note and repo link
    v->addWidget(new QLabel(QStringLiteral("Wayland 环境下自动粘贴需 wtype/ydotool，缺失时仅复制不粘贴。")));
    auto* btnRepo = new QPushButton(QStringLiteral("打开开源地址"));
    v->addWidget(btnRepo);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    v->addWidget(box);

    // Signals
    connect(keyEdit, &QKeySequenceEdit::keySequenceChanged, this, [this](const QKeySequence& ks){ emit hotkeyChanged(ks); });
    connect(chkAuto, &QCheckBox::toggled, this, [this, spDelay](bool on){ spDelay->setEnabled(on); emit autoPasteChanged(on); });
    connect(spDelay, qOverload<int>(&QSpinBox::valueChanged), this, [this](int ms){ emit pasteDelayChanged(ms); });
    connect(spPreload, qOverload<int>(&QSpinBox::valueChanged), this, [this](int n){ emit preloadChanged(n); });
    connect(chkPause, &QCheckBox::toggled, this, [this](bool on){ emit pausedChanged(on); });
    connect(btnClean, &QPushButton::clicked, this, [this, spDays]{ emit cleanupRequested(spDays->value()); });
    connect(btnRepo, &QPushButton::clicked, this, []{ QDesktopServices::openUrl(QUrl("https://github.com/xxxx/xxxx")); });
    connect(cbTheme, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, cbTheme](int){
        m_settings->setThemeMode(cbTheme->currentData().toString());
    });

    // no extra persistence on accept
}
