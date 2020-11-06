find_package(Qt5 CONFIG REQUIRED QuickTest Test)
find_package(GTest REQUIRED)

enable_testing(true)
set(QMLTEST_LIBS ${QML_LIBS} Qt5::QuickTest)

set(TESTS_INCLUDES
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests/qml
    ${CMAKE_SOURCE_DIR}/tests/unittests)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${LRC}/include/libringclient
    ${LRC}/include)

# Common jami files
add_library(test_common_obj OBJECT ${COMMON_SOURCES} ${COMMON_HEADERS})
target_link_libraries(test_common_obj ${QMLTEST_LIBS})
target_compile_definitions(test_common_obj PRIVATE ENABLE_TESTS="ON")

# QML tests
add_executable(qml_tests
    ${CMAKE_SOURCE_DIR}/tests/qml/main.cpp
    ${QML_RESOURCES}
    ${QML_RESOURCES_QML}
    $<TARGET_OBJECTS:test_common_obj>)

target_link_libraries(qml_tests
    ${QMLTEST_LIBS}
    ${test_common_objects}
    ${ringclient}
    ${qrencode})

target_include_directories(qml_tests PUBLIC
    ${TESTS_INCLUDES}
    ${LRC}/include/libringclient
    ${LRC}/include)

target_compile_definitions(qml_tests PRIVATE ENABLE_TESTS="ON")

add_test(NAME QmlTests COMMAND qml_tests -input ${PROJECT_SOURCE_DIR}/tests/qml/)

# Unittests
add_executable(unittests
    ${CMAKE_SOURCE_DIR}/tests/unittests/main_unittests.cpp
    ${CMAKE_SOURCE_DIR}/tests/unittests/dummy_unittest.cpp
    ${QML_RESOURCES}
    ${QML_RESOURCES_QML}
    $<TARGET_OBJECTS:test_common_obj>)

target_link_libraries(unittests
    ${QMLTEST_LIBS}
    ${test_common_objects}
    ${ringclient}
    ${qrencode}
    gtest
    pthread)

target_include_directories(unittests PUBLIC
    ${TESTS_INCLUDES}
    ${LRC}/include/libringclient
    ${LRC}/include)

target_compile_definitions(unittests PRIVATE ENABLE_TESTS="ON")

add_test(NAME UnitTests COMMAND unittests)
