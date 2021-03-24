import tempfile
import re
import sys
import os
import subprocess
import platform
import argparse
import shutil
import fileinput

if platform.system() == "Windows":
    from colorama import init

    # init ANSI escape character sequences for windows
    init()


class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def execute_cmd(cmd, use_subprocess_pipe=False):
    p = subprocess.Popen(cmd,
                         shell=True,
                         stdout=subprocess.PIPE if use_subprocess_pipe else sys.stdout)
    output, _ = p.communicate()

    if use_subprocess_pipe:
        if output:
            return output
        return -1
    else:
        if p.returncode != 0:
            sys.exit()


class globalVar:
    # global var
    system_name = platform.system()
    client_dir = os.path.dirname(os.path.realpath(__file__))
    mode = "Release"
    qt_version = ""
    qt_path = ""
    daemon_path = client_dir + os.sep + '..' + os.sep + 'daemon'
    lrc_path = client_dir + os.sep + '..' + os.sep + 'lrc'
    output_path = ""

    stamp = execute_cmd('git rev-parse HEAD', True)[0:8]
    if type(stamp) is bytes:
        stamp = stamp.decode("utf-8")
    stampFile = client_dir + os.sep + ".deploy.stamp"


def check_stamp():
    if os.path.exists(globalVar.stampFile):
        with open(globalVar.stampFile) as f:
            contents = f.read()
            if contents.strip() == globalVar.stamp:
                print(bcolors.OKGREEN + "Deployment stamp up-to-date" + bcolors.ENDC)
                sys.exit()


def write_stamp():
    with open(".deploy.stamp", "w") as file:
        file.write(globalVar.stamp)


def setup_parameters(parsed_args):
    if globalVar.system_name == "Windows":
        if parsed_args.mode:
            if parsed_args.mode == "Beta":
                globalVar.mode = "Beta"

        if parsed_args.qtVersion:
            globalVar.qt_version = parsed_args.qtVersion
            qt_minor_ver = int(globalVar.qt_version.split('.')[1])
            if qt_minor_ver < 14:
                print(bcolors.WARNING + "Qt version not supported" + bcolors.ENDC)
                sys.exit()
            globalVar.qt_path = "C:\\Qt\\" + globalVar.qt_version + \
                ("\\msvc2017_64" if qt_minor_ver < 15 else "\\msvc2019_64")
        else:
            globalVar.qt_path = "C:\\Qt\\5.15.0\\msvc2019_64"
    else:
        if parsed_args.qtPath:
            globalVar.qt_path = parsed_args.qtPath
        else:
            globalVar.qt_version = execute_cmd('qmake -v', True)
            if globalVar.qt_version == -1:
                print(bcolors.WARNING + "No qmake found, trying qmake-qt5..." + bcolors.ENDC)

                globalVar.qt_version = execute_cmd('qmake-qt5 -v', True)
                if globalVar.qt_version == -1:
                    print(bcolors.FAIL + "No qmake found!" + bcolors.ENDC)
                    sys.exit()

            if type(globalVar.qt_version) is bytes:
                globalVar.qt_version = globalVar.qt_version.decode("utf-8")
            globalVar.qt_version = globalVar.qt_version.split(
                'Qt version')[1].split('in')[0].strip()
            qt_minor_ver = int(globalVar.qt_version.split('.')[1])
            if qt_minor_ver < 14:
                print(bcolors.WARNING + "Qt version not supported" + bcolors.ENDC)
                sys.exit()

    if parsed_args.daemonPath:
        globalVar.daemon_path = parsed_args.daemonPath

    if parsed_args.lrcPath:
        globalVar.lrc_path = parsed_args.lrcPath

    if parsed_args.outputPath:
        if not os.path.exists(parsed_args.outputPath):
            os.makedirs(parsed_args.outputPath)
        globalVar.output_path = parsed_args.outputPath
    else:
        if globalVar.system_name != "Windows":
            globalVar.output_path = globalVar.client_dir + os.sep + 'build-local'
        else:
            globalVar.output_path = globalVar.client_dir + \
                os.sep + "x64" + os.sep + globalVar.mode


