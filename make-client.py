#!/usr/bin/env python3

import tempfile
import re
import sys
import os
import subprocess
import platform
import argparse
import multiprocessing
import fileinput
from enum import Enum

qt_version_default = '6.2.0'

vs_where_path = os.path.join(
    os.environ['ProgramFiles(x86)'], 'Microsoft Visual Studio', 'Installer', 'vswhere.exe'
)

host_is_64bit = (False, True)[platform.machine().endswith('64')]
this_dir = os.path.dirname(os.path.realpath(__file__))
build_dir = os.path.join(this_dir, 'build')

temp_path = os.environ['TEMP']
openssl_include_dir = 'C:\\Qt\\Tools\\OpenSSL\\Win_x64\\include\\openssl'

qt_path = os.path.join('c:', os.sep, 'Qt')
qt_kit_path = 'msvc2019_64'
qt_root_path = os.getenv('QT_ROOT_DIRECTORY', qt_path)

# project path
unit_test_project = os.path.join(build_dir, 'tests', 'unittests.vcxproj')
qml_test_project = os.path.join(build_dir, 'tests', 'qml_tests.vcxproj')

# test executable command
qml_test_exe = os.path.join(this_dir, 'x64', 'test', 'qml_tests.exe -input ') + \
    os.path.join(this_dir, 'tests', 'qml')
unit_test_exe = os.path.join(this_dir, 'x64', 'test', 'unittests.exe')


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
    output = subprocess.check_output(
        ' '.join(cmd)).decode('utf-8', errors='ignore')
    if output:
        return output.splitlines()[0]
    else:
        return


def getQtVersionNumber(qt_version, version_type):
    version_list = qt_version.split('.')
    return version_list[version_type.value]


def findMSBuild():
    filename = 'MSBuild.exe'
    for root, _, files in os.walk(findVSLatestDir() + r'\\MSBuild'):
        if filename in files:
            return os.path.join(root, filename)


def getMSBuildArgs(arch, config_str, configuration_type=''):
    msbuild_args = [
        '/nologo',
        '/verbosity:minimal',
        '/maxcpucount:' + str(multiprocessing.cpu_count()),
        '/p:Platform=' + arch,
        '/p:Configuration=' + config_str,
        '/p:useenv=true']
    if (configuration_type != ''):
        msbuild_args.append('/p:ConfigurationType=' + configuration_type)
    return msbuild_args


def getVSEnv(arch='x64', platform='', version=''):
    env_cmd = 'set path=%path:"=% && ' + \
        getVSEnvCmd(arch, platform, version) + ' && set'
    p = subprocess.Popen(env_cmd,
                         shell=True,
                         stdout=subprocess.PIPE)
    stdout, _ = p.communicate()
    out = stdout.decode('utf-8', errors='ignore').split("\r\n")[5:-1]
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


def replace_necessary_vs_prop(project_path, toolset, sdk_version):
    # force toolset
    replace_vs_prop(project_path,
                    'PlatformToolset',
                    toolset)
    # force unicode
    replace_vs_prop(project_path,
                    'CharacterSet',
                    'Unicode')
    # force sdk_version
    replace_vs_prop(project_path,
                    'WindowsTargetPlatformVersion',
                    sdk_version)


def build_project(msbuild, msbuild_args, proj, env_vars):
    args = []
    args.extend(msbuild_args)
    args.append(proj)
    cmd = [msbuild]
    cmd.extend(args)
    if (execute_cmd(cmd, True, env_vars)):
        print("Build failed when building ", proj)
        sys.exit(1)


def replace_vs_prop(filename, prop, val):
    p = re.compile(r'(?s)<' + prop + r'\s?.*?>(.*?)<\/' + prop + r'>')
    val = r'<' + prop + r'>' + val + r'</' + prop + r'>'
    with fileinput.FileInput(filename, inplace=True) as file:
        for line in file:
            print(re.sub(p, val, line), end='')


def init_submodules():
    print('Initializing submodules...')

    if (execute_cmd(['git', 'submodule', 'update', '--init'], False)):
        print('Submodule initialization error.')
    else:
        if (execute_cmd(['git', 'submodule', 'update', '--recursive'], False)):
            print('Submodule recursive checkout error.')
        else:
            print('Submodule recursive checkout finished.')


