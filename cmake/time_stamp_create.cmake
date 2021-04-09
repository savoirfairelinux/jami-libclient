execute_process(COMMAND git rev-parse HEAD
                OUTPUT_VARIABLE VERSION_PATCH)
if (EXISTS ${TIME_STAMP_FILE})
    message("Keep the old time stamp")
else()
    message("Creating time stamp ...")
    file(WRITE ${TIME_STAMP_FILE} "${VERSION_PATCH}")
endif()