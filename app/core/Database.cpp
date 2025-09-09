#include "Database.h"

#include <QStandardPaths>
#include <QDir>
#include <QSqlError>
#include <QSqlRecord>
#include <QDateTime>
#include <QSet>

Database::Database(QObject* parent)
    : QObject(parent) {}

bool Database::open() {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base);
    m_dbPath = base + "/clipboard.db";

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(m_dbPath);
    if (!m_db.open()) {
        emit error(QStringLiteral("Failed to open DB: %1").arg(m_db.lastError().text()));
        return false;
    }
    return migrate();
}

bool Database::migrate() {
    QSqlQuery q(m_db);
    if (!q.exec("PRAGMA journal_mode=WAL")) {}
    if (!q.exec("PRAGMA foreign_keys=ON")) {}

    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS items (\n"
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
            "  type TEXT CHECK(type IN ('text','image')) NOT NULL,\n"
            "  text TEXT,\n"
            "  media_path TEXT,\n"
            "  mime TEXT,\n"
            "  width INTEGER,\n"
            "  height INTEGER,\n"
            "  hash TEXT,\n"
            "  is_favorite INTEGER DEFAULT 0,\n"
            "  created_at INTEGER NOT NULL,\n"
            "  app_name TEXT,\n"
            "  app_pid INTEGER\n"
            ")")) {
        emit error(q.lastError().text());
        return false;
    }
    q.exec("CREATE INDEX IF NOT EXISTS idx_items_created_at ON items(created_at DESC)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_items_fav ON items(is_favorite)");

    // Ensure new columns exist (schema migrations)
    // Check existing columns
    QSet<QString> cols;
    if (q.exec("PRAGMA table_info(items)")) {
        while (q.next()) cols.insert(q.value(1).toString());
    }
    if (!cols.contains("usage_count")) {
        QSqlQuery qa(m_db);
        if (!qa.exec("ALTER TABLE items ADD COLUMN usage_count INTEGER DEFAULT 0")) {
            // ignore; on older SQLite without ALTER TABLE capabilities or if failed, read as 0 elsewhere
        }
    }

    ensureFts();
    return true;
}

bool Database::ensureFts() {
    QSqlQuery q(m_db);
    // Try to create FTS5 virtual table and triggers; ignore errors if unavailable.
    if (q.exec("CREATE VIRTUAL TABLE IF NOT EXISTS items_fts USING fts5(text, content='items', content_rowid='id', tokenize='unicode61')")) {
        m_hasFts = true;
    } else {
        m_hasFts = false;
    }
    q.exec("CREATE TRIGGER IF NOT EXISTS items_ai AFTER INSERT ON items BEGIN\n"
           "  INSERT INTO items_fts(rowid, text) VALUES (new.id, new.text);\n"
           "END;");
    q.exec("CREATE TRIGGER IF NOT EXISTS items_ad AFTER DELETE ON items BEGIN\n"
           "  INSERT INTO items_fts(items_fts, rowid, text) VALUES('delete', old.id, old.text);\n"
           "END;");
    q.exec("CREATE TRIGGER IF NOT EXISTS items_au AFTER UPDATE ON items BEGIN\n"
           "  INSERT INTO items_fts(items_fts, rowid, text) VALUES('delete', old.id, old.text);\n"
           "  INSERT INTO items_fts(rowid, text) VALUES (new.id, new.text);\n"
           "END;");
    return true;
}

bool Database::insertItem(const HistoryItem& item, qint64* outId) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO items(type, text, media_path, mime, width, height, hash, is_favorite, created_at, app_name, app_pid)\n"
              "VALUES(?,?,?,?,?,?,?,?,?,?,?)");
    q.addBindValue(item.type == ItemType::Text ? QStringLiteral("text") : QStringLiteral("image"));
    q.addBindValue(item.text);
    q.addBindValue(item.mediaPath);
    q.addBindValue(item.mime);
    q.addBindValue(item.width);
    q.addBindValue(item.height);
    q.addBindValue(item.hash);
    q.addBindValue(item.favorite ? 1 : 0);
    q.addBindValue(item.createdAt);
    q.addBindValue(item.appName);
    q.addBindValue(item.appPid);
    if (!q.exec()) {
        emit error(q.lastError().text());
        return false;
    }
    if (outId) *outId = q.lastInsertId().toLongLong();
    return true;
}

