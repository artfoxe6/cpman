# CLAUDE.md

> Qt6 跨平台剪贴板历史工具  
> 目标平台：macOS / Windows / Linux（X11/Wayland）  
> UI 框架：Qt Widgets（跟随系统深浅色）  
> 全局快捷键：默认 `Ctrl/⌘ + Shift + M`（qhotkey，可配置）  
> 本文为交付给代码生成代理（如 Claude Code）的执行说明：精确定义范围、行为、接口、数据结构、依赖与测试。

---

## 1. 产品概述与范围

- **驻留托盘**：启动后仅显示系统托盘图标；不弹出主窗口。

- **聚焦式浮窗**：通过全局快捷键呼出一个 Spotlight 风格的无边框置顶浮窗；点击窗口外或按 `Esc` 关闭；支持拖拽移动。

- **剪贴板监听**：仅记录 `text/plain` 与 `image/*`；忽略其他类型；**消除连续重复**；写入 SQLite；图片原图落盘、DB 仅存路径与元信息。

- **预加载**：启动后预加载最近 **1000** 条（可配置）到内存（文本+元数据+缩略图）用于快速搜索。

- **搜索**：搜索框空格分词、**AND** 逻辑；“数据库”复选框=切换为 SQLite（FTS5 优先）；“收藏列表”= 仅显示 `is_favorite=1`。

- **交互**：焦点在搜索框时也可用 `↑/↓/PgUp/PgDn/Home/End` 导航列表；预览右上角爱心切换收藏；`Enter` 复制选中项→隐藏浮窗→**1000ms 后自动粘贴**（平台差异详见 §9）。

- **设置**：修改全局快捷键；启用/禁用自动粘贴；配置预加载上限；显示 DB/媒体占用并可清理历史；开源地址；“暂停监听”。

- **主题**：自动跟随系统深/浅色；托盘与 UI 图标自适应。

- **单实例**：重复启动时激活既有实例并显示浮窗。

---

## 2. 详细规格

### 2.1 托盘与启动流程

- 托盘菜单：
  
  1. 打开/关闭浮窗
  
  2. 设置…
  
  3. 暂停/恢复监听
  
  4. 退出

- 默认不显示任何窗口；注册全局快捷键；启动剪贴板监听；预加载内存索引。

### 2.2 浮窗 UI 布局

- **顶部**
  
  - 搜索输入框（无按钮，实时筛选）
  
  - 两个勾选框：`数据库`、`收藏列表`

- **底部**（左右分栏）
  
  - 左：历史列表（图文混排，显示缩略图/文本摘要；**仅垂直滚动**，超长文本省略号并提供 tooltip）
  
  - 右：预览窗（`QScrollArea`）
    
    - 文本：自动换行，可选等宽字体
    
    - 图片：**fit-to-width** 等比缩放、可上下滚动；双击在 `fit-to-width / 100%` 间切换
    
    - 右上角爱心：收藏切换（空心/实心）

### 2.3 键盘与交互

- 搜索框安装事件过滤：`↑/↓/PgUp/PgDn/Home/End` 转发给列表选择。

- `Enter`：若有选中→复制到剪贴板→隐藏浮窗→延时 1000ms 触发系统“粘贴”组合键；若无选中→选中第 1 项并执行。

- `Ctrl/⌘ + D`：切换当前项收藏状态。

- 点击窗口外/窗口失焦/按 `Esc`：关闭浮窗。

- 列表排序：统一按 `created_at DESC`。

### 2.4 剪贴板采集规则

- 支持：纯文本（UTF-8，默认最大 100KB）、图片（常见 `image/*`）。

- 忽略：HTML、RTF、富文本、文件列表、应用私有 MIME。

- 去重：仅当**与上一条记录内容完全相同**时跳过。

- 图片：保存原图至媒体目录；生成缩略图（最长边约 512px）；DB 存路径、尺寸、哈希等。

- 来源信息（可选）：应用名、PID（若跨平台可行）。

---

## 3. 搜索与过滤

- **关键字解析**：按空格拆分、去空、大小写不敏感；**AND** 关系。

