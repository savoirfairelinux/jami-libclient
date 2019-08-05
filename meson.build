project('libringclient', 'cpp',
        version: '1.0.0',
        license: 'GPL3+',
        default_options: ['cpp_std=gnu++17', 'buildtype=debugoptimized'],
        meson_version:'>= 0.50' )

modqt5 = import('qt5')
modpkgconfig = import('pkgconfig')

depqt5 = dependency('qt5', modules: ['Core', 'Gui', 'Sql'])
depsigcpp = dependency('sigc++-2.0', version: '>=2.10.2')
depthreads = dependency('threads')

# https://doc.qt.io/qt-5/signalsandslots.html#3rd-party-signals-and-slots
add_project_arguments('-DQT_NO_KEYWORDS', language: 'cpp')
moc_args = ['-DQT_NO_KEYWORDS'] # for meta-object compiler

pathdaemon = get_option('daemon-install-prefix')
if pathdaemon == ''
  pathdaemon = get_option('prefix')
endif

if get_option('daemon-mute')
  add_project_arguments('-DDAEMON_MUTING_IS_ON', language: 'cpp')
  moc_args += '-DDAEMON_MUTING_IS_ON'
endif

if get_option('video')
  add_project_arguments('-DENABLE_VIDEO', language: 'cpp')
  moc_args += '-DENABLE_VIDEO'
endif

if host_machine.system() == 'windows'
  add_project_arguments('-DWINDOWS', language: 'cpp')
  moc_args += '-DWINDOWS'
endif

libjamiclient_incdirs = include_directories(
  'src'
)

libjamiclient_sources = files(
  #Data objects
  'src/uri.cpp',
  'src/video/renderer.cpp',

  #Models
  'src/contactmodel.cpp',
  'src/newcallmodel.cpp',
  'src/newdevicemodel.cpp',
  'src/newcodecmodel.cpp',
  'src/conversationmodel.cpp',
  'src/database.cpp',
  'src/authority/daemon.cpp',
  'src/authority/storagehelper.cpp',
  'src/lrc.cpp',
  'src/newaccountmodel.cpp',
  'src/peerdiscoverymodel.cpp',
  'src/callbackshandler.cpp',
  'src/behaviorcontroller.cpp',
  'src/datatransfermodel.cpp',
  'src/newvideo.cpp',
  'src/shmrenderer.cpp',
  'src/directrenderer.cpp',

  #Default interface implementations
  'src/globalinstances.cpp',
  'src/pixmapmanipulatordefault.cpp',
  'src/dbuserrorhandlerdefault.cpp',

  #Other
  'src/avmodel.cpp',
  'src/namedirectory.cpp',
  'src/smartinfohub.cpp'
)

libjamiclient_deps = [ depqt5, depsigcpp, depthreads ]

if get_option('daemon-interface') == 'library'
  depjami = meson.get_compiler('cpp').find_library('ring', required: false)
  if not depjami.found()
    depjami = meson.get_compiler('cpp').find_library('ring', dirs: pathdaemon / get_option('libdir'))
  endif

  libjamiclient_incdirs = [libjamiclient_incdirs, pathdaemon / get_option('includedir') / 'dring']
  libjamiclient_sources += 'src/daemonproxies/daemonproxy-library.cpp'
  libjamiclient_deps += depjami

  add_project_arguments('-DDAEMON_INTERFACE_IS_LIBRARY', language: 'cpp')
  moc_args += '-DDAEMON_INTERFACE_IS_LIBRARY'

elif get_option('daemon-interface') == 'dbus'
  depsdbuscpp = dependency('sdbus-c++', version: '>= 0.7.2')
  libjamiclient_deps += depsdbuscpp
  gen = generator('sdbus-c++-xml2cpp', arguments: ['--proxy', '@OUTPUT@', '@INPUT@'],
                  output: '@BASENAME@.proxy.h')

  pathinterfaces = pathdaemon / get_option('datadir') / 'dbus-1' / 'interfaces'
  dbusinterfaces = files(
    pathinterfaces / 'net.jami.daemon1.CallManager.xml',
    pathinterfaces / 'net.jami.daemon1.ConfigurationManager.xml',
    pathinterfaces / 'net.jami.daemon1.DataTransfer.xml',
    pathinterfaces / 'net.jami.daemon1.Instance.xml',
    pathinterfaces / 'net.jami.daemon1.PresenceManager.xml' )
  if get_option('video')
    dbusinterfaces += pathinterfaces / 'net.jami.daemon1.VideoManager.xml'
  endif
  proxies = gen.process(dbusinterfaces)
endif

