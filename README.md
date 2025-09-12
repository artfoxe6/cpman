# cpman — Qt6 跨平台剪贴板历史工具

一个专注隐私与高效工作的剪贴板历史工具，基于 Qt 6 跨平台实现，支持 macOS / Windows / Linux（X11/Wayland）。应用常驻系统托盘，通过全局快捷键呼出 Spotlight 风格的聚焦浮窗，快速搜索并预览文本与图片历史，支持自动粘贴与收藏。

项目实现与行为契约以根目录 AGENTS.md 为单一事实来源，README 提供使用与构建指南。

**核心特性**
- 托盘应用：启动后仅显示托盘图标，单实例运行。
- 聚焦浮窗：无边框置顶，点击窗外或 Esc 关闭，可拖拽移动。
- 剪贴板监听：仅记录 text/plain 与 image/*，消除连续重复。
- 预加载：默认预加载最近 1000 条到内存（可配置）。
- 搜索：空格分词 AND；可切换“数据库”模式（FTS5 优先）。
- 收藏：列表与预览窗支持切换收藏，支持“仅收藏”过滤。
- 提交：Enter 复制并隐藏浮窗，可选 1000ms 自动粘贴。
- 设置：全局快捷键、自动粘贴、预加载上限、清理历史等。
- 主题：跟随系统深浅色，图标自适应。

**默认快捷键与交互**
- 全局：Ctrl/⌘ + Shift + M 呼出/隐藏浮窗（可配置）。
- 列表导航：↑/↓/PgUp/PgDn/Home/End（搜索框聚焦也可用）。
- 提交：Enter（无选中时自动选择第 1 项后提交）。
- 收藏：Ctrl/⌘ + D 切换当前项收藏状态。
- 关闭：Esc 或点击窗口外。

**支持平台**
- Windows 10/11（Explorer，自动粘贴 SendInput）
- macOS 12–15（需辅助功能权限以启用自动粘贴）
- Ubuntu 22.04+（X11：XTest；Wayland：wtype/ydotool 或降级为仅复制）

---

**构建依赖**
- CMake ≥ 3.20
- Qt ≥ 6.5（Widgets/Gui/Sql/Concurrent/Network）
- 编译器：MSVC 2019+/Xcode 14+/GCC 11+/Clang 14+
- Linux（自动粘贴/X11）：`libxtst`（开发包）
- Linux（Wayland 自动粘贴，可选）：`wtype` 或 `ydotool`

**获取源码并构建**
- Release 构建
  - `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`
  - `cmake --build build -j`
- 产物位置
  - macOS：`build/app/cpman.app`
  - Windows：`build/app/cpman.exe`
  - Linux：`build/app/cpman`

**全局快捷键（QHotkey）**
- 已集成 QHotkey（CMake 自动查找；若未安装，将自动使用 FetchContent 获取）。
- 如需禁用：配置时添加 `-DHAVE_QHOTKEY=OFF`。

---

**运行与使用**
- 首次运行后常驻系统托盘；通过托盘菜单可以开启/关闭浮窗、打开设置、暂停/恢复监听、退出。
- 浮窗顶部：搜索框 + “数据库/收藏列表”勾选；底部：左侧历史列表（图文混排，垂直滚动），右侧预览区（文本换行/图片等比 fit-to-width、双击 100%/铺满切换，右上角爱心收藏）。
- 搜索：空格分词 AND；未勾选“数据库”时在内存集合中子串匹配；勾选时使用 SQLite FTS5（回退多条件 LIKE）。
- 提交：Enter 复制到系统剪贴板 → 隐藏浮窗 → 1000ms 后尝试自动粘贴（Wayland 可能不生效）。

---

**数据存储与路径**
- 数据库：`QStandardPaths::AppDataLocation/clipboard.db`
- 媒体目录：`.../media/{uuid}.png|jpg|webp`（原图落盘）
- 缩略图：`.../thumbs/{uuid}.jpg`（可选）
- Schema：见 `AGENTS.md`（含 `items` 表与 `items_fts` 及触发器）

**设置项（QSettings）**
- `hotkey/sequence`：默认 `Ctrl+Shift+M`
- `paste/auto`：默认 `true`（Wayland 推荐 `false`）
- `preload/count`：默认 `1000`（1~5000）
- `capture/allowRepeat`：默认 `false`
- `capture/paused`：默认 `false`
- `theme/mode`：`system|light|dark`

---

**打包与分发（CPack）**
- 已集成跨平台打包：
  - macOS：DMG（DragNDrop）。需要 `macdeployqt` 将 Qt 依赖写入 `.app`。
  - Windows：NSIS 安装包。需要 `windeployqt` 收集 Qt 依赖。
  - Linux：默认 `DEB` 与 `TGZ`；若安装了 `linuxdeploy` 与 `appimagetool`，额外生成 AppImage。
- 命令：
  - `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`
  - `cmake --build build -j`
  - 可选部署：
    - macOS：`macdeployqt build/app/cpman.app`
    - Windows：`windeployqt build\app\cpman.exe`
  - 生成安装包：`cpack --config build/CPackConfig.cmake`
- 产物：
  - `cpman-<ver>-Darwin.dmg` / `cpman-<ver>-Windows.exe` / `cpman-<ver>-Linux.deb|.tar.gz` / `cpman-<ver>.AppImage`

---

**平台注意事项**
- macOS 自动粘贴需要“辅助功能”权限：系统设置 → 隐私与安全性 → 辅助功能 → 勾选 cpman。
- Wayland 无法稳定注入组合键：若未安装 `wtype/ydotool`，将仅复制不自动粘贴；设置页可关闭“自动粘贴”。
- X11 自动粘贴依赖 `libXtst`（发行包请声明/打包此依赖）。

---

**开发结构**
- `app/core/*`：剪贴板监听、数据库、内存索引、图片存储、热键、自动粘贴、设置、单实例等。
- `app/ui/*`：浮窗、列表模型与委托、预览、托盘、设置对话框、主题。
- `app/platform/*`：平台自动粘贴与应用信息获取。
- `app/resources.qrc`：深/浅色图标资源。

---

**常见问题**
- 启动未见窗口：应用默认驻留托盘，请通过全局快捷键或托盘菜单唤出浮窗。
- 全局快捷键无效：确认系统未占用该组合；在设置中更换；Linux 桌面环境可能需要额外权限/关闭冲突快捷键。
- 自动粘贴无效：macOS 确认“辅助功能”权限；Wayland 请安装 `wtype` 或 `ydotool` 并确保在 PATH 中。

---

**贡献与致谢**
- 贡献：欢迎提交 Issue/PR（模块边界与行为参见 AGENTS.md）。
- 第三方依赖：
  - Qt 6（LGPL/GPL 等多种授权）
  - QHotkey（BSD-3-Clause）
  - Linux/X11：`libXtst`（XTest）

---

**许可证**
- 本项目使用 GPL-3.0 许可证，详见根目录 `LICENSE`。