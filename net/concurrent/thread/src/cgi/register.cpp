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
		s += string("&");
		puts("Content-Type:text/html\n");
		cout << s << endl;
		
		shared_ptr<MysqlConn> conn = pool->getConn();
		cout << "get conn!" << endl;

		vector<int> bg, ed;
		for (int i = 0 ; i < s.size() ; i ++ ) {
			if (s[i] == '=') bg.emplace_back(i);
			else if (s[i] == '&') ed.emplace_back(i);
		}

		string sql = "insert into user values(";
		for (int i = 0 ; i < bg.size() ; i ++ ) {
			sql += (string("'") + s.substr(bg[i]+1, ed[i]-bg[i]-1) + string("'"));
			if (i != bg.size()-1) sql += string(",");
		}
		sql += ")";
		cout << sql << endl;

		// 创建关联函数
		// conn->update("");
		
		if (conn->update(sql)) {
			cout << "update success!" << endl;
			// update redis
			// https://www.cnblogs.com/bwbfight/p/11099482.html
			// 1. 构造sql hset
			// 2. conn->update("");
		} else {
			cout << "update sql error!" << endl;
		}
	} else {
		puts("Content-Type:text/html\n");
		puts("not found.");
	}
  return 0;
}
