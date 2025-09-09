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
#include <QSize>

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

    // Theme mode selector removed: always follow system theme

    // Default popup size
    auto* sizeRow = new QHBoxLayout();
    sizeRow->addWidget(new QLabel(QStringLiteral("默认窗口大小 (宽×高):")));
    m_spW = new QSpinBox();
    m_spH = new QSpinBox();
    m_spW->setRange(400, 3840);
    m_spH->setRange(300, 2160);
    const QSize storedSize = m_settings->popupSize();
    const QSize initialSize = storedSize.isValid() ? storedSize : QSize(960, 600);
    m_spW->setValue(initialSize.width());
    m_spH->setValue(initialSize.height());
    sizeRow->addWidget(m_spW);
    sizeRow->addWidget(new QLabel(QStringLiteral("×")));
    sizeRow->addWidget(m_spH);
    auto* btnUseCurrent = new QPushButton(QStringLiteral("使用当前窗口大小为默认值"));
    sizeRow->addSpacing(8);
    sizeRow->addWidget(btnUseCurrent);
    v->addLayout(sizeRow);

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
    // Theme switching disabled; always follow system, no signal needed
    connect(m_spW, qOverload<int>(&QSpinBox::valueChanged), this, [this](int){ emit windowSizeChanged(QSize(m_spW->value(), m_spH->value())); });
    connect(m_spH, qOverload<int>(&QSpinBox::valueChanged), this, [this](int){ emit windowSizeChanged(QSize(m_spW->value(), m_spH->value())); });
    connect(btnUseCurrent, &QPushButton::clicked, this, [this]{ emit useCurrentWindowSizeRequested(); });

    // no extra persistence on accept
}

void SettingsDialog::setWindowSizeDisplay(const QSize& sz) {
    if (!m_spW || !m_spH) return;
    if (!sz.isValid()) return;
    // Block signals to avoid re-emitting windowSizeChanged while reflecting programmatic update
    const QSignalBlocker b1(m_spW);
    const QSignalBlocker b2(m_spH);
    m_spW->setValue(sz.width());
    m_spH->setValue(sz.height());
}