HistoryItem Database::fromQuery(const QSqlQuery& q) {
    HistoryItem it;
    it.id = q.value("id").toLongLong();
    const QString typeStr = q.value("type").toString();
    it.type = (typeStr == "image") ? ItemType::Image : ItemType::Text;
    it.text = q.value("text").toString();
    it.mediaPath = q.value("media_path").toString();
    it.mime = q.value("mime").toString();
    it.width = q.value("width").toInt();
    it.height = q.value("height").toInt();
    it.hash = q.value("hash").toString();
    it.favorite = q.value("is_favorite").toInt() != 0;
    it.usageCount = q.record().indexOf("usage_count") >= 0 ? q.value("usage_count").toInt() : 0;
    it.createdAt = q.value("created_at").toLongLong();
    it.appName = q.value("app_name").toString();
    it.appPid = q.value("app_pid").toInt();
    return it;
}

QVector<HistoryItem> Database::fetchRecent(int limit) {
    QVector<HistoryItem> out;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT * FROM items ORDER BY created_at DESC LIMIT %1").arg(limit));
    if (q.exec()) {
        while (q.next()) out.push_back(fromQuery(q));
    }
    return out;
}

QVector<HistoryItem> Database::searchMemoryLike(const QStringList& tokens, bool onlyFav, int limit) {
    // Fallback multi-token LIKE AND combined
    QString sql = "SELECT * FROM items WHERE 1=1";
    if (onlyFav) sql += " AND is_favorite=1";
    for (int i = 0; i < tokens.size(); ++i) {
        sql += QStringLiteral(" AND text LIKE ?");
    }
    sql += QStringLiteral(" ORDER BY created_at DESC LIMIT %1").arg(limit);
    QSqlQuery q(m_db);
    q.prepare(sql);
    for (const auto& t : tokens) q.addBindValue("%" + t + "%");
    QVector<HistoryItem> out;
    if (q.exec()) {
        while (q.next()) out.push_back(fromQuery(q));
    }
    return out;
}

QVector<HistoryItem> Database::searchFts(const QStringList& tokens, bool onlyFav, int limit) {
    if (!m_hasFts || tokens.isEmpty()) {
        return searchMemoryLike(tokens, onlyFav, limit);
    }
    // Build FTS query: tokens joined with AND
    QString match;
    for (int i = 0; i < tokens.size(); ++i) {
        if (i) match += " AND ";
        QString t = tokens[i];
        t.replace('"', "\"");
        match += '"' + t + '"';
    }
    QString sql = "SELECT items.* FROM items_fts JOIN items ON items_fts.rowid = items.id WHERE items_fts MATCH ?";
    if (onlyFav) sql += " AND is_favorite=1";
    sql += QStringLiteral(" ORDER BY created_at DESC LIMIT %1").arg(limit);
    QSqlQuery q(m_db);
    q.prepare(sql);
    q.addBindValue(match);
    QVector<HistoryItem> out;
    if (q.exec()) {
        while (q.next()) out.push_back(fromQuery(q));
    } else {
        // FTS not available or failed, fallback
        out = searchMemoryLike(tokens, onlyFav, limit);
    }
    return out;
}

bool Database::toggleFavorite(qint64 id, bool on) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE items SET is_favorite=? WHERE id=?");
    q.addBindValue(on ? 1 : 0);
    q.addBindValue(id);
    return q.exec();
}

bool Database::incrementUsage(qint64 id) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE items SET usage_count = COALESCE(usage_count,0) + 1 WHERE id=?");
    q.addBindValue(id);
    return q.exec();
}

bool Database::vacuum() {
    QSqlQuery q(m_db);
    return q.exec("VACUUM");
}

bool Database::deleteOlderThan(qint64 cutoffMs, QStringList* outMediaPaths) {
    // Collect media paths first to delete files after DB cleanup
    if (outMediaPaths) outMediaPaths->clear();
    QSqlQuery qsel(m_db);
    qsel.prepare("SELECT media_path FROM items WHERE created_at < ? AND type='image' AND media_path IS NOT NULL");
    qsel.addBindValue(cutoffMs);
    if (qsel.exec()) {
        while (qsel.next()) {
            const QString p = qsel.value(0).toString();
            if (outMediaPaths && !p.isEmpty()) outMediaPaths->push_back(p);
        }
    }
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM items WHERE created_at < ?");
    q.addBindValue(cutoffMs);
    if (!q.exec()) return false;
    return true;
}
