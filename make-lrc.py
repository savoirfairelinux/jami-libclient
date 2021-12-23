#!/usr/bin/env python3

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
from enum import Enum

qt_version_default = '6.2.1'

vs_where_path = os.path.join(
    os.environ['ProgramFiles(x86)'], 'Microsoft Visual Studio', 'Installer', 'vswhere.exe'
)

host_is_64bit = (False, True)[platform.machine().endswith('64')]
this_dir = os.path.dirname(os.path.realpath(__file__))
build_dir = os.path.join(this_dir, 'build')

qt_path = os.path.join('c:', os.sep, 'Qt')
qt_kit_path = 'msvc2019_64'
qt_root_path = os.getenv('QT_ROOT_DIRECTORY', qt_path)

def execute_cmd(cmd, with_shell=False, env_vars=None, cmd_dir=os.getcwd()):
    p = subprocess.Popen(cmd,
                         shell=with_shell,
                         stdout=sys.stdout,
                         env=env_vars,
                         cwd=cmd_dir)
    _, _ = p.communicate()
    return p.returncode


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


def getVSEnv(arch='x64', platform='', version=''):
    env_cmd = 'set path=%path:"=% && ' + \
        getVSEnvCmd(arch, platform, version) + ' && set'
    p = subprocess.Popen(env_cmd,
                         shell=True,
                         stdout=subprocess.PIPE)
    stdout, _ = p.communicate()
    out = stdout.decode('utf-8', 'ignore').split("\r\n")[5:-1]
    return dict(s.split('=', 1) for s in out)


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


def build(qtver):
    print("Building with Qt " + qtver)

    config_str = 'Release'

    vs_env_vars = {}
    vs_env_vars.update(getVSEnv())

    qt_dir = os.path.join(qt_root_path, qtver, qt_kit_path)
    daemon_dir = os.path.dirname(this_dir) + '\\daemon'
    daemon_bin = daemon_dir + '\\build\\x64\\ReleaseLib_win32\\bin\\jami.lib'

    cmake_options = [
        '-DCMAKE_PREFIX_PATH=' + qt_dir,
        '-DCMAKE_BUILD_TYPE=' + config_str,
        '-Dring_BIN=' + daemon_bin,
        '-DRING_INCLUDE_DIR=' + daemon_dir + '\\src\\jami'
    ] 

    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    cmd = ['cmake', '..']

    print('Configuring...')
    cmd.extend(cmake_options)
    if(execute_cmd(cmd, False, vs_env_vars, build_dir)):
        print("Cmake generate error")
        sys.exit(1)

    print('Building...')
    cmd = [
        'cmake', '--build', '.',
        '--config', config_str,
        '--', '-m'
    ]
    if(execute_cmd(cmd, False, vs_env_vars, build_dir)):
        print("Cmake build error")
        sys.exit(1)


def parse_args():
    ap = argparse.ArgumentParser(description="Windows Jami-lrc build tool")
    ap.add_argument(
        '-q', '--qtver', default=qt_version_default,
        help='Sets the Qt version to build with')

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
    build(parsed_args.qtver)


if __name__ == '__main__':
    main()
