import sys
import os
import subprocess
import platform
import argparse
import multiprocessing
import shutil


# vs help
win_sdk_default = '10.0.16299.0'

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
    for root, _, files in os.walk(findVSLatestDir() + r'\\MSBuild'):
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
        '-f', '--force', action='store_true',
        help='Force action')
    ap.add_argument(
        '-p', '--purge', action='store_true',
        help='Purges the build directory')
    ap.add_argument(
        '-q', '--qtver', default='5.9.4',
        help='Sets the Qt version to build with')
    ap.add_argument(
        '-g', '--gen', action='store_true',
        help='Generates vs project files for lrc using CMake')
    ap.add_argument(
        '-a', '--arch', default='x64',
        help='Sets the build architecture')
    ap.add_argument(
        '-s', '--sdk', default=win_sdk_default, type=str,
        help='Use specified windows sdk version')

    parsed_args = ap.parse_args()

    return parsed_args

def build_project(msbuild, msbuild_args, proj, env_vars):
    args = []
    args.extend(msbuild_args)
    args.append(proj)
    cmd = [msbuild]
    cmd.extend(args)
    p = subprocess.Popen(cmd, shell=True,
                         stdout=sys.stdout,
                         env=env_vars)
    _, perr = p.communicate()
    if perr:
        print("Build failed when building ", proj)
        sys.exit(1)

def purge_build_dir():
    this_dir = os.path.dirname(os.path.realpath(__file__))
    build_dir = this_dir + r'\\msvc'
    if os.path.exists(build_dir):
        try:
            shutil.rmtree(build_dir)
        except Exception as e:
            print('Error while deleting directory: '+ str(e))
            sys.exit(1)

def main():
    if not host_is_64bit:
        print('These scripts will only run on a 64-bit Windows system for now!')
        sys.exit(1)

    if int(getLatestVSVersion()) < 15:
        print('These scripts require at least Visual Studio v15 2017!')
        sys.exit(1)

    parsed_args = parse_args()

    if parsed_args.purge:
        print('Purging build dir')
        purge_build_dir()

    elif parsed_args.build:
        print('Building with Qt: ' + parsed_args.qtver + ' ' + parsed_args.arch)
        vs_env_vars = {}
        vs_env_vars.update(getVSEnv())
        this_dir = os.path.dirname(os.path.realpath(__file__))
        qtwrapper_proj_path = this_dir + r'\\msvc\\src\\qtwrapper\\qtwrapper.vcxproj'
        lrc_proj_path = this_dir + r'\\msvc\\ringclient_static.vcxproj'
        msbuild = findMSBuild()
        if not os.path.isfile(msbuild):
            raise IOError('msbuild.exe not found. path=' + msbuild)
        msbuild_args = [
            '/nologo',
            '/verbosity:normal',
            '/maxcpucount:' + str(multiprocessing.cpu_count()),
            '/p:Platform=' + parsed_args.arch,
            '/p:Configuration=' + 'Release',
            '/p:useenv=true']
        build_project(msbuild, msbuild_args, qtwrapper_proj_path, vs_env_vars)
        build_project(msbuild, msbuild_args, lrc_proj_path, vs_env_vars)
        
    elif parsed_args.gen:
        print('Generating with Qt: ' + parsed_args.qtver + ' ' + parsed_args.arch)
        if parsed_args.force:
            purge_build_dir()
        this_dir = os.path.dirname(os.path.realpath(__file__))
        daemon_dir = os.path.dirname(this_dir) + r'\\daemon'
        daemon_bin = daemon_dir + r'\\MSVC\\x64\\ReleaseLib_win32\\bin\\dring.lib'
        if not os.path.exists(daemon_bin):
            print("Daemon library not found!")
            sys.exit(1)
        # we just assume Qt is installed in the default folder
        qt_dir = r'C:\\Qt\\' + parsed_args.qtver
        cmake_gen = getCMakeGenerator(getLatestVSVersion())
        qt_cmake_dir=  qt_dir + r'\\msvc2017_64\\lib\\cmake\\'
        cmake_options = [
            '-DQt5Core_DIR=' + qt_cmake_dir + 'Qt5Core',
            '-DQt5Sql_DIR=' + qt_cmake_dir + 'Qt5Sql',
            '-DQt5LinguistTools_DIR=' + qt_cmake_dir + 'Qt5LinguistTools',
            '-DQt5Concurrent_DIR=' + qt_cmake_dir + 'Qt5Concurrent',
            '-DQt5Gui_DIR=' + qt_cmake_dir + 'Qt5Gui',
            '-Dring_BIN=' + daemon_bin,
            '-DRING_INCLUDE_DIR=' + daemon_dir + r'\\src\\dring',
            '-DCMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION=' + parsed_args.sdk
        ]      
        build_dir = this_dir + r'\\msvc'
        if not os.path.exists(build_dir):
            os.makedirs(build_dir)
        os.chdir(build_dir)
        cmd = ['cmake', '..', '-G', cmake_gen]
        cmd.extend(cmake_options)
        p = subprocess.Popen(cmd, shell=True,
                             stdout=subprocess.PIPE)
        _, perr = p.communicate()
        if perr:
            print("Couldn't generate!")
            sys.exit(1)

    print('Done')

if __name__ == '__main__':
    main()