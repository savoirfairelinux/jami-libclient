execute_process(COMMAND git rev-parse HEAD
                OUTPUT_VARIABLE VERSION_PATCH)

# remove leading and trailing spaces
string(STRIP "${VERSION_PATCH}" VERSION_PATCH)

message("Checking time stamp ...")
if(EXISTS ${TIME_STAMP_FILE})
    file (STRINGS ${TIME_STAMP_FILE} VERSION_IN_FILE)
    if(NOT "${VERSION_IN_FILE}" STREQUAL "${VERSION_PATCH}")
        file (REMOVE "${TIME_STAMP_FILE}")
    endif()
endif()