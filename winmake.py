import sys
import os
import subprocess
import platform
import argparse

# vs help
vs_where_path = os.path.join(
    os.environ['ProgramFiles(x86)'], 'Microsoft Visual Studio', 'Installer', 'vswhere.exe'
)

host_is_64bit = (False, True)[platform.machine().endswith('64')]

def getLatestVSVersion():
    args = [
        '-latest',
        '-products *',
        '-requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
        '-property installationVersion'
    ]
    cmd = [vs_where_path] + args
    output = subprocess.check_output(' '.join(cmd)).decode('utf-8')
    if output:
        return output.splitlines()[0].split('.')[0]
    else:
        return

def findVSLatestDir():
    args = [
        '-latest',
        '-products *',
        '-requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
        '-property installationPath'
    ]
    cmd = [vs_where_path] + args
    output = subprocess.check_output(' '.join(cmd)).decode('utf-8')
    if output:
        return output.splitlines()[0]
    else:
        return


def findMSBuild():
    filename = 'MSBuild.exe'
    for root, _, files in os.walk(findVSLatestDir() + r'\MSBuild'):
        if filename in files:
            return os.path.join(root, filename)


def getVSEnv(arch='x64', platform='', version='10.0.16299.0'):
    env_cmd = getVSEnvCmd(arch, platform, version) + " && set"
    p = subprocess.Popen(env_cmd,
                         shell=True,
                         stdout=subprocess.PIPE)
    stdout, _ = p.communicate()
    out = stdout.decode('utf-8').split("\r\n")[5:-1]
    return dict(s.split('=') for s in out)


def getCMakeGenerator(vs_version):
    if vs_version == '15':
        return 'Visual Studio 15 2017 Win64'
    else:
        return 'Visual Studio ' + vs_version + ' 2019'

def getVSEnvCmd(arch='x64', platform='', version='10.0.16299.0'):
    vcEnvInit = [findVSLatestDir() + r'\VC\Auxiliary\Build\"vcvarsall.bat']
    if platform != '':
        args = [arch, platform, version]
    else:
        args = [arch, version]
    if args:
        vcEnvInit.extend(args)
    vcEnvInit = 'call \"' + ' '.join(vcEnvInit)
    return vcEnvInit

def parse_args():
    ap = argparse.ArgumentParser(description="Windows Jami-lrc build tool")
    ap.add_argument(
        '-b', '--build', action='store_true',
        help='Build lrc')
    ap.add_argument(
        '-c', '--clean', action='store_true',
        help='Cleans the build directory')
    ap.add_argument(
        '-qt', '--qtver', default='5.9.4',
        help='Sets the Qt version to build with')
    ap.add_argument(
        '-g', '--gen', action='store_true',
        help='Generates vs project files for lrc using CMake')
    ap.add_argument(
        '-a', '--arch', default='x64',
        help='Sets the build architecture')

    parsed_args = ap.parse_args()

    return parsed_args


def main():
    if not host_is_64bit:
        print('These scripts will only run on a 64-bit Windows system for now!')
        sys.exit(1)

    if int(getLatestVSVersion()) < 15:
        print('These scripts require at least Visual Studio v15 2017!')
        sys.exit(1)

    parsed_args = parse_args()

    if parsed_args.clean:
        print('clean')
    elif parsed_args.build:
        print('building with Qt: ' + parsed_args.qtver + ' ' + parsed_args.arch)
    elif parsed_args.gen:
        print('generating with Qt: ' + parsed_args.qtver + ' ' + parsed_args.arch)
        lrc_dir = os.getcwd()
        daemon_dir = os.path.dirname(lrc_dir) + r'\\daemon'
        daemon_bin = daemon_dir + r'\\MSVC\\x64\\ReleaseLib_win32\\bin\\dring.lib'
        # we just assume Qt is installed in the default folder
        qt_dir = r'C:\\Qt\\' + parsed_args.qtver
        cmake_gen = getCMakeGenerator(getLatestVSVersion())
        qt_cmake_dir=  qt_dir + r'\\msvc2017_64\\lib\\cmake'
        cmake_options = [
            '-DQt5Core_DIR=' + qt_cmake_dir + r'\\Qt5Core',
            '-DQt5Sql_DIR=' + qt_cmake_dir + r'\\Qt5Sql',
            '-DQt5LinguistTools_DIR=' + qt_cmake_dir + r'\\Qt5LinguistTools',
            '-DQt5Concurrent_DIR=' + qt_cmake_dir + r'\\Qt5Concurrent',
            '-DQt5Gui_DIR=' + qt_cmake_dir + r'\\Qt5Gui',
            '-Dring_BIN=' + daemon_bin,
            '-DRING_INCLUDE_DIR=' + daemon_dir + r'\\src\\dring'
        ]
        cmd = ['cmake', '-G', cmake_gen]
        cmd.extend(cmake_options)
        print(cmd)
        p = subprocess.Popen(cmd, shell=True,
                         stdout=subprocess.PIPE)
        p.communicate()
        
if __name__ == '__main__':
    main()