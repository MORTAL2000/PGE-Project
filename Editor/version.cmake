# Major
set(EDITOR_VERSION_1 0)
# Minor
set(EDITOR_VERSION_2 3)
# Revision
set(EDITOR_VERSION_3 2)
# Patch
set(EDITOR_VERSION_4 0)
# Type of version: "-alpha","-beta","-dev", or "" aka "release"
set(EDITOR_VERSION_REL "-dev")

add_definitions(-DEDITOR_VERSION_1=${EDITOR_VERSION_1})
add_definitions(-DEDITOR_VERSION_2=${EDITOR_VERSION_2})
add_definitions(-DEDITOR_VERSION_3=${EDITOR_VERSION_3})
add_definitions(-DEDITOR_VERSION_4=${EDITOR_VERSION_4})
add_definitions(-DEDITOR_VERSION_REL=${EDITOR_VERSION_REL})

set(EDITOR_VERSION_STRING "${EDITOR_VERSION_1}.${EDITOR_VERSION_2}")

if(NOT ${EDITOR_VERSION_3} EQUAL 0 OR NOT ${EDITOR_VERSION_4} EQUAL 0)
    string(CONCAT EDITOR_VERSION_STRING "${EDITOR_VERSION_STRING}" ".${EDITOR_VERSION_3}")
endif()

if(NOT ${EDITOR_VERSION_4} EQUAL 0)
    string(CONCAT EDITOR_VERSION_STRING "${EDITOR_VERSION_STRING}" ".${EDITOR_VERSION_4}")
endif()

if(NOT "${EDITOR_VERSION_REL}" STREQUAL "")
    string(CONCAT EDITOR_VERSION_STRING "${EDITOR_VERSION_STRING}" "${EDITOR_VERSION_REL}")
endif()

message("== Editor version ${EDITOR_VERSION_STRING} ==")