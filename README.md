# Jami-qt

![jami-logo](https://jami.net/assets/images/logo-jami.svg?v=8595727d35)

#### Share, freely and privately

# Introduction

Jami provides all its users a universal communication tool, autonomous, free, secure and built on a distributed architecture thus requiring no authority or central server to function.

`jami-qt` is the cross platform client for [Jami](https://jami.net/). For now, it's mainly used for the Windows platform and is not tested on other platforms.

For more information about the jami project, see the following:

+ Main website: https://jami.net/
+ Download: https://jami.net/download/
+ Bug tracker: https://git.jami.net/
+ Repositories: https://review.jami.net

# Getting involved

+ Browse our [current issues](https://git.jami.net/savoirfairelinux/jami-client-qt/issues), or file an issue.
+ IRC: #jami on freenode
+ ML: jami@gnu.org
+ Documentation: https://docs.jami.net
+ Localization happens on [Transifex](https://www.transifex.com/savoirfairelinux/jami/dashboard/)
+ [Our contributions propositions](https://git.jami.net/groups/savoirfairelinux/-/epics/1) or [feature requests](https://git.jami.net/savoirfairelinux/ring-project/wikis/technical/4.3.-Features-requests) asked by the community
+ Packaging: Feel free to contact us

## Notes

+ Coding style is managed by the clang-format, if you want to contribute, please use the pre-commit hook automatically installed with `./make-ring.py --init`
+ We use gerrit for our review. Please read https://git.jami.net/savoirfairelinux/ring-project/wikis/tutorials/Working-with-gerrit if you want to submit patches.

# Build instructions

Note: This project is quite new, and still need some work for the build integration.

## Dependencies

This client is only the graphical part, you will need to also build the daemon and LRC (the library containing the logic for desktop clients). Because of this, the recommended way is to clone our [meta-repository](https://review.jami.net/admin/repos/ring-project) containing all submodules needed.

In order to use the Qt Client it is necessary to have the Qt version 5.14 or higher. If your system does not have it you can install it [from sources or download the binary installer](https://www.qt.io/download).

## Build all projects

```bash
git clone https://review.jami.net/ring-project
```

Jami installer uses **python3**. If it's not installed, please install it:

```bash
cd ring-project/
./make-ring.py --init
```

Then you will need to install dependencies:

+ For GNU/Linux

```bash
./make-ring.py --dependencies --qt # needs sudo
```

Then, you can build daemon, lrc and client-qt with:

```bash
./make-ring.py --install --qt
```

And you will have the daemon in `daemon/bin/dring` and the client in `client-qt/build-local/jami-qt`. You also can run it with

If you use a Qt version that is not wide-system installed you need to specify its path after the `--qt` flag, i. e., `./make-ring.py --install --qt /home/<username>/Qt/5.15.0/gcc_64


```bash
./make-ring.py --run --qt
```

## Build only the client

```bash
cd client-qt
mkdir build
cd build
cmake .. -DQT5_VER=5.15.0 -DQT5_PATH=/home/<username>/Qt/5.15.0/gcc_64 -DLRC=<path_to_lrc> -DCMAKE_INSTALL_PREFIX=<installation_path>
make -j
```

Variables `QT5_VER` and `QT5_PATH` are used to specify version and path for a custom installation of Qt.

If lrc library is installed in a custom directory you can set its path with the variable LRC. Additionally you can specify built library location with `LRCLIB` (otherwise it will seach inside LRC with the suffixes `/lib`, `/build` and `/build-local`).

After the build has finished, you are finally ready to launch jami-qt in your build directory. 

If you want to install it to the path provided by `CMAKE_INSTALL_PREFIX` you can run:

```bash
make install
```


If you want more details, or separately build other projects you can check [this page](https://git.jami.net/savoirfairelinux/ring-project/wikis/technical/Build-instructions).

## Building On Native Windows

Only 64-bit MSVC build can be compiled.

> Note: command ```./make-ring.py --init``` is not required on the Windows build <br>

**Setup Before Building:**
- Download [Qt (Open Source)](https://www.qt.io/download-open-source?hsCtaTracking=9f6a2170-a938-42df-a8e2-a9f0b1d6cdce%7C6cb0de4f-9bb5-4778-ab02-bfb62735f3e5)<br>

  | | Prebuild | Module |
  |---|---|---|
  | Components: | msvc2017_64 | Qt WebEngine |

- Download [Visual Studio](https://visualstudio.microsoft.com/) (version >= 2015) <br>
- Install Qt Vs Tools under extensions, and configure msvc2017_64 path under Qt Options <br>

  | | Qt Version | SDK | Toolset |
  |---|---|---|---|
  | Minimum requirement: | 5.9.4 | 10.0.16299.0 | V141 |

- Install [Python3](https://www.python.org/downloads/) for Windows

**Start Building**
- Using Command Prompt
```sh
    git clone https://review.jami.net/ring-project
    cd ring-project/
    git submodule update --init daemon lrc client-windows
    git submodule update --recursive --remote daemon lrc client-windows
```
- Using **Elevated Command Prompt**
```sh
    python make-ring.py --dependencies
```

> Note:
> 1. This command will install **chocolatey** which may require you to restart the Command Prompt to be able to use it.
> 2. This command will install **msys2 (64 bit)** by using chocolatey command which may cause issues below: <br>
>    a. Choco may require you to restart the Command Prompt after finishing installing msys2. <br>
>    b. Only if you have already installed msys2 (64 bit) under the default installation folder, we will use the existing one.
> 3. This command will install **strawberry perl** by using chocolatey command which may fail if you have already installed it.
> 4. This command will install **cmake** by using chocolatey command which will not add cmake into PATH (environment variable). <br>
>
> The issue 1, 2(a), 3 can be solved by restarting the Command Prompt under Administrator right and re-run the command. <br>
> The issue 3 can be solved by uninstalling your current strawberry perl and re-run the command. <br>
> The issue 4 can be solved by adding the location of the cmake.exe into PATH. <br>

- Using a new **Non-Elevated Command Prompt**
```sh
    python make-ring.py --install
```
- Then you should be able to use the Visual Studio Solution file in client-windows folder **(Configuration = Release, Platform = x64)**

> Note: <br>
> To control the toolset and the sdk version that are used by msbuild, you can use ```--toolset``` and ```--sdk``` options <br>
> To control which Qt version should be used (qmake, windeployqt), uou can use ```--qtver``` option <br>
> By default: ```toolset=v141```, ```sdk=10.0.16299.0```,  ```qtver=5.9.4``` <br>
> For example:
```sh
    python make-ring.py --install --toolset v142 --sdk 10.0.18362.0 --qtver 5.12.0
```

### Build Module individually

- Jami-qt also support building each module (daemon, lrc, jami-qt) seperately

**Daemon**

- Make sure that dependencies is built by make-ring.py
- On MSVC folder (ring-project\daemon\MSVC):
```sh
    cmake -DCMAKE_CONFIGURATION_TYPES="ReleaseLib_win32" -DCMAKE_VS_PLATFORM_NAME="x64" -G "Visual Studio 16 2019" -A x64 -T '$(DefaultPlatformToolset)' ..
    python winmake.py -b daemon
```
- This will generate a ```.lib``` file in the path of ring-project\daemon\MSVC\x64\ReleaseLib_win32\bin

> Note: each dependencies contrib for daemon can also be updated individually <br>
> For example:
```bash
    python winmake.py -b opendht
```

**Lrc**

- Make sure that daemon is built first

```bash
    cd lrc
    python make-lrc.py -gb
```

**Jami-qt**

- Make sure that daemon, lrc are built first

```bash
    cd client-windows
    python make-client.py -d
    python make-client.py -b
    powershell -ExecutionPolicy Unrestricted -File copy-runtime-files.ps1
```

**Note**
- For all python scripts, both ```--toolset``` and ```--sdk``` options are available.
- For more available options, run scripts with ```-h``` option.
- ```--qtver``` option is available on ```make-lrc.py``` and ```make-client.py```.

## Packaging On Native Windows

- To be able to generate a msi package, first download and install [Wixtoolset](https://wixtoolset.org/releases/).
- In Visual Studio, download WiX Toolset Visual Studio Extension.
- Build client-windows project first, then the JamiInstaller project, msi package should be stored in ring-project\client-windows\JamiInstaller\bin\Release

#### Debugging

Compile the client with `BUILD=Debug` and compile LibRingClient with
`-DCMAKE_BUILD_TYPE=Debug`

# License

Copyright (C) 2020 Savoir-faire Linux Inc.

Jami is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

See COPYING or https://www.gnu.org/licenses/gpl-3.0.en.html for the full GPLv3 license.