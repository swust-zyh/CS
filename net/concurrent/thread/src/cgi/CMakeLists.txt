# cmake version
cmake_minimum_required(VERSION 3.22.1)

# project name
project(http_thread_server)

set (EXECUTABLE_OUTPUT_PATH /usr/local/share/html_docs/cgi)
# add_subdirectory()
# aux_source_directory()

# include_directories()
link_libraries (
  "/usr/lib/libPool.so"
  "/usr/lib/libMysqlConn.so"
  "/usr/lib/libConnectionPool.so"
)

# set(LIBRARY_SRC )
# add_library(_static STATIC ${LIBRARY_SRC})
# SET_TARGET_PROPERTIES(_static PROPERTIES OUTPUT_NAME "")
# SET_TARGET_PROPERTIES(_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
# add_library( SHARED ${LIBRARY_SRC})
# SET_TARGET_PROPERTIES( PROPERTIES OUTPUT_NAME "")
# SET_TARGET_PROPERTIES( PROPERTIES CLEAN_DIRECT_OUTPUT 1)

add_executable(register register.cpp)
set (
	pthread_mysql_jsoncpp # val name
	-pthread -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lz -lzstd -lssl -lcrypto -lresolv -lm -ljsoncpp
)
TARGET_LINK_LIBRARIES(register ${pthread_mysql_jsoncpp})

add_executable(search search.cpp)
TARGET_LINK_LIBRARIES(search ${pthread_mysql_jsoncpp})

# install()

