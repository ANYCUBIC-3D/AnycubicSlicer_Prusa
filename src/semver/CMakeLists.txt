cmake_minimum_required(VERSION 2.8.12)
project(semver)

add_library(semver STATIC
    semver.c
    semver.h
)

encoding_check(semver)
INSTALL(TARGETS semver
            RUNTIME DESTINATION bin/
            LIBRARY DESTINATION lib/
            ARCHIVE DESTINATION lib/
    )

INSTALL(FILES semver.h DESTINATION include/semver)   