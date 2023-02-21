# cmake version
cmake_minimum_required(VERSION 3.22.1)

# project name
project(http_thread_server)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../bin)

add_subdirectory(cgi)
# add_subdirectory(cgi ${PROJECT_SOURCE_DIR}/../html_docs/cgi)
# aux_source_directory(. DIR_SRCS)

# include_directories()
link_libraries (
  "/usr/lib/libPool.so"
  "/usr/lib/libMysqlConn.so"
  "/usr/lib/libConnectionPool.so"
	"/usr/lib/libtpool.so"
)

# set(LIBRARY_SRC ${DIR_SRCS})
# add_library(_static STATIC ${LIBRARY_SRC})
# SET_TARGET_PROPERTIES(_static PROPERTIES OUTPUT_NAME "")
# SET_TARGET_PROPERTIES(_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
# add_library( SHARED ${LIBRARY_SRC})
# SET_TARGET_PROPERTIES( PROPERTIES OUTPUT_NAME "")
# SET_TARGET_PROPERTIES( PROPERTIES CLEAN_DIRECT_OUTPUT 1)

add_executable(server server.cpp)
set (
  pthread_mysql_jsoncpp # val name
  -pthread -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lz -lzstd -lssl -lcrypto -lresolv -lm -ljsoncpp
)
TARGET_LINK_LIBRARIES(server ${pthread_mysql_jsoncpp})
# TARGET_LINK_LIBRARIES()

# install()
