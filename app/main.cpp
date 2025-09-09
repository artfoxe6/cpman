#include <QApplication>
#include <QTimer>
#include <QScreen>
#include <QClipboard>
#include <QListView>
#include <QDateTime>
#include <algorithm>

#include "core/Settings.h"
#include "core/Database.h"
#include "core/InMemoryStore.h"
#include "core/ImageStore.h"
#include "core/ClipboardWatcher.h"
#include "core/HotkeyManager.h"
#include "core/AutoPaster.h"
#include "core/SingleInstance.h"
#include "core/FocusManager.h"

#include "ui/TrayIcon.h"
#include "ui/MainPopup.h"
#include "ui/HistoryListModel.h"
#include "ui/HistoryItemDelegate.h"
#include "ui/SettingsDialog.h"
#include "ui/PreviewPane.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("cpman");
    QApplication::setOrganizationName("cpman");
    // This is a tray-first app; do not quit when dialogs close
    QApplication::setQuitOnLastWindowClosed(false);

    // Single instance
    SingleInstance instance("cpman.single");
    if (!instance.ensure()) {
        instance.notifyShow();
        return 0;
    }

    Settings settings;
    Database db;
    // Log DB errors to stderr to aid diagnosis if open/migrate/inserts fail
    QObject::connect(&db, &Database::error, [](const QString& msg){ qWarning() << "DB error:" << msg; });
    if (!db.open()) {
        // proceed with limited functionality
    }
    ImageStore imageStore; imageStore.ensureDirs();
    InMemoryStore mem;
    mem.setCapacity(std::clamp(settings.preloadCount(), 200, 5000));
    mem.preload(db.fetchRecent(mem.capacity()));

    ClipboardWatcher watcher(&db, &mem, &imageStore, &settings);
    watcher.start();
    // Apply initial paused state from settings
    watcher.setPaused(settings.paused());

    TrayIcon tray;
    tray.attachSettings(&settings);
    tray.show();
    // Reflect initial paused state in tray menu label
    tray.setPaused(settings.paused());

    MainPopup popup;
    FocusManager focus;
    popup.attachSettings(&settings);
    HistoryListModel model;
    model.setItems(mem.items());
    popup.setListModel(&model);
    HistoryItemDelegate* delegate = new HistoryItemDelegate(&popup);
    // Assign via object name lookup; MainPopup exposes list indirectly; set via view pointer not public, so fetch child
    // We can set it by finding the QListView child.
    auto listViews = popup.findChildren<QListView*>();
    if (!listViews.isEmpty()) listViews.first()->setItemDelegate(delegate);
    QObject::connect(&mem, &InMemoryStore::itemsChanged, [&]{ model.setItems(mem.items()); });

    QObject::connect(&tray, &TrayIcon::togglePopupRequested, [&]{
        if (popup.isVisible()) popup.hidePopup();
        else { focus.rememberForeground(); popup.showPopup(); }
    });
    QObject::connect(&tray, &TrayIcon::pauseToggled, &watcher, &ClipboardWatcher::setPaused);
    QObject::connect(&tray, &TrayIcon::quitRequested, &app, &QApplication::quit);
    QObject::connect(&instance, &SingleInstance::showRequested, [&]{ focus.rememberForeground(); popup.showPopup(); });

    // Hotkey (placeholder)
    HotkeyManager hotkey;
    if (!hotkey.registerHotkey(settings.hotkey())) {
        // Fallback: no global hotkey; user can use tray icon.
    }
    QObject::connect(&hotkey, &HotkeyManager::triggered, [&]{ focus.rememberForeground(); popup.showPopup(); });

    AutoPaster autoPaster;

    auto doSearch = [&](const QString& query, bool useDb, bool onlyFav){
        const int limit = mem.capacity();
        QStringList tokens;
        for (const auto& t : query.split(' ', Qt::SkipEmptyParts)) tokens << t.trimmed();
        if (!useDb && tokens.isEmpty() && !onlyFav) { model.setItems(mem.items()); return; }
        if (useDb) model.setItems(db.searchFts(tokens, onlyFav, limit));
        else model.setItems(mem.filterMemory(tokens, onlyFav));
    };
    QObject::connect(&popup, &MainPopup::requestSearch,
                     [&](QString query, bool useDb, bool onlyFav){ doSearch(query, useDb, onlyFav); });

    // Commit: copy selected and optionally auto-paste
    QObject::connect(&popup, &MainPopup::commitRequested, [&]{
        QModelIndex idx = popup.currentIndex();
        if (!idx.isValid()) {
            if (model.rowCount() == 0) return;
            idx = model.index(0, 0);
            popup.setCurrentIndex(idx);
        }
        const qint64 id = model.data(idx, HistoryListModel::IdRole).toLongLong();
        const QString type = model.data(idx, HistoryListModel::TypeRole).toString();
        if (type == "text") {
            const QString text = model.data(idx, HistoryListModel::TextRole).toString();
            QGuiApplication::clipboard()->setText(text);
        } else {
            const QString path = model.data(idx, HistoryListModel::MediaPathRole).toString();
            QImage img(path);
            if (!img.isNull()) QGuiApplication::clipboard()->setImage(img);
        }
        popup.hidePopup();
        // Try to restore focus to the previously active app before autopaste
        focus.restoreForeground();
        if (settings.autoPaste()) autoPaster.schedulePaste(settings.pasteDelayMs());
        // increment usage count
        db.incrementUsage(id);
        mem.incrementUsageById(id);
    });

    // When popup hides without commit (Esc/click outside), also restore focus.
    QObject::connect(&popup, &MainPopup::popupHidden, [&]{ focus.restoreForeground(); });

    // Favorite toggle from preview heart
    QObject::connect(popup.previewPane(), &PreviewPane::favoriteToggled,
                     [&](qint64 id, bool on){
                         db.toggleFavorite(id, on);
                         mem.setFavoriteById(id, on);
                         // refresh current view per filters
                         doSearch(popup.queryText(), popup.useDbChecked(), popup.onlyFavChecked());
                     });

    // removed: tray add-foreground-app-to-blacklist wiring

    // Settings dialog wiring
    SettingsDialog settingsDlg(&settings);
    QObject::connect(&tray, &TrayIcon::settingsRequested, [&]{ settingsDlg.exec(); });
    QObject::connect(&settingsDlg, &SettingsDialog::hotkeyChanged, [&](QKeySequence ks){ settings.setHotkey(ks); hotkey.unregister(); hotkey.registerHotkey(ks); });
    QObject::connect(&settingsDlg, &SettingsDialog::autoPasteChanged, [&](bool on){ settings.setAutoPaste(on); });
    QObject::connect(&settingsDlg, &SettingsDialog::preloadChanged, [&](int n){ settings.setPreloadCount(n); mem.setCapacity(std::clamp(n, 200, 5000)); mem.preload(db.fetchRecent(mem.capacity())); model.setItems(mem.items()); });
    QObject::connect(&settingsDlg, &SettingsDialog::pasteDelayChanged, [&](int ms){ settings.setPasteDelayMs(ms); });
    QObject::connect(&settingsDlg, &SettingsDialog::pausedChanged, [&](bool on){ settings.setPaused(on); watcher.setPaused(on); tray.setPaused(on); });
    QObject::connect(&settingsDlg, &SettingsDialog::cleanupRequested, [&](int days){
        const qint64 cutoff = QDateTime::currentMSecsSinceEpoch() - qint64(days) * 24 * 3600 * 1000;
        QStringList mediaToRemove;
        if (db.deleteOlderThan(cutoff, &mediaToRemove)) {
            imageStore.removeMediaFiles(mediaToRemove);
            db.vacuum();
            // Reload memory set
            mem.preload(db.fetchRecent(mem.capacity()));
            model.setItems(mem.items());
        }
    });
    QObject::connect(&settingsDlg, &SettingsDialog::windowSizeChanged, [&](QSize sz){
        settings.setPopupSize(sz);
        popup.applyDefaultSize(sz);
    });
    QObject::connect(&settingsDlg, &SettingsDialog::useCurrentWindowSizeRequested, [&]{
        const QSize sz = popup.size();
        settings.setPopupSize(sz);
        popup.applyDefaultSize(sz);
        settingsDlg.setWindowSizeDisplay(sz);
    });

    return app.exec();
}
