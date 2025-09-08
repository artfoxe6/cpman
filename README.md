# cpman — Qt6 跨平台剪贴板历史工具（骨架）

本仓库实现了 AGENTS.md 规范下的工程骨架与最小可运行的应用结构，涵盖托盘、浮窗、数据库、剪贴板监听、内存预加载、平台自动粘贴（占位）等模块接口。

当前状态：
- 已完成：项目结构、CMake、托盘、浮窗布局、SQLite 封装（含 FTS 尝试）、剪贴板监听与去重、图片落盘与缩略图、内存预加载与筛选、单实例占位、自动粘贴平台桩、设置封装。
- 待完善：QHotkey 全局快捷键集成、列表委托绘制与预览细节、Wayland 自动粘贴集成外部工具、设置页各项功能与占用显示、主题与资源图标。

## 构建

依赖：
- Qt ≥ 6.5（Widgets/Gui/Sql/Concurrent）
- Linux/X11（可选自动粘贴）：libXtst 开发包
- Wayland（可选自动粘贴）：wtype 或 ydotool（任选其一）

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release (add -DHAVE_QHOTKEY=ON if your cache had it disabled)
cmake --build build -j
```

可执行文件位于 `build/app/`（依据平台与生成器有所差异）。

## 运行

- 程序常驻托盘，默认不显示窗口。
- 托盘菜单提供“打开/关闭”、“设置…”、“暂停/恢复监听”、“退出”。
- 全局快捷键（QHotkey）尚未接入，暂以托盘点击唤出浮窗。

## X11/Wayland 说明

- X11：若检测到 `libXtst`，使用 XTest 注入 Ctrl+V。
- Wayland：尝试调用 `wtype`，失败则回退 `ydotool`；二者均不可用时仅复制不粘贴。

提示：首次在 macOS 使用自动粘贴可能需要“辅助功能”权限。

## QHotkey（可选）

工程内置 HotkeyManager 占位实现。可通过集成 QHotkey 启用全局快捷键（建议）：
- 启用：在配置时加 `-DHAVE_QHOTKEY=ON`。
- CMake 会尝试 `find_package(QHotkey CONFIG)`，若失败再查找头文件/库创建导入目标。
- 请确保 QHotkey 的头文件与库可被发现（例如设置 `CMAKE_PREFIX_PATH` 指向安装前缀，或手动指定包含与库路径）。

## 开源地址

占位：`https://github.com/xxxx/xxxx`

## 许可证

待定（建议 MIT 或 Apache-2.0）。
