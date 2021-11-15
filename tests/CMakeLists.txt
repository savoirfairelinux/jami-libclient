find_package(Qt5 CONFIG REQUIRED QuickTest Test)

if(MSVC)
    # Download and unpack googletest for windows
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
    )
    # for Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
else()
    find_package(GTest REQUIRED)
endif()

enable_testing(true)
set(QML_TEST_LIBS ${QML_LIBS} Qt5::QuickTest Qt5::Test)
set(TESTS_INCLUDES
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests/qml
    ${CMAKE_SOURCE_DIR}/tests/unittests)
set(TEST_QML_RESOURCES ${CMAKE_SOURCE_DIR}/tests/qml/resources.qrc)

# Common jami files
add_library(test_common_obj OBJECT ${COMMON_SOURCES} ${COMMON_HEADERS})
target_link_libraries(test_common_obj ${QML_TEST_LIBS})
target_compile_definitions(test_common_obj PRIVATE ENABLE_TESTS="ON")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src)

# QML tests
add_executable(qml_tests
               ${CMAKE_SOURCE_DIR}/tests/qml/main.cpp
               ${QML_RESOURCES}
               ${QML_RESOURCES_QML}
               ${TEST_QML_RESOURCES}
               $<TARGET_OBJECTS:test_common_obj>)

target_link_libraries(qml_tests
                      ${QML_TEST_LIBS}
                      ${test_common_objects})

target_compile_definitions(qml_tests PRIVATE ENABLE_TESTS="ON")

# Unittests
set(UNIT_TESTS_HEADER_FILES ${CMAKE_SOURCE_DIR}/tests/unittests/globaltestenvironment.h)

set(UNIT_TESTS_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/tests/unittests/main_unittest.cpp
    ${CMAKE_SOURCE_DIR}/tests/unittests/account_unittest.cpp
    ${CMAKE_SOURCE_DIR}/tests/unittests/contact_unittest.cpp)

add_executable(unittests
               ${UNIT_TESTS_HEADER_FILES}
               ${UNIT_TESTS_SOURCE_FILES}
               ${QML_RESOURCES}
               ${QML_RESOURCES_QML}
               $<TARGET_OBJECTS:test_common_obj>)

target_link_libraries(unittests
                      ${QML_TEST_LIBS}
                      ${test_common_objects}
                      gtest)

target_compile_definitions(unittests PRIVATE ENABLE_TESTS="ON")

