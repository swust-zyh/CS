#########################################################################
# File Name     : cgi.sh
# Author        : mikezheng
# mail          : csmikezheng@gmail.com
# Created Time  : 2022年07月26日 星期二 08时51分24秒
#########################################################################

#!/bin/bash

g++ register.cpp ../pool.cpp /usr/include/ConnectionPool.cpp /usr/include/MysqlConn.cpp -pthread -std=c++14 $(mysql_config --cflags) -ljsoncpp $(mysql_config --libs) -o register

cp -r register ../../html_docs/cgi

g++ search.cpp ../pool.cpp /usr/include/ConnectionPool.cpp /usr/include/MysqlConn.cpp -pthread -std=c++14 $(mysql_config --cflags) -ljsoncpp $(mysql_config --libs) -o search

cp -r search ../../html_docs/cgi
