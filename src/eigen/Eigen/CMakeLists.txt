file(GLOB Eigen_directory_files_to_install "*")
file(GLOB TXTS_G "${CMAKE_CURRENT_SOURCE_DIR}/*.txt" )

foreach(item ${TXTS_G})
    list(REMOVE_ITEM Eigen_directory_files_to_install ${item})
endforeach(item)
list(REMOVE_ITEM Eigen_directory_files_to_install "${CMAKE_CURRENT_SOURCE_DIR}/src")

INSTALL(FILES ${Eigen_directory_files_to_install} DESTINATION include/Eigen)



file(GLOB_RECURSE HDRS_G "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
foreach(_source ${HDRS_G})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "/" "\\" _group_path "${_source_path}")
    string(REPLACE  "${CMAKE_CURRENT_SOURCE_DIR}/src" "" _group_path "${_source_path}")
    INSTALL(FILES ${_source} DESTINATION include/Eigen/src/${_group_path})
endforeach(_source)