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

    // removed: pause listening checkbox

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

    // Storage stats with open-folder buttons
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_lblDb = new QLabel();
    auto* btnOpenDbDir = new QPushButton(QStringLiteral("打开目录"));
    auto* dbRow = new QHBoxLayout();
    dbRow->addWidget(m_lblDb, 1);
    dbRow->addWidget(btnOpenDbDir);
    v->addLayout(dbRow);

    m_lblMedia = new QLabel();
    auto* btnOpenMediaDir = new QPushButton(QStringLiteral("打开目录"));
    auto* mediaRow = new QHBoxLayout();
    mediaRow->addWidget(m_lblMedia, 1);
    mediaRow->addWidget(btnOpenMediaDir);
    v->addLayout(mediaRow);

    refreshStorageStats();

    // Cleanup controls
    auto* cleanRow = new QHBoxLayout();
    cleanRow->addWidget(new QLabel(QStringLiteral("清理 X 天前:")));
    auto* spDays = new QSpinBox(); spDays->setRange(1, 3650); spDays->setValue(30);
    cleanRow->addWidget(spDays);
    cleanRow->addSpacing(12);
    cleanRow->addWidget(new QLabel(QStringLiteral("跳过使用次数大于:")));
    auto* spUsageSkip = new QSpinBox(); spUsageSkip->setRange(0, 1000000); spUsageSkip->setValue(0);
    spUsageSkip->setToolTip(QStringLiteral("跳过使用次数大于此值的项目；0 表示跳过已使用(≥1次)的项目，仅删除从未使用的旧记录"));
    cleanRow->addWidget(spUsageSkip);
    cleanRow->addWidget(new QLabel(QStringLiteral("次")));
    auto* btnClean = new QPushButton(QStringLiteral("执行清理"));
    cleanRow->addSpacing(12);
    cleanRow->addWidget(btnClean);
    v->addLayout(cleanRow);

    // removed: blacklist editor

    // Wayland note and repo link
    v->addWidget(new QLabel(QStringLiteral("Wayland 环境下自动粘贴需 wtype/ydotool，缺失时仅复制不粘贴。")));
    auto* btnRepo = new QPushButton(QStringLiteral("项目主页"));
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
    connect(btnClean, &QPushButton::clicked, this, [this, spDays, spUsageSkip]{ emit cleanupRequested(spDays->value(), spUsageSkip->value()); });
    connect(btnRepo, &QPushButton::clicked, this, []{ QDesktopServices::openUrl(QUrl("https://github.com/artfoxe6/cpman")); });
    // Open storage directories
    connect(btnOpenDbDir, &QPushButton::clicked, this, [base]{
        // Ensure base dir exists and open its location (DB directory)
        QDir().mkpath(base);
        QDesktopServices::openUrl(QUrl::fromLocalFile(base));
    });
    connect(btnOpenMediaDir, &QPushButton::clicked, this, [base]{
        const QString mediaPath = base + "/media";
        QDir().mkpath(mediaPath);
        QDesktopServices::openUrl(QUrl::fromLocalFile(mediaPath));
    });
    // Theme switching disabled; always follow system, no signal needed
    connect(m_spW, qOverload<int>(&QSpinBox::valueChanged), this, [this](int){ emit windowSizeChanged(QSize(m_spW->value(), m_spH->value())); });
    connect(m_spH, qOverload<int>(&QSpinBox::valueChanged), this, [this](int){ emit windowSizeChanged(QSize(m_spW->value(), m_spH->value())); });
    connect(btnUseCurrent, &QPushButton::clicked, this, [this]{ emit useCurrentWindowSizeRequested(); });

    // no extra persistence on accept
}

void SettingsDialog::refreshStorageStats() {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto sizeToStr = [](qint64 s){
        const char* units[] = {"B","KB","MB","GB"}; int i=0; double d=s; while (d>1024 && i<3){d/=1024; ++i;} return QString::number(d,'f', (i==0?0:1)) + " " + units[i]; };
    // DB
    QFileInfo dbFi(base + "/clipboard.db");
    if (m_lblDb) {
        m_lblDb->setText(QStringLiteral("数据库: %1").arg(dbFi.exists()? sizeToStr(dbFi.size()) : QStringLiteral("(不存在)")));
        m_lblDb->setToolTip(dbFi.absoluteFilePath());
    }
    // Media dir
    const QString mediaPath = base + "/media";
    QDir mediaDir(mediaPath);
    qint64 total=0; if (mediaDir.exists()) {
        QDirIterator it(mediaDir.absolutePath(), QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) { it.next(); total += it.fileInfo().size(); }
    }
    if (m_lblMedia) {
        m_lblMedia->setText(QStringLiteral("媒体: %1").arg(sizeToStr(total)));
        m_lblMedia->setToolTip(mediaDir.absolutePath());
    }
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