def copy_deployment_files():
    # dependency bin files and misc
    files_to_copy = [
        globalVar.daemon_path + "\\contrib\\build\\ffmpeg\\Build\\win32\\x64\\bin\\avcodec-58.dll",
        globalVar.daemon_path + "\\contrib\\build\\ffmpeg\\Build\\win32\\x64\\bin\\avutil-56.dll",
        globalVar.daemon_path +
        "\\contrib\\build\\ffmpeg\\Build\\win32\\x64\\bin\\avformat-58.dll",
        globalVar.daemon_path +
        "\\contrib\\build\\ffmpeg\\Build\\win32\\x64\\bin\\avdevice-58.dll",
        globalVar.daemon_path +
        "\\contrib\\build\\ffmpeg\\Build\\win32\\x64\\bin\\swresample-3.dll",
        globalVar.daemon_path + "\\contrib\\build\\ffmpeg\\Build\\win32\\x64\\bin\\swscale-5.dll",
        globalVar.daemon_path + "\\contrib\\build\\ffmpeg\\Build\\win32\\x64\\bin\\avfilter-7.dll",
        globalVar.daemon_path + "\\contrib\\build\\openssl\\libcrypto-1_1-x64.dll",
        globalVar.daemon_path + "\\contrib\\build\\openssl\\libssl-1_1-x64.dll",
        globalVar.client_dir + "\\qt.conf",
        globalVar.client_dir + "\\images\\jami.ico",
        globalVar.client_dir + "\\License.rtf"
    ]

    for file in files_to_copy:
        print(bcolors.OKBLUE + "Copying: " + file +
              " -> " + globalVar.output_path + bcolors.ENDC)
        if os.path.exists(file):
            shutil.copy(file, globalVar.output_path)
        else:
            print(bcolors.FAIL + file + " does not exist" + bcolors.ENDC)
            sys.exit()

    # qt windeploy
    win_deploy_qt = globalVar.qt_path + "\\bin\\windeployqt.exe --qmldir " + \
        globalVar.client_dir + "\\src --release " + globalVar.output_path + "\\Jami.exe"
    execute_cmd(win_deploy_qt)


def copy_ringtones():
    # ringtones
    copy_to_path = globalVar.output_path + os.sep + "ringtones"
    if not os.path.exists(copy_to_path):
        os.makedirs(copy_to_path)
    ringtone_path = globalVar.client_dir + "\\..\\daemon\\ringtones"

    print(bcolors.OKCYAN + "Copying ringtones..." + bcolors.ENDC)
    for _, _, files in os.walk(ringtone_path):
        for file in files:
            print(bcolors.OKBLUE + "Copying ringtone: " +
                  file + " -> " + copy_to_path + bcolors.ENDC)
            try:
                shutil.copy(ringtone_path + os.sep + file, copy_to_path)
            except:
                print(bcolors.FAIL + "Unable to copy file." + bcolors.ENDC)


def compile_and_copy_web_resources():
    # web resources
    rcc = 'rcc'

    if globalVar.qt_path:
        rcc = globalVar.qt_path + os.sep + 'bin' + os.sep + \
            'rcc' + ('.exe' if globalVar.system_name == "Windows" else '')

    if execute_cmd(rcc + ' -v', True) == -1:
        print(bcolors.WARNING + "No rcc found, trying rcc-qt5..." + bcolors.ENDC)

        rcc = rcc.replace('rcc', 'rcc-qt5')
        if execute_cmd(rcc + ' -v', True) == -1:
            print(bcolors.FAIL + "No rcc found!" + bcolors.ENDC)
            sys.exit()

    lrc_web_resources_path = globalVar.lrc_path + os.sep + "src" + os.sep + 'webresource.qrc'

    execute_cmd(rcc + ' -binary ' + lrc_web_resources_path +
                ' -o ' + globalVar.output_path + os.sep + 'webresource.rcc')


