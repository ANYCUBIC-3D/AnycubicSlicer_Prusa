# Anycubic Slicer specific CMake

cmake_minimum_required(VERSION 2.8.12)
project(qoi)

add_library(qoi STATIC
    qoi.h
    qoilib.c
)
INSTALL(TARGETS qoi
            RUNTIME DESTINATION bin/
            LIBRARY DESTINATION lib/
            ARCHIVE DESTINATION lib/
    )

    
file(GLOB HDRS_G "${CMAKE_CURRENT_SOURCE_DIR}/*.h*")
INSTALL(FILES ${HDRS_G} DESTINATION include/qoi) 