def build_deps(arch):
    print('Patching and building qrencode')
    apply_cmd = [
        'git',
        'apply',
        '--reject',
        '--ignore-whitespace',
        '--whitespace=fix'
    ]
    qrencode_dir = os.path.join(this_dir, '3rdparty', 'qrencode-win32')
    patch_file = os.path.join(this_dir, 'qrencode-win32.patch')
    apply_cmd.append(patch_file)
    print(apply_cmd)
    if(execute_cmd(apply_cmd, False, None, qrencode_dir)):
        print("Couldn't patch qrencode-win32.")

    vs_env_vars = {}
    vs_env_vars.update(getVSEnv())

    msbuild = findMSBuild()
    if not os.path.isfile(msbuild):
        raise IOError('msbuild.exe not found. path=' + msbuild)
    msbuild_args = getMSBuildArgs(arch, 'Release-Lib')
    proj_path = os.path.join(qrencode_dir, 'qrencode-win32',
                             'vc8', 'qrcodelib', 'qrcodelib.vcxproj')

    build_project(msbuild, msbuild_args, proj_path, vs_env_vars)


def build(arch, config_str, qtver, tests=False):
    print("Building with Qt " + qtver)

    vs_env_vars = {}
    vs_env_vars.update(getVSEnv())

    qt_dir = os.path.join(qt_root_path, qtver, qt_kit_path)

    cmake_options = [
        '-DCMAKE_PREFIX_PATH=' + qt_dir,
        '-DCMAKE_BUILD_TYPE=' + config_str
    ]
    if tests:
        cmake_options.append('-DENABLE_TESTS=true')

    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    config_str = 'Release'
    cmd = ['cmake', '..']
    if (config_str == 'Beta'):
        cmake_options.append('-DBETA=1')

    print('Generating project using cmake ' + config_str + '|' + arch)
    cmd.extend(cmake_options)
    if(execute_cmd(cmd, False, vs_env_vars, build_dir)):
        print("Cmake generate error")
        sys.exit(1)

    print('Building projects in ' + config_str + '|' + arch)
    cmd = ['cmake', '--build', '.', '--config', config_str]
    if(execute_cmd(cmd, False, vs_env_vars, build_dir)):
        print("Cmake build error")
        sys.exit(1)


def build_tests_projects(arch, config_str, msbuild, vs_env_vars, toolset,
                         sdk_version, force_option=True):
    print('Building test projects')

    test_projects_application_list = [unit_test_project, qml_test_project]

    # unit tests, qml tests
    for project in test_projects_application_list:
        if (force_option):
            replace_necessary_vs_prop(project, toolset, sdk_version)

        msbuild_args = getMSBuildArgs(arch, config_str, toolset)
        build_project(msbuild, msbuild_args, project, vs_env_vars)


def run_tests(mute_jamid, output_to_files):
    print('Running client tests')

    test_exe_command_list = [qml_test_exe, unit_test_exe]

    if mute_jamid:
        test_exe_command_list[0] += ' -mutejamid'
        test_exe_command_list[1] += ' -mutejamid'
    if output_to_files:
        test_exe_command_list[0] += ' -o ' + \
            os.path.join(this_dir, 'x64', 'test', 'qml_tests.txt')
        test_exe_command_list[1] += ' > ' + \
            os.path.join(this_dir, 'x64', 'test', 'unittests.txt')

    test_result_code = 0

    # make sure that the tests are rendered offscreen
    os.environ["QT_QPA_PLATFORM"] = 'offscreen'
    os.environ["QT_QUICK_BACKEND"] = 'software'

    for test_exe_command in test_exe_command_list:
        if (execute_cmd(test_exe_command, True)):
            test_result_code = 1
    sys.exit(test_result_code)


def parse_args():
    ap = argparse.ArgumentParser(description="Client qt build tool")
    subparser = ap.add_subparsers(dest="subparser_name")

    ap.add_argument(
        '-a', '--arch', default='x64',
        help='Sets the build architecture')
    ap.add_argument(
        '-t', '--runtests', action='store_true',
        help='Build and run tests')
    ap.add_argument(
        '-b', '--beta', action='store_true',
        help='Build Qt Client in Beta Config')
    ap.add_argument(
        '-q', '--qtver', default=qt_version_default,
        help='Sets the version of Qmake')
    ap.add_argument(
        '-m', '--mute', action='store_true', default=False,
        help='Mute jamid logs')

    subparser.add_parser('init')
    subparser.add_parser('pack')

    run_test = subparser.add_parser('runtests')
    run_test.add_argument(
        '-l', '--logtests', action='store_true', default=False,
        help='Output tests log to files')

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

    if parsed_args.subparser_name == 'init':
        init_submodules()
        build_deps(parsed_args.arch)
    elif parsed_args.subparser_name == 'pack':
        print('Package generation is not yet implemented.')
        sys.exit(1)
    else:
        config = ('Release', 'Beta')[parsed_args.beta]
        build(parsed_args.arch, config,
              qt_version_default, parsed_args.runtests)
        if parsed_args.runtests:
            run_tests(parsed_args.mutejamid, parsed_args.outputtofiles)


if __name__ == '__main__':
    main()
