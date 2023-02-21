/**
 *    author:  mikezheng
 *    created: 07-08-2022 17:43:33
**/

#include "pool.h"

// 只能写绝对路径
ConnectionPool* pool = ConnectionPool::getPool("/usr/local/etc/dbconf.json");
