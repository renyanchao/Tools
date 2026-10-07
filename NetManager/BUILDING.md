# Building NetManager

NetManager uses CMake and vcpkg manifest mode for third-party C++ dependencies.
The project no longer links against `Tool/libprotobufd.lib`; protobuf is resolved by vcpkg for the current platform and compiler.

## Build Scripts

Run the script from the NetManager project root, or double-click it on Windows.

Windows:

```powershell
.\build_windows.bat
```

Linux:

```bash
./build_linux.sh
```

The scripts use paths relative to the script directory:

- Project root: the directory containing the script.
- vcpkg root: `..\..\vcpkg`.
- Build directory: `Build`.

If `..\..\vcpkg` does not exist, the script clones vcpkg there and bootstraps it.
On the first CMake configure, vcpkg reads `vcpkg.json` and installs protobuf for the selected triplet.
The scripts use CMake presets and retain build artifacts for incremental builds.
CMake generates protobuf C++ files into each preset's `generated/Proto` directory.

## Script Settings

All build parameters are stored in the scripts. Edit the script file to change them.

Windows defaults:

- Build config: `Debug`
- vcpkg triplet: `x64-windows`

Linux defaults:

- Build config: `Debug`
- vcpkg triplet: `x64-linux`

Both scripts print progress logs and wait for user input before exiting.

## VS Code 开发

打开仓库根目录的 `NetManager.code-workspace`。安装推荐的 C/C++、CMake Tools、Remote - SSH 和 EditorConfig 扩展。
需要 CMake 3.21 或更新版本。Windows 需要 VS2022 的“使用 C++ 的桌面开发”组件。

首次运行 `build_windows.bat`；Linux 使用 `sh build_linux.sh`，初始化 vcpkg 和依赖。
随后执行 `CMake: Select Configure Preset`，选择 `windows-debug` 或 `linux-debug`。
执行 `CMake: Configure` 后，按 `Ctrl+Shift+B` 增量构建。
执行 `CMake: Set Debug Target`，选择 `server` 或 `client`，按 F5 调试。

脚本和 VS Code 使用同一份 `CMakePresets.json`。
Windows 构建目录为 `Build/windows-debug`，Linux 为 `Build/linux-debug`。
需要清除配置缓存时执行 `CMake: Delete Cache and Reconfigure`。
脚本顶部的 `BUILD_PRESET` 决定预设；生成器、triplet 和配置统一在预设文件中定义。

## Linux Remote SSH

Ubuntu/Debian 远程主机首次准备工具：

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build gdb git openssh-server curl zip unzip tar pkg-config
```

确认 CMake 至少为 3.21。
Windows VS Code 执行 `Remote-SSH: Connect to Host...`，连接自己的 Linux 主机。
远程安装 C/C++、CMake Tools 和 EditorConfig 扩展，再打开远程仓库中的 `NetManager.code-workspace`。
Windows/Linux 使用各自的 Git checkout，通过 Git 同步源码；不复制 Build 目录。
远程窗口中的终端、编译和调试都在 Linux 执行。
当前 Linux epoll 和服务端入口仍需完善，开发环境配置不代表服务端已经支持 Linux 运行。

## 编码约定

C++ 源码沿用 UTF-8 BOM；`.sh` 使用 UTF-8 无 BOM 和 LF。
JSON/工作区文件使用 UTF-8 无 BOM。EditorConfig 扩展会按仓库规则保存文件。