def release_and_copy_translations():
    # translations binary
    lrelease = 'lrelease'

    if globalVar.qt_path:
        lrelease = globalVar.qt_path + os.sep + 'bin' + os.sep + \
            'lrelease' + ('.exe' if globalVar.system_name == "Windows" else '')

    if execute_cmd(lrelease + ' -version', True) == -1:
        print(bcolors.WARNING + "No lrelease found, trying lrelease-qt5..." + bcolors.ENDC)

        lrelease = lrelease.replace('lrelease', 'lrelease-qt5')
        if execute_cmd(lrelease + ' -version', True) == -1:
            print(bcolors.FAIL + "No lrelease found!" + bcolors.ENDC)
            sys.exit()

    qt_version_check = execute_cmd(lrelease + ' -version', True)
    if type(qt_version_check) is bytes:
        qt_version_check = qt_version_check.decode("utf-8")
    qt_version_check = qt_version_check.split('version')[1].strip()
    qt_minor_ver = int(qt_version_check.split('.')[1])
    if qt_minor_ver < 14:
        print(bcolors.WARNING + "Qt version not supported" + bcolors.ENDC)
        sys.exit()

    # lrc translations
    lrc_ts_path = globalVar.lrc_path + os.sep + 'translations'
    copy_to_path = globalVar.output_path + os.sep + 'share' + \
        os.sep + 'libringclient' + os.sep + 'translations'
    if not os.path.exists(copy_to_path):
        os.makedirs(copy_to_path)

    print(bcolors.OKCYAN + "Release lrc translations..." + bcolors.ENDC)
    for _, _, files in os.walk(lrc_ts_path):
        for file in files:
            if file.endswith(".ts"):
                execute_cmd(lrelease + " " + lrc_ts_path + os.sep + file)

    for _, _, files in os.walk(lrc_ts_path):
        for file in files:
            if file.endswith(".qm"):
                print(bcolors.OKBLUE + "Copying translation file: " +
                      file + " -> " + copy_to_path + bcolors.ENDC)
                shutil.copy(lrc_ts_path + os.sep + file, copy_to_path)

    # client translations
    client_ts_path = globalVar.client_dir + os.sep + 'translations'
    copy_to_path = globalVar.output_path + os.sep + \
        'share' + os.sep + 'ring' + os.sep + 'translations'
    if not os.path.exists(copy_to_path):
        os.makedirs(copy_to_path)

    print(bcolors.OKCYAN + "Release client translations..." + bcolors.ENDC)
    for _, _, files in os.walk(client_ts_path):
        for file in files:
            if file.endswith(".ts"):
                execute_cmd(lrelease + " " +
                            client_ts_path + os.sep + file)

    for _, _, files in os.walk(client_ts_path):
        for file in files:
            if file.endswith(".qm"):
                print(bcolors.OKBLUE + "Copying translation file: " +
                      file + " -> " + copy_to_path + bcolors.ENDC)
                shutil.copy(client_ts_path + os.sep + file, copy_to_path)


def parse_args():
    ap = argparse.ArgumentParser(description="Copy runtime files tool")
    if globalVar.system_name != "Windows":
        ap.add_argument(
            '-q', '--qtPath', default='',
            help='Qt path')
    else:
        ap.add_argument(
            '-m', '--mode', default='',
            help='Release or Beta mode')

        ap.add_argument(
            '-q', '--qtVersion', default='',
            help='Qt version')

    ap.add_argument(
        '-d', '--daemonPath', default='',
        help='Daemon path')
    ap.add_argument(
        '-l', '--lrcPath', default='',
        help='Lrc path')
    ap.add_argument(
        '-o', '--outputPath', default='',
        help='Output path')

    parsed_args = ap.parse_args()

    return parsed_args


def main():
    # check stamp
    check_stamp()

    # parse args
    parsed_args = parse_args()

    # set up global var
    setup_parameters(parsed_args)

    print(bcolors.OKCYAN + "****************************************" + bcolors.ENDC)
    print(bcolors.OKBLUE + "copying deployment files..." + bcolors.ENDC)
    print(bcolors.OKBLUE + "using daemonDir:    " +
          globalVar.daemon_path + bcolors.ENDC)
    print(bcolors.OKBLUE + "using lrcDir:       " +
          globalVar.lrc_path + bcolors.ENDC)
    if globalVar.qt_path:
        print(bcolors.OKBLUE + "using QtDir:        " +
              globalVar.qt_path + bcolors.ENDC)
    else:
        print(bcolors.OKBLUE + "using system Qt" + bcolors.ENDC)
    if globalVar.system_name == "Windows":
        print(bcolors.OKBLUE + globalVar.mode + " mode" + bcolors.ENDC)
    print(bcolors.OKCYAN + "****************************************" + bcolors.ENDC)

    # deployment
    if globalVar.system_name == "Windows":
        copy_deployment_files()
        copy_ringtones()

    # translations
    release_and_copy_translations()

    # web resources
    compile_and_copy_web_resources()

    # write stamp
    write_stamp()

    print(bcolors.OKGREEN + "Copy completed" + bcolors.ENDC)


if __name__ == '__main__':
    main()