libjamiclient_sources += modqt5.preprocess(
  moc_headers: [
    'src/api/avmodel.h',
    'src/api/behaviorcontroller.h',
    'src/api/contactmodel.h',
    'src/api/conversationmodel.h',
    'src/api/datatransfermodel.h',
    'src/api/newaccountmodel.h',
    'src/api/newcallmodel.h',
    'src/api/newcodecmodel.h',
    'src/api/newdevicemodel.h',
    'src/api/newvideo.h',
    'src/api/peerdiscoverymodel.h',
    'src/callbackshandler.h',
    'src/database.h',
    'src/directrenderer.h',
    'src/namedirectory.h',
    'src/private/namedirectory_p.h',
    'src/private/smartInfoHub_p.h',
    'src/private/videorenderer_p.h',
    'src/shmrenderer.h',
    'src/smartinfohub.h',
    'src/video/renderer.h'
  ],
  moc_sources: [ # must include "<basename>.moc"
    'src/avmodel.cpp',
    'src/contactmodel.cpp',
    'src/conversationmodel.cpp',
    'src/datatransfermodel.cpp',
    'src/directrenderer.cpp',
    'src/newaccountmodel.cpp',
    'src/newcodecmodel.cpp',
    'src/newdevicemodel.cpp',
    'src/peerdiscoverymodel.cpp',
    'src/shmrenderer.cpp'
  ],
  include_directories: libjamiclient_incdirs,
  dependencies: libjamiclient_deps,
  moc_extra_arguments: moc_args )

libjamiclient = library('ringclient',
                        libjamiclient_sources,
                        include_directories: libjamiclient_incdirs,
                        dependencies: libjamiclient_deps,
                        install: true )

modpkgconfig.generate(libjamiclient,
                      name: 'libjamiclient',
                      filebase: 'libjamiclient',
                      description: 'Jami client communication library' )

# We don't use modqt5.compile_translations() because of the upstream bug:
# https://github.com/mesonbuild/meson/issues/5019
translations = [
  'lrc_ar',
  'lrc_bg',
  'lrc_ca',
  'lrc_cs_CZ',
  'lrc_da_DK',
  'lrc_da',
  'lrc_de_DE',
  'lrc_de',
  'lrc_el',
  'lrc_en_GB',
  'lrc_en',
  'lrc_eo',
  'lrc_es_AR',
  'lrc_es',
  'lrc_et_EE',
  'lrc_eu',
  'lrc_fa_IR',
  'lrc_fi',
  'lrc_fr_BE',
  'lrc_fr_CA',
  'lrc_fr_CH',
  'lrc_fr_FR',
  'lrc_fr',
  'lrc_he',
  'lrc_hi_IN',
  'lrc_hr',
  'lrc_hu',
  'lrc_id',
  'lrc_it_IT',
  'lrc_it',
  'lrc_ja',
  'lrc_ko_KR',
  'lrc_lt',
  'lrc_ms',
  'lrc_nb',
  'lrc_nl_BE',
  'lrc_nl_NL',
  'lrc_nl',
  'lrc_pa',
  'lrc_pl_PL',
  'lrc_pl',
  'lrc_pt_BR',
  'lrc_pt_PT',
  'lrc_pt',
  'lrc_ro',
  'lrc_ru_RU',
  'lrc_ru',
  'lrc_sk_SK',
  'lrc_sl',
  'lrc_sq_AL',
  'lrc_sv',
  'lrc_tr',
  'lrc_uk',
  'lrc_zh_CN',
  'lrc_zh',
  'lrc_zh_TW'
]

foreach translation: translations
  custom_target('translation-' + translation,
                command: ['lrelease', '@INPUT@', '-qm', '@OUTPUT@'],
                input: 'translations/'+translation+'.ts', output: translation+'.qm',
                build_by_default: true,
                install: true,
                install_dir: get_option('prefix') / get_option('datadir') / meson.project_name() / 'translations' )
endforeach

install_subdir('src/api',
               install_dir: get_option('prefix') / get_option('includedir') / meson.project_name() / 'api',
               strip_directory: true )

install_subdir('src/interfaces',
               install_dir: get_option('prefix') / get_option('includedir') / meson.project_name() / 'interfaces',
               strip_directory: true )

install_headers('src/video/renderer.h', subdir: meson.project_name() / 'video')
install_headers('src/smartinfohub.h', subdir: meson.project_name())
install_headers('src/typedefs.h', subdir: meson.project_name())
install_headers('src/globalinstances.h', subdir: meson.project_name())
install_headers('src/namedirectory.h', subdir: meson.project_name())