- **模式切换**：
  
  - 未勾选“数据库”：在内存预加载集合中进行子串匹配（快速）。
  
  - 勾选“数据库”：使用 SQLite FTS5（`unicode61`）检索；若 FTS5 不可用则回退多条件 `LIKE '%kw%'`。

- **收藏过滤**：在当前模式下追加 `is_favorite=1` 过滤（与其他条件为交集）。

- **结果顺序**：`created_at DESC`。

- **预期规模**：内存预加载默认 1000（可配置 1~5000）；数据库可无限。

---

## 4. 数据模型与持久化

### 4.1 目录结构

- DB 文件：`QStandardPaths::AppDataLocation/clipboard.db`

- 媒体目录：`.../media/{uuid}.png|jpg|webp`

- 缩略图（可选）：`.../thumbs/{uuid}.jpg`

### 4.2 数据表（推荐 Schema）

```sql
CREATE TABLE IF NOT EXISTS items (
  id            INTEGER PRIMARY KEY AUTOINCREMENT,
  type          TEXT CHECK(type IN ('text','image')) NOT NULL,
  text          TEXT,
  media_path    TEXT,
  mime          TEXT,
  width         INTEGER,
  height        INTEGER,
  hash          TEXT,
  is_favorite   INTEGER DEFAULT 0,
  created_at    INTEGER NOT NULL,
  app_name      TEXT,
  app_pid       INTEGER
);
CREATE INDEX IF NOT EXISTS idx_items_created_at ON items(created_at DESC);
CREATE INDEX IF NOT EXISTS idx_items_fav ON items(is_favorite);

-- FTS（文本全文检索；若环境支持）
CREATE VIRTUAL TABLE IF NOT EXISTS items_fts
USING fts5(text, content='items', content_rowid='id', tokenize='unicode61');

CREATE TRIGGER IF NOT EXISTS items_ai AFTER INSERT ON items BEGIN
  INSERT INTO items_fts(rowid, text) VALUES (new.id, new.text);
END;
CREATE TRIGGER IF NOT EXISTS items_ad AFTER DELETE ON items BEGIN
  INSERT INTO items_fts(items_fts, rowid, text) VALUES('delete', old.id, old.text);
END;
CREATE TRIGGER IF NOT EXISTS items_au AFTER UPDATE ON items BEGIN
  INSERT INTO items_fts(items_fts, rowid, text) VALUES('delete', old.id, old.text);
  INSERT INTO items_fts(rowid, text) VALUES (new.id, new.text);
END;
```

> 迁移：使用 `PRAGMA user_version` 控制版本；历史清理或迁移后执行 `VACUUM`。  
> 文件一致性：清理时同步删除媒体文件与缩略图，避免孤立文件。

---

## 5. 架构与模块

```
app/
  core/
    ClipboardWatcher.*     // 监听剪贴板、过滤、去重、构建记录
    Database.*             // SQLite 封装（连接/迁移/查询/FTS/清理）
    InMemoryStore.*        // 预加载数据容器 + 过滤查找
    ImageStore.*           // 媒体落盘/缩略图/哈希/清理
    HotkeyManager.*        // qhotkey 封装，保存/更新全局快捷键
    AutoPaster.*           // 平台粘贴事件注入（Win/mac/X11/Wayland）
    Settings.*             // QSettings 封装
    SingleInstance.*       // QLocalServer 单实例与激活
  ui/
    MainPopup.*            // 浮窗容器与布局/显示隐藏/键盘路由
    HistoryListModel.*     // QAbstractListModel（图文混排数据）
    HistoryItemDelegate.*  // 列表绘制（缩略图、图标、文本省略）
    PreviewPane.*          // 预览（文本/图片）与爱心按钮
    TrayIcon.*             // 托盘图标与菜单
    SettingsDialog.*       // 设置对话框
  platform/
    AutoPaste_win.cpp      // SendInput(Ctrl+V)
    AutoPaste_mac.mm       // CGEventPost(⌘+V)
    AutoPaste_x11.cpp      // XTestFakeKeyEvent
    AutoPaste_wayland.cpp  // 调用 wtype/ydotool 或降级
main.cpp
resources.qrc             // 图标（light/dark）、样式资源
```

