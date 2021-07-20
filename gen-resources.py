import os
import sys
import re

resdir = 'resources'
qmlfile = os.path.join('src', 'constant', 'JamiResources.qml')
sep = '_'

# replace characters that aren't valid within QML property names
formatProp = lambda str: (
    "".join([{".": sep, "-": sep, " ": sep}
        .get(c, c) for c in str]
    ).lower())

with open('resources.qrc', 'w') as qrc, open(qmlfile, 'w') as qml:
    qrc.write('<RCC>\n')
    qml.write('pragma Singleton\nimport QtQuick 2.14\nQtObject {\n')
    for root, _, files in os.walk(resdir):
        if len(files):
            prefix = root.rsplit(os.sep, 1)[-1]
            qrc.write('\t<qresource prefix="/%s">\n' % prefix)
            for filename in files:
                # use posix separators in the resource path
                filepath = os.path.join(root, filename).replace(os.sep, '/')
                qrc.write('\t\t<file alias="%s">%s</file>\n'
                    % (filename, filepath))
                # only record images/icons as properties
                if (re.match("icons|images", prefix)):
                    qml.write('    readonly property string %s: "qrc:/%s"\n'
                        % (formatProp(filename), filepath.split('/', 1)[1]))
            qrc.write('\t</qresource>\n')
    qml.write('}')
    qrc.write('</RCC>')