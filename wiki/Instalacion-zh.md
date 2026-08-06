# 安装

md-editor 可以在 Linux、Windows 和 macOS 上运行。

## 下载（二进制文件）

| 平台 | 格式 | 说明 |
|---|---|---|
| Linux x86_64 | **AppImage** | 单个可执行文件；赋予可执行权限后直接打开 |
| Windows x64 | **便携版 ZIP** | 解压即用，无需安装程序 |
| macOS（Apple Silicon + Intel） | 通用 **DMG** | 未签名：首次启动请用 **Ctrl+单击 → 打开** |

## 从源代码编译

### 依赖

- **CMake** ≥ 3.16
- **Qt 6** ≥ 6.5，并带上它的**私有头文件**（*Widgets*、*PrintSupport*、
  *LinguistTools* 模块）。导出为 ODF/DOCX/EPUB 需要这些私有头文件（`Qt6::GuiPrivate`，
  即 Qt 自带的 QZip）。在 Debian/Ubuntu 上，它们在 `qt6-base-private-dev` 里，与
  `qt6-base-dev` 是两个包。
- **C++17** 编译器（GCC 9+、Clang 10+ 或 MSVC 19.20+）
- **Hunspell** *（可选）*，用于拼写检查（`libhunspell-dev`）；没有它，其余部分照样
  可以编译。

```bash
# Debian/Ubuntu
sudo apt-get install qt6-base-dev qt6-base-private-dev cmake g++
sudo apt-get install libhunspell-dev          # 可选（拼写检查）
```

要渲染**图表**（可选，运行时才需要），需要装有 `mmdc`（Mermaid）和／或 `plantuml`
（PlantUML）。

### 编译

```bash
cmake -S . -B build && cmake --build build
./build/md-editor [文件.md]
```

在 Windows 上可以使用 *Visual Studio 17 2022* 生成器。

### 在 Linux 上安装

`install.sh` 脚本会编译一个体积最小的版本，并安装可执行文件、`.desktop` 启动项和桌面
图标：

```bash
sudo ./install.sh                    # 安装到 /usr/local
PREFIX="$HOME/.local" ./install.sh   # 用户级安装，无需 sudo
```

### 测试

```bash
ctest --test-dir build --output-on-failure
```
