if (EXISTS ${TIME_STAMP_FILE})
    message("No need for daemon deployment")
else()
    message("Daemon deploying ...")
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
              "${PROJECT_ROOT_DIR}/resources/images/jami.ico"
              "${PROJECT_ROOT_DIR}/License.rtf"
         DESTINATION ${COPY_TO_PATH})
    # Cannot copy symbolic link using file COPY, create insread.
    file(GLOB_RECURSE RingTones "${DRING_PATH}/ringtones/*.ul"
                                "${DRING_PATH}/ringtones/*.ogg"
                                "${DRING_PATH}/ringtones/*.wav"
                                "${DRING_PATH}/ringtones/*.opus")
    list(REMOVE_ITEM RingTones "${DRING_PATH}/ringtones/default.opus")
    file(COPY ${RingTones}
         DESTINATION ${COPY_TO_PATH}/ringtones)
    file(CREATE_LINK "${COPY_TO_PATH}/ringtones/01_AfroNigeria.opus"
                     "${COPY_TO_PATH}/ringtones/default.opus")
endif()
