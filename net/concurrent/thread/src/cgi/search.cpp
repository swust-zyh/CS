/**
 *    author:  mikezheng
 *    created: 23-07-2022 13:09:52
**/

#include <csapp.h>
#include "../pool.h"

using namespace std;

string s;

signed main(int argc, char *argv[]) {
  char* in = getenv("CONTENT_LENGTH");
	int len = atoi(in);
	if (len) {
		for (int i = 0 ; i < len ; i ++ ) {
			char c; cin >> c; s += c;
		}
		puts("Content-Type:text/html\n");
		s = s.substr(3);
		cout << s << endl;

		// RedisConnectionPool* redisConn = redisPool->getRedisConn();
		// redis 指令
		
		shared_ptr<MysqlConn> conn = pool->getConn();
		cout << "get conn!" << endl;
		
		string sql = "select * from user where id='"+s+"'";
		cout << sql << endl;
		
		if (conn->query(sql)) {
			while (conn->next()) {
				for (int i = 2 ; i < conn->getColNum() ; i ++ ) {
					cout << conn->name(i) << "=" << conn->value(i) << endl;
				}
			}
		} else {
			cout << "query sql error!" << endl;
		}
	} else {
		puts("Content-Type:text/html\n");
		puts("not found.");
	}
  return 0;
}