if(MSVC)
    include_directories(${LRC_SRC_PATH}
                        ${DRING_SRC_PATH})

    # QML tests
    target_link_libraries(qml_tests
                          ${QTWRAPPER_LIB}
                          ${RINGCLIENT_STATIC_LIB}
                          ${QRENCODE_LIB}
                          ${GNUTLS_LIB}
                          ${DRING_LIB}
                          ${WINDOWS_SYS_LIBS})

    target_include_directories(qml_tests PUBLIC
                               ${TESTS_INCLUDES}
                               ${LRC_SRC_PATH}
                               ${DRING_SRC_PATH})

    # output test executable files into test folder
    set_target_properties(qml_tests
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/x64/test"
    )

    # POST_BUILD steps

    # check time stamp
    set(TIME_STAMP_FILE ".deploy.stamp")
    add_custom_command(TARGET qml_tests POST_BUILD
                       WORKING_DIRECTORY "$<TARGET_FILE_DIR:qml_tests>"
                       COMMAND ${CMAKE_COMMAND} -DTIME_STAMP_FILE=${TIME_STAMP_FILE}
                               -P ${PROJECT_SOURCE_DIR}/cmake/time_stamp_check.cmake)

    # daemon deploy
    add_custom_command(TARGET qml_tests POST_BUILD
                       WORKING_DIRECTORY "$<TARGET_FILE_DIR:qml_tests>"
                       COMMAND ${CMAKE_COMMAND} -DTIME_STAMP_FILE=${TIME_STAMP_FILE}
                               -DCOPY_TO_PATH=$<TARGET_FILE_DIR:qml_tests>
                               -DDRING_PATH=${DRING}
                               -DPROJECT_ROOT_DIR=${PROJECT_SOURCE_DIR}
                               -P ${PROJECT_SOURCE_DIR}/cmake/windows_daemon_deploy.cmake)

    # Qt deploy for test qmls
    add_custom_command(TARGET qml_tests POST_BUILD
                       WORKING_DIRECTORY "$<TARGET_FILE_DIR:qml_tests>"
                       COMMAND ${CMAKE_COMMAND} -DTIME_STAMP_FILE=${TIME_STAMP_FILE}
                               -DWIN_DEPLOY_QT_PATH=${CMAKE_PREFIX_PATH}/bin
                               -DQML_SRC_DIR=${CMAKE_SOURCE_DIR}/tests/qml
                               -DEXE_NAME=$<TARGET_FILE:qml_tests>
                               -DOFF_SCREEN_PLUGIN_REQUESTED=TRUE
                               -DCOPY_TO_PATH=$<TARGET_FILE_DIR:qml_tests>/platforms
                               -DOFF_SCREEN_PLUGIN_PATH=${CMAKE_PREFIX_PATH}/plugins/platforms
                               -P ${PROJECT_SOURCE_DIR}/cmake/windows_qt_deploy.cmake)

    # Qt deploy for src qmls
    add_custom_command(TARGET qml_tests POST_BUILD
                       WORKING_DIRECTORY "$<TARGET_FILE_DIR:qml_tests>"
                       COMMAND ${CMAKE_COMMAND} -DTIME_STAMP_FILE=${TIME_STAMP_FILE}
                               -DWIN_DEPLOY_QT_PATH=${CMAKE_PREFIX_PATH}/bin
                               -DQML_SRC_DIR=${SRC_DIR}
                               -DEXE_NAME=$<TARGET_FILE:qml_tests>
                               -P ${PROJECT_SOURCE_DIR}/cmake/windows_qt_deploy.cmake)

    # create time stamp
    add_custom_command(TARGET qml_tests POST_BUILD
                       WORKING_DIRECTORY "$<TARGET_FILE_DIR:qml_tests>"
                       COMMAND ${CMAKE_COMMAND} -DTIME_STAMP_FILE=${TIME_STAMP_FILE}
                               -P ${PROJECT_SOURCE_DIR}/cmake/time_stamp_create.cmake)

    # Unittests
    target_link_libraries(unittests
                          ${QTWRAPPER_LIB}
                          ${RINGCLIENT_STATIC_LIB}
                          ${QRENCODE_LIB}
                          ${GNUTLS_LIB}
                          ${DRING_LIB}
                          ${WINDOWS_SYS_LIBS})

    target_include_directories(unittests PUBLIC
                               ${TESTS_INCLUDES}
                               ${LRC_SRC_PATH}
                               ${DRING_SRC_PATH})

    # output test executable files into test folder
    set_target_properties(unittests
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/x64/test"
    )
else()
    include_directories(${LRC}/include/libringclient
                        ${LRC}/include
                        ${LIBNM_INCLUDE_DIRS}
                        ${LIBNOTIFY_INCLUDE_DIRS}
                        ${LIBGDKPIXBUF_INCLUDE_DIRS})

    # QML tests
    target_link_libraries(qml_tests
                          ${ringclient}
                          ${qrencode}
                          ${X11}
                          ${LIBNM_LIBRARIES}
                          ${LIBNOTIFY_LIBRARIES}
                          ${LIBGDKPIXBUF_LIBRARIES})

    target_include_directories(qml_tests PUBLIC
                               ${TESTS_INCLUDES}
                               ${LRC}/include/libringclient
                               ${LRC}/include)

    add_test(NAME QmlTests COMMAND qml_tests -input ${PROJECT_SOURCE_DIR}/tests/qml/)

    # Unittests
    target_link_libraries(unittests
                          ${ringclient}
                          ${qrencode}
                          pthread
                          ${X11}
                          ${LIBNM_LIBRARIES}
                          ${LIBNOTIFY_LIBRARIES}
                          ${LIBGDKPIXBUF_LIBRARIES})

    target_include_directories(unittests PUBLIC
                               ${TESTS_INCLUDES}
                               ${LRC}/include/libringclient
                               ${LRC}/include)

    add_test(NAME UnitTests COMMAND unittests)
endif()