# Jami-qt

`jami-qt` is the cross platform client for Jami. For now, it's mainly used for the Windows platform and is not tested on other platforms.

![jami-logo](images/logo-jami-standard-coul.png)


For more information about the jami project, see the following:

- Main website: https://jami.net/
- Bug tracker: https://git.jami.net/
- Repositories: https://gerrit-ring.savoirfairelinux.com

## Building On Native Windows
---

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
---

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
    pandoc -f markdown -t html5 -o changelog.html changelog.md
    python make-client.py -d
    python make-client.py -b
    powershell -ExecutionPolicy Unrestricted -File copy-runtime-files.ps1
```

**Note**
- For all python scripts, both ```--toolset``` and ```--sdk``` options are available.
- For more available options, run scripts with ```-h``` option.
- ```--qtver``` option is available on ```make-lrc.py``` and ```make-client.py```.

## Packaging On Native Windows
---

- To be able to generate a msi package, first download and install [Wixtoolset](https://wixtoolset.org/releases/).
- In Visual Studio, download WiX Toolset Visual Studio Extension.
- Build client-windows project first, then the JamiInstaller project, msi package should be stored in ring-project\client-windows\JamiInstaller\bin\Release

## Linux
---

> For now, this process is experimental.

- LibRing and LibRingClient
must be installed first. If you have not already done so, go to the
[\#How to Build LibRing (or
Daemon)](#How_to_Build_LibRing_(or_Daemon) "wikilink") and [\#How to
Build LibRingClient (or
LRC)](#How_to_Build_LibRingClient_(or_LRC) "wikilink") sections.
- Building the whole ring-project is recommended, however, lrc might need to be rebuilt with cmake option ```-DCMAKE_INSTALL_PREFIX=/usr```

#### Other Requirements

-   Qt 5.9.4 (qt open source)
-   libqt5svg*, qtwebengine5-dev, qtmultimedia5-dev, qtdeclarative5-dev, pandoc

#### Getting the Source Code

```bash
    git clone https://review.jami.net/ring-client-windows
```

#### Build Instructions

**Windows Client dependencies**

- For Debian based:
```bash
    sudo apt install qtmultimedia5-dev libqt5svg5* qtwebengine5-dev qtdeclarative5-dev qtquickcontrols2-5-dev qml-module-qtquick* pandoc
```
- For Fedora:
```bash
    sudo dnf install qt5-qtsvg-devel qt5-qtwebengine-devel qt5-qtmultimedia-devel qt5-qtdeclarative-devel qt5-qtquickcontrols2-devel pandoc
```

**Build Windows Client**

```bash
    cd ring-client-windows
    pandoc -f markdown -t html5 -o changelog.html changelog.md
    mkdir build
    cd build
    qmake -qt=qt5 ../jami-qt.pro
    make -j9
```
- Then, you are finally ready to launch jami-qt in your build directory.

#### Debugging

Compile the client with `BUILD=Debug` and compile LibRingClient with
`-DCMAKE_BUILD_TYPE=Debug`

#### Known issues

1. The build system is not straight forward
2. Video doesn't work
3. Can't maximize/minimize window
4. Crash if the daemon is not started and installed.

## Mac OS
---
TBD
