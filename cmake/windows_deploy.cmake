if (EXISTS ${TIME_STAMP_FILE})
    message("No need to deploy")
else()
    message("Deploying ...")
    file(COPY "${DRING_PATH}/contrib/build/ffmpeg/Build/win32/x64/bin/avcodec-58.dll"
              "${DRING_PATH}/contrib/build/ffmpeg/Build/win32/x64/bin/avutil-56.dll"
              "${DRING_PATH}/contrib/build/ffmpeg/Build/win32/x64/bin/avformat-58.dll"
              "${DRING_PATH}/contrib/build/ffmpeg/Build/win32/x64/bin/avdevice-58.dll"
              "${DRING_PATH}/contrib/build/ffmpeg/Build/win32/x64/bin/swresample-3.dll"
              "${DRING_PATH}/contrib/build/ffmpeg/Build/win32/x64/bin/swscale-5.dll"
              "${DRING_PATH}/contrib/build/ffmpeg/Build/win32/x64/bin/avfilter-7.dll"
              "${DRING_PATH}/contrib/build/openssl/libcrypto-1_1-x64.dll"
              "${DRING_PATH}/contrib/build/openssl/libssl-1_1-x64.dll"
              "${PROJECT_ROOT_DIR}/qt.conf"
              "${PROJECT_ROOT_DIR}/images/jami.ico"
              "${PROJECT_ROOT_DIR}/License.rtf"
         DESTINATION ${COPY_TO_PATH})
    execute_process(COMMAND "${WIN_DEPLOY_QT_PATH}/windeployqt.exe"
                            --verbose 1
                            --qmldir ${QML_SRC_DIR}
                            --release ${EXE_NAME})
endif()