### 5.1 关键数据结构（接口层）

- `HistoryItem`
  
  - `id: qint64`
  
  - `type: enum{Text, Image}`
  
  - `text: QString`（Text）
  
  - `mediaPath: QString`（Image）
  
  - `mime: QString`
  
  - `width, height: int`（Image）
  
  - `hash: QString`（sha256）
  
  - `favorite: bool`
  
  - `createdAt: qint64 (epoch ms)`
  
  - `appName: QString`（可选）
  
  - `appPid: int`（可选）

### 5.2 主要信号/事件（行为契约）

- `ClipboardWatcher::itemCaptured(const HistoryItem&)`

- `InMemoryStore::itemsChanged()`

- `PreviewPane::favoriteToggled(qint64 id, bool on)`

- `MainPopup::requestSearch(QString query, bool useDb, bool onlyFav)`

- `MainPopup::commitRequested()`（回车提交）

- `SettingsDialog::hotkeyChanged(QKeySequence)`

- `TrayIcon::pauseToggled(bool paused)`

- 单实例通道：`ShowPopup` 消息

---

## 6. 设置与维护

### 6.1 设置项（QSettings 建议键）

- `hotkey/sequence`: `Ctrl+Shift+M`（跨平台保存）

- `paste/auto`: `true`（Wayland 默认 `false` 亦可，视实现）

- `preload/count`: `1000`（范围 1~5000）

- `capture/allowRepeat`: `false`（是否允许连续重复）

- `capture/paused`: `false`

- `theme/mode`: `system|light|dark`

- `db/path`: 自动生成，不建议用户修改

### 6.2 设置对话框功能

- 修改并测试全局快捷键（冲突反馈）。

- 自动粘贴开关（Wayland 限制说明）。

- 预加载条数调整。

- 存储占用：显示 DB 文件大小、媒体目录大小。

- 清理历史：输入“清理 X 天前”→执行删除与 `VACUUM`。

- 打开开源地址 `https://github.com/xxxx/xxxx`（系统浏览器）。

- 隐私：暂停监听切换。

---

## 7. 主题、图标与无障碍

- **主题**：基于 `QStyleHints::colorScheme()` 或平台 API 自动切换；提供强制 light/dark 选项。

- **图标**：准备 light/dark 两套 SVG（托盘、爱心空心/实心、收藏、数据库、搜索等）。macOS 托盘建议模板图标以便系统着色。

- **滚动条**：列表禁用水平滚动，文本超长省略，多行 tooltip。

- **可访问性**：控件提供可读名称（`accessibleName`）、快捷键与焦点顺序；爱心支持键盘触发与状态描述。

---

## 8. 性能与稳定性目标

- 预加载 1000 条含缩略图的列表：**首开 < 400ms**（NVMe 桌面）/ **< 1s**（HDD）。

- 搜索响应：内存模式 **< 50ms**（1000 条）；数据库 FTS **< 150ms**（10 万条量级）。

- 图片预览：> 60 FPS 滚动（常见 2K 图）；缩略图懒加载。

- 崩溃防护：I/O 与 DB 操作在线程池或后台任务中执行；UI 线程仅渲染。

---

## 9. 跨平台行为（自动粘贴）

- **Windows**：`SendInput` 注入 `Ctrl+V`；处理修饰键按下/抬起顺序。

- **macOS**：使用 `CGEventCreateKeyboardEvent` + `kCGEventFlagMaskCommand` 注入 `⌘+V`。

- **Linux / X11**：`XTestFakeKeyEvent` 注入 `Control_L + V`；需要链接 `libXtst`。

- **Linux / Wayland**：优先调用 `wtype` 或 `ydotool`（若存在）；否则降级为“仅复制不粘贴”。设置页需明确提示该限制并提供开关。

> 安全/权限：如平台对输入注入有限制（例如 macOS 的辅助功能权限），需在设置页提示用户授予。

---

## 10. 安全、隐私与合规

- **隐私开关**：托盘与设置页提供“暂停监听”。

