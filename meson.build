project('libjamiclient', 'cpp',
        version: '1.0.0',
        license: 'GPL3+',
        default_options: ['cpp_std=gnu++14', 'buildtype=debugoptimized'],
        meson_version:'>= 0.50' )

modqt5 = import('qt5')
depqt5 = dependency('qt5', modules: ['Core', 'Sql'])

pathdaemon = get_option('daemon-install-prefix')
if pathdaemon == ''
  pathdaemon = get_option('prefix')
endif

if get_option('video')
  add_project_arguments('-DENABLE_VIDEO', language: 'cpp')
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
  'src/authority/databasehelper.cpp',
  'src/lrc.cpp',
  'src/newaccountmodel.cpp',
  'src/peerdiscoverymodel.cpp',
  'src/callbackshandler.cpp',
  'src/behaviorcontroller.cpp',
  'src/datatransfermodel.cpp',
  'src/newvideo.cpp',
  'src/shmrenderer.cpp',
  'src/directrenderer.cpp',

  #Communication
  'src/dbus/configurationmanager.cpp',
  'src/dbus/callmanager.cpp',
  'src/dbus/instancemanager.cpp',
  'src/dbus/videomanager.cpp',
  'src/dbus/presencemanager.cpp',

  #Default interface implementations
  'src/globalinstances.cpp',
  'src/pixmapmanipulatordefault.cpp',
  'src/dbuserrorhandlerdefault.cpp',

  #Other
  'src/avmodel.cpp',
  'src/namedirectory.cpp',
  'src/smartinfohub.cpp'
)

libjamiclient_deps = [ depqt5 ]

if get_option('daemon-interface') == 'library'
  add_project_arguments('-DENABLE_LIBWRAP', language: 'cpp')
  depjami = meson.get_compiler('cpp').find_library('ring', dirs: pathdaemon / get_option('libdir'))

  libjamiclient_incdirs = [libjamiclient_incdirs, pathdaemon / get_option('includedir') / 'dring']
  libjamiclient_deps += depjami

elif get_option('daemon-interface') == 'dbus'
  depsdbuscpp = dependency('sdbus-c++')
  libjamiclient_deps += depsdbuscpp
  gen = generator('sdbus-c++-xml2cpp', arguments: ['--proxy', '@OUTPUT@', '@INPUT@'], 
                  output: '@BASENAME@.proxy.h')

  pathinterfaces = pathdaemon / get_option('datadir') / 'dbus-1' / 'interfaces'
  dbusinterfaces = [
    pathinterfaces / 'net.jami.daemon1.CallManager.xml',
    pathinterfaces / 'net.jami.daemon1.ConfigurationManager.xml',
    pathinterfaces / 'net.jami.daemon1.DataTransfer.xml',
    pathinterfaces / 'net.jami.daemon1.Instance.xml',
    pathinterfaces / 'net.jami.daemon1.PresenceManager.xml' ]
  if get_option('video')
    dbusinterfaces += pathinterfaces / 'net.jami.daemon1.VideoManager.xml'
  endif
  proxies = gen.process(dbusinterfaces)
endif

libjamiclient_sources += modqt5.preprocess(
#  moc_headers : 'mainWindow.h',   # Only headers that need moc should be put here
#  moc_sources : 'helperFile.cpp', # must have #include "moc_helperFile.cpp"
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
    'src/qtwrapper/callmanager_wrap.h',
    'src/qtwrapper/configurationmanager_wrap.h',
    'src/qtwrapper/instancemanager_wrap.h',
    'src/qtwrapper/presencemanager_wrap.h',
    'src/qtwrapper/videomanager_wrap.h',
    'src/shmrenderer.h',
    'src/smartinfohub.h',
    'src/video/renderer.h' ],
  moc_sources: [
    'src/avmodel.cpp',
    'src/conversationmodel.cpp',
    'src/contactmodel.cpp',
    'src/datatransfermodel.cpp',
    'src/directrenderer.cpp',
    'src/newaccountmodel.cpp',
    'src/newcodecmodel.cpp',
    'src/newdevicemodel.cpp',
    'src/peerdiscoverymodel.cpp',
    'src/shmrenderer.cpp' ],
  include_directories: libjamiclient_incdirs,
  dependencies: libjamiclient_deps )

libjamiclient = library('jamiclient',
                        libjamiclient_sources,
                        include_directories: libjamiclient_incdirs,
                        dependencies: libjamiclient_deps,
                        install: true )

translations = modqt5.compile_translations(
  ts_files: [    
    'translations/lrc_ar.ts',
    'translations/lrc_bg.ts',
    'translations/lrc_ca.ts',
    'translations/lrc_cs_CZ.ts',
    'translations/lrc_da_DK.ts',
    'translations/lrc_da.ts',
    'translations/lrc_de_DE.ts',
    'translations/lrc_de.ts',
    'translations/lrc_el.ts',
    'translations/lrc_en_GB.ts',
    'translations/lrc_en.ts',
    'translations/lrc_eo.ts',
    'translations/lrc_es_AR.ts',
    'translations/lrc_es.ts',
    'translations/lrc_et_EE.ts',
    'translations/lrc_eu.ts',
    'translations/lrc_fa_IR.ts',
    'translations/lrc_fi.ts',
    'translations/lrc_fr_BE.ts',
    'translations/lrc_fr_CA.ts',
    'translations/lrc_fr_CH.ts',
    'translations/lrc_fr_FR.ts',
    'translations/lrc_fr.ts',
    'translations/lrc_he.ts',
    'translations/lrc_hi_IN.ts',
    'translations/lrc_hr.ts',
    'translations/lrc_hu.ts',
    'translations/lrc_id.ts',
    'translations/lrc_it_IT.ts',
    'translations/lrc_it.ts',
    'translations/lrc_ja.ts',
    'translations/lrc_ko_KR.ts',
    'translations/lrc_lt.ts',
    'translations/lrc_ms.ts',
    'translations/lrc_nb.ts',
    'translations/lrc_nl_BE.ts',
    'translations/lrc_nl_NL.ts',
    'translations/lrc_nl.ts',
    'translations/lrc_pa.ts',
    'translations/lrc_pl_PL.ts',
    'translations/lrc_pl.ts',
    'translations/lrc_pt_BR.ts',
    'translations/lrc_pt_PT.ts',
    'translations/lrc_pt.ts',
    'translations/lrc_ro.ts',
    'translations/lrc_ru_RU.ts',
    'translations/lrc_ru.ts',
    'translations/lrc_sk_SK.ts',
    'translations/lrc_sl.ts',
    'translations/lrc_sq_AL.ts',
    'translations/lrc_sv.ts',
    'translations/lrc_tr.ts',
    'translations/lrc_uk.ts',
    'translations/lrc_zh_CN.ts',
    'translations/lrc_zh.ts',
    'translations/lrc_zh_TW.ts' ],
  build_by_default: true,
  install: true )

install_subdir('src/api',
               install_dir: get_option('prefix') / get_option('includedir') / meson.project_name(),
               strip_directory: true )
