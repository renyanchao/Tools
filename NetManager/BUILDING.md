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
The scripts remove the `Build` directory before configuring.
CMake generates protobuf C++ files from `Proto/*.proto` into `Build/generated/Proto`.

## Script Settings

All build parameters are stored in the scripts. Edit the script file to change them.

Windows defaults:

- Build config: `Debug`
- vcpkg triplet: `x64-windows`

Linux defaults:

- Build config: `Debug`
- vcpkg triplet: `x64-linux`

Both scripts print progress logs and wait for user input before exiting.
