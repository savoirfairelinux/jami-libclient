
    message("Qt deploying in dir " ${QML_SRC_DIR})
    execute_process(COMMAND "${MAC_DEPLOY_QT_PATH}/macdeployqt"
                            ${EXE_NAME}
                            -qmldir=${QML_SRC_DIR})
