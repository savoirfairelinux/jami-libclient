
# Build instructions

There is basically two ways to build `client-qt`:
+ Use `build.py` script which will build all Jami (daemon/client lib/client-qt)
+ Build only this client.

## GNU/Linux

### With build.py

```bash
git clone https://review.jami.net/ring-project
```

Jami installer uses **python3**. If it's not installed, please install it:

```bash
cd ring-project/
./build.py --init
```

Then you will need to install dependencies:

+ For GNU/Linux

```bash
./build.py --dependencies --qt # needs sudo
```

Then, you can build daemon, lrc and client-qt with:

```bash
./build.py --install --qt
```

And you will have the daemon in `daemon/bin/jamid` and the client in `client-qt/build-local/jami-qt`. You also can run it with

If you use a Qt version that is not wide-system installed you need to specify its path after the `--qt` flag, i. e., `./build.py --install --qt /home/<username>/Qt/5.15.0/gcc_64


```bash
./build.py --run --qt
```

Notes:
+ `--global-install` to install client-qt globally under /usr/local
+ `--prefix` to change the destination of the install.

## Build only the client

In order to use the Qt Client it is necessary to have the Qt version 5.14 or higher. If your system does not have it you can install it [from sources or download the binary installer](https://www.qt.io/download).

## Dependencies, Debian based

```
sudo apt-get install cmake make doxygen g++ gettext libnotify-dev pandoc nasm libqrencode-dev \
                     libnotify-dev libnm-dev \
                     qtbase5-dev libqt5sql5-sqlite \ # Qt > 5.14!!!!
                     qtmultimedia5-dev libqt5svg5-dev qtwebengine5-dev qtdeclarative5-dev \
                     qtquickcontrols2-5-dev qml-module-qtquick2 qml-module-qtquick-controls \
                     qml-module-qtquick-controls2 qml-module-qtquick-dialogs \
                     qml-module-qtquick-layouts qml-module-qtquick-privatewidgets \
                     qml-module-qtquick-shapes qml-module-qtquick-window2 \
                     qml-module-qtquick-templates2 qml-module-qt-labs-platform \
                     qml-module-qtwebengine qml-module-qtwebchannel \
                     qml-module-qt-labs-qmlmodels
```

## Dependencies, Fedora

```
sudo dnf install qt5-qtsvg-devel qt5-qtwebengine-devel qt5-qtmultimedia-devel qt5-qtdeclarative-devel qt5-qtquickcontrols2-devel qt5-qtquickcontrols qrencode-devel NetworkManager-libnm-devel
```

## Build only this repository

```bash
# In this repository
mkdir build
cd build
cmake ..
make -j
```

cmake can take several options:
+ Variables `QT5_VER` and `QT5_PATH` are used to specify version and path for a custom installation of Qt.
+ If lrc library is installed in a custom directory you can set its path with the variable LRC. Additionally you can specify built library location with `LRC` (otherwise it will seach inside LRC with the suffixes `/lib`, `/build` and `/build-local`).

e.g.

```
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/home/amarok/Projects/jami/install/lrc -DLRC=/home/user/install-lrc
```

After the build has finished, you are finally ready to launch jami-qt in your build directory.

If you want to install it to the path provided by `CMAKE_INSTALL_PREFIX` you can run:

```bash
make install
```

## Building On Native Windows

Only 64-bit MSVC build can be compiled.

> Note: command ```./make-ring.py --init``` is not required on the Windows build <br>

**Setup Before Building:**
- Download [Qt (Open Source)](https://www.qt.io/download-open-source?hsCtaTracking=9f6a2170-a938-42df-a8e2-a9f0b1d6cdce%7C6cb0de4f-9bb5-4778-ab02-bfb62735f3e5)<br>

  | | Prebuild | Module |
  |---|---|---|
  | Components: | msvc2019_64 | Qt WebEngine |

- Download [Visual Studio](https://visualstudio.microsoft.com/) (version >= 2019) <br>
- Install Qt Vs Tools under extensions, and configure msvc2017_64 path under Qt Options <br>

  | | Qt Version | SDK | Toolset |
  |---|---|---|---|
  | Minimum requirement: | 5.14.0 | 10.0.16299.0 | V142 |

- Install [Python3](https://www.python.org/downloads/) for Windows

**Start Building**
- Using Command Prompt
```sh
    git clone https://review.jami.net/ring-project
    cd ring-project/
    git submodule update --init daemon lrc client-qt
    git submodule update --recursive --remote daemon lrc client-qt
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
> By default: ```toolset=v142```, ```sdk=10.0.16299.0```,  ```qtver=5.15.0``` <br>
> For example:
```sh
    python make-ring.py --install --toolset v142 --sdk 10.0.16299.0 --qtver 5.15.0
```

### Build Module Individually

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
```

**Note**
- For all python scripts, both ```--toolset``` and ```--sdk``` options are available.
- For more available options, run scripts with ```-h``` option.
- ```--qtver``` option is available on ```make-lrc.py``` and ```make-client.py```.

## Packaging On Native Windows

- To be able to generate a msi package, first download and install [Wixtoolset](https://wixtoolset.org/releases/).
- In Visual Studio, download WiX Toolset Visual Studio Extension.
- Build client-windows project first, then the JamiInstaller project, msi package should be stored in ring-project\client-windows\JamiInstaller\bin\Release

## Testing for Client-qt on Windows

- We currently use [GoogleTest](https://github.com/google/googletest) and [Qt Quick Test](https://doc.qt.io/qt-5/qtquicktest-index.html#introduction) in our product. To build and run tests, you could use the following command.

```
    python make-client.py -b -wt
    python make-client.py runtests
```

- Note that, for tests, the path of local storage files for jami will be changed based on following environment variables.

```
    %JAMI_DATA_HOME% = %TEMP% + '\\jami_test\\jami'
    %JAMI_CONFIG_HOME% = %TEMP% + '\\jami_test\\.config'
    %JAMI_CACHE_HOME% = %TEMP% + '\\jami_test\\.cache'
```

- These environment variables will be temporarily set when using make-client.py to run tests.

## Debugging

Compile the client with `BUILD=Debug` and compile LibRingClient with
`-DCMAKE_BUILD_TYPE=Debug`