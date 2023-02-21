#########################################################################
# File Name     : install.sh
# Author        : mikezheng
# mail          : csmikezheng@gmail.com
# Created Time  : 2022年07月27日 星期三 10时16分48秒
#########################################################################

#!/bin/bash
if [ ! -d "build" ]; then
	mkdir build
fi
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make
make install
cd ../src
if [ ! -d "build" ]; then
	mkdir build
fi
cd build
cmake ..
make
