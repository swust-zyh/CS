# cmake version
cmake_minimum_required(VERSION 3.22.1)

# project name
project(http_thread_server)

# add_subdirectory(src ${PROJECT_SOURCE_DIR}/bin)
# aux_source_directory(./lib LIB_SRCS)

# include_directories()

# set(LIBRARY_SRC )
# add_library(_static STATIC ${LIBRARY_SRC})
# SET_TARGET_PROPERTIES(_static PROPERTIES OUTPUT_NAME "")
# SET_TARGET_PROPERTIES(_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
# add_library( SHARED ${LIBRARY_SRC})
# SET_TARGET_PROPERTIES( PROPERTIES OUTPUT_NAME "")
# SET_TARGET_PROPERTIES( PROPERTIES CLEAN_DIRECT_OUTPUT 1)

# add_executable()
# TARGET_LINK_LIBRARIES()

# install()
install(DIRECTORY lib/ DESTINATION lib)
install(FILES dbconf.json tpoolconf.json DESTINATION local/etc)
install(DIRECTORY html_docs DESTINATION local/share)
