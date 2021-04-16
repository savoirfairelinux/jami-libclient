if (EXISTS ${TIME_STAMP_FILE})
    message("No need for Qt deployment in dir " ${QML_SRC_DIR})
else()
    message("Qt deploying in dir " ${QML_SRC_DIR})
    execute_process(COMMAND "${WIN_DEPLOY_QT_PATH}/windeployqt.exe"
                            --verbose 1
                            --qmldir ${QML_SRC_DIR}
                            --release ${EXE_NAME})
    if (DEFINED OFF_SCREEN_PLUGIN_REQUESTED)
        # for not showing window when testing
        file(COPY "${OFF_SCREEN_PLUGIN_PATH}/qoffscreen.dll"
             DESTINATION ${COPY_TO_PATH})
    endif()
endif()