import tempfile
import re
import sys
import os
import subprocess
import platform
import argparse
import multiprocessing
import shutil
import fileinput
import re

# vs help
win_sdk_default = '10.0.16299.0'
win_toolset_default = 'v142'

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


def getVSEnv(arch='x64', platform='', version=''):
    env_cmd = 'set path=%path:"=% && ' + \
        getVSEnvCmd(arch, platform, version) + ' && set'
    p = subprocess.Popen(env_cmd,
                         shell=True,
                         stdout=subprocess.PIPE)
    stdout, _ = p.communicate()
    out = stdout.decode('utf-8').split("\r\n")[5:-1]
    return dict(s.split('=', 1) for s in out)


def getCMakeGenerator(vs_version):
    if vs_version == '15':
        return 'Visual Studio 15 2017 Win64'
    else:
        return 'Visual Studio ' + vs_version + ' 2019'


def getVSEnvCmd(arch='x64', platform='', version=''):
    vcEnvInit = [findVSLatestDir() + r'\VC\Auxiliary\Build\"vcvarsall.bat']
    if platform != '':
        args = [arch, platform, version]
    else:
        args = [arch, version]
    if args:
        vcEnvInit.extend(args)
    vcEnvInit = 'call \"' + ' '.join(vcEnvInit)
    return vcEnvInit


def build_project(msbuild, msbuild_args, proj, env_vars):
    args = []
    args.extend(msbuild_args)
    args.append(proj)
    cmd = [msbuild]
    cmd.extend(args)
    p = subprocess.Popen(cmd, shell=True,
                         stdout=sys.stdout,
                         env=env_vars)
    _, _ = p.communicate()
    if p.returncode:
        print("Build failed when building ", proj)
        sys.exit(1)


def purge_dir(build_dir):
    if os.path.exists(build_dir):
        try:
            shutil.rmtree(build_dir)
        except Exception as e:
            print('Error while removing directory: ' + str(e))
            sys.exit(1)


def generate(force, qtver, sdk, toolset, arch):
    # check build dir
    this_dir = os.path.dirname(os.path.realpath(__file__))
    build_dir = this_dir + '\\msvc'
    if os.path.exists(build_dir):
        if not force:
            print("Skipping generate, build directory already exists!")
            return
        purge_dir(build_dir)
    print('Generating lrc with Qt-' + qtver + ' ' +
          arch + ' ' + sdk + ' ' + toolset)
    this_dir = os.path.dirname(os.path.realpath(__file__))
    daemon_dir = os.path.dirname(this_dir) + '\\daemon'
    daemon_bin = daemon_dir + '\\build-local\\x64\\ReleaseLib_win32\\bin\\dring.lib'
    if not os.path.exists(daemon_bin):
        print("Daemon library not found!")
        sys.exit(1)
    # we just assume Qt is installed in the default folder
    qt_dir = 'C:\\Qt\\' + qtver
    cmake_gen = getCMakeGenerator(getLatestVSVersion())

    qt_cmake_dir = qt_dir + '\\msvc2019_64\\lib\\cmake\\'
    cmake_options = [
        '-DQt5_DIR=' + qt_cmake_dir + 'Qt5',
        '-DQt5Core_DIR=' + qt_cmake_dir + 'Qt5Core',
        '-DQt5Sql_DIR=' + qt_cmake_dir + 'Qt5Sql',
        '-DQt5LinguistTools_DIR=' + qt_cmake_dir + 'Qt5LinguistTools',
        '-DQt5Concurrent_DIR=' + qt_cmake_dir + 'Qt5Concurrent',
        '-DQt5Gui_DIR=' + qt_cmake_dir + 'Qt5Gui',
        '-Dring_BIN=' + daemon_bin,
        '-DRING_INCLUDE_DIR=' + daemon_dir + '\\src\\dring',
        '-DCMAKE_SYSTEM_VERSION=' + sdk
    ]
    build_dir = this_dir + '\\msvc'
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)
    os.chdir(build_dir)
    cmd = ['cmake', '..', '-G', cmake_gen]
    cmd.extend(cmake_options)
    p = subprocess.Popen(cmd, shell=True,
                         stdout=subprocess.PIPE)
    _, _ = p.communicate()
    if p.returncode:
        print("Couldn't generate!")
        sys.exit(1)

    qtwrapper_proj_path = this_dir + '\\msvc\\src\\qtwrapper\\qtwrapper.vcxproj'
    lrc_proj_path = this_dir + '\\msvc\\ringclient_static.vcxproj'

    # force toolset
    replace_vs_prop(qtwrapper_proj_path,
                    'PlatformToolset',
                    toolset)
    replace_vs_prop(lrc_proj_path,
                    'PlatformToolset',
                    toolset)
    # force unicode
    replace_vs_prop(qtwrapper_proj_path,
                    'CharacterSet',
                    'Unicode')
    replace_vs_prop(lrc_proj_path,
                    'CharacterSet',
                    'Unicode')
    os.chdir(this_dir)


def replace_vs_prop(filename, prop, val):
    p = re.compile(r'(?s)<' + prop + r'\s?.*?>(.*?)<\/' + prop + r'>')
    val = r'<' + prop + r'>' + val + r'</' + prop + r'>'
    with fileinput.FileInput(filename, inplace=True) as file:
        for line in file:
            print(re.sub(p, val, line), end='')


def build(arch, toolset):
    print('Building lrc Release|' + arch)
    vs_env_vars = {}
    vs_env_vars.update(getVSEnv())
    this_dir = os.path.dirname(os.path.realpath(__file__))
    build_dir = this_dir + '\\msvc'
    qtwrapper_proj_path = build_dir + '\\src\\qtwrapper\\qtwrapper.vcxproj'
    lrc_proj_path = build_dir + '\\ringclient_static.vcxproj'
    msbuild = findMSBuild()
    if not os.path.isfile(msbuild):
        raise IOError('msbuild.exe not found. path=' + msbuild)
    msbuild_args = [
        '/nologo',
        '/verbosity:minimal',
        '/maxcpucount:' + str(multiprocessing.cpu_count()),
        '/p:Platform=' + arch,
        '/p:Configuration=' + 'Release',
        '/p:useenv=true',
        '/p:PlatformToolset=' + toolset]
    build_project(msbuild, msbuild_args, qtwrapper_proj_path, vs_env_vars)
    build_project(msbuild, msbuild_args, lrc_proj_path, vs_env_vars)


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
        '-q', '--qtver', default='5.14.2',
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
    ap.add_argument(
        '-t', '--toolset', default=win_toolset_default, type=str,
        help='Use specified platform toolset version')

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

    if parsed_args.purge:
        print('Purging build dir')
        this_dir = os.path.dirname(os.path.realpath(__file__))
        build_dir = this_dir + '\\msvc'
        purge_dir(build_dir)

    if parsed_args.gen:
        generate(parsed_args.force, parsed_args.qtver,
                 parsed_args.sdk, parsed_args.toolset, parsed_args.arch)

    if parsed_args.build:
        build(parsed_args.arch, parsed_args.toolset)


if __name__ == '__main__':
    main()