- **数据保留**：用户可清理历史（按时间）；默认不做自动清理，可提供“仅保留最近 N 天”选项。

- **传输**：无网络同步；**不**上传任何内容。

- **日志**：默认仅记录错误与指标（不含剪贴板内容）；可在调试模式开启详细日志。

---

## 11. 依赖与构建

- **依赖**：
  
  - Qt ≥ 6.5（Widgets/Gui/Sql/Concurrent）
  
  - **QHotkey**（全局快捷键）
  
  - SQLite（Qt 驱动，自带；FTS5 建议启用）
  
  - Linux/X11：`libXtst`
  
  - Linux/Wayland（可选）：`wtype` 或 `ydotool`

- **构建**：CMake（推荐）；生成资源文件 `resources.qrc`（图标/样式）。

- **打包**：
  
  - Windows：windeployqt + 安装包（可选 NSIS）
  
  - macOS：macdeployqt + 签名/沙盒（如需）
  
  - Linux：AppImage/Flatpak（注意 X11/Wayland 依赖与权限声明）

---

## 12. 质量保证与测试矩阵

### 12.1 功能用例（抽样）

- 托盘存在且菜单项可用；首次启动注册默认全局快捷键。

- 浮窗：快捷键呼出；点击窗外/`Esc` 收起；拖拽移动。

- 监听：复制文本/图片后在列表出现；连续复制相同内容仅一条。

- 搜索：多关键词 AND；“数据库/收藏列表”复选逻辑正确；排序倒序。

- 收藏：点击爱心或 `Ctrl/⌘+D` 状态切换，并能过滤显示。

- 提交：`Enter` 复制→隐藏→ 1000ms 自动粘贴；（Wayland 缺依赖时不粘贴）。

- 设置：修改热键、开关自动粘贴、调整预加载、清理历史（DB/文件均减少）。

- 主题：跟随系统切换图标与配色。

- 单实例：二次启动激活浮窗。

### 12.2 平台/环境

| 平台                    | 桌面系统     | 剪贴板 | 自动粘贴             | 主题切换     |
| --------------------- | -------- | --- | ---------------- | -------- |
| Windows 10/11         | Explorer | ✔︎  | ✔︎（SendInput）    | ✔︎       |
| macOS 12–15           | Finder   | ✔︎  | ✔︎（CGEvent）      | ✔︎（模板图标） |
| Ubuntu 22.04（X11）     | GNOME    | ✔︎  | ✔︎（XTest）        | ✔︎       |
| Ubuntu 22.04（Wayland） | GNOME    | ✔︎  | △（wtype/ydotool） | ✔︎       |

---

## 13. 非功能性要求

- **启动时间**：冷启动至托盘就绪 < 1.5s；浮窗显示 < 120ms。

- **内存**：常驻 < 150MB（预加载 1000 条含缩略图）。

- **磁盘**：图片落盘可控；提供清理策略并显示占用。

- **本地化**：默认中文；使用 Qt 翻译机制预留英文。

---

## 14. 可交付物

- 源码树（含模块划分与资源）

- 构建脚本（CMakeLists.txt）

- 发行产物（Win/mac/Linux 各一）

- 使用指南（README，含 Wayland 限制说明）

- 变更日志与版本号（语义化）

---

## 15. 开源信息

- 设置页展示开源地址：`https://github.com/xxxx/xxxx`（可点击打开系统浏览器）。

- 许可证建议：MIT 或 Apache-2.0（与第三方依赖兼容）。

---

## 16. 已知风险与权衡

- **Wayland 输入注入**限制：无法保证自动粘贴；以外部工具兜底并提供显式开关与提示。

- **系统快捷键冲突**：默认使用 `Ctrl/⌘+Shift+M`；设置中允许修改与禁用。

- **数据库体积膨胀**：图片外置 + 定期清理 + `VACUUM`。

- **隐私**：提供暂停；默认不记录敏感格式/来源（用户自定）。

---

> 本文件为实现的“单一事实来源”。生成代码时请严格遵循以上行为与接口约定；如需扩展（如 OCR、云同步、多设备），应通过新增模块与可配置开关实现，且不改变默认行为。
