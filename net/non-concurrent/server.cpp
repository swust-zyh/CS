/**
 *    author:  mikezheng
 *    created: 02-07-2022 17:17:51
**/

#include <csapp.h>

using namespace std;

#define SERVER_PORT 666

signed main(int argc,char *argv[]) {
	// 接收器 返回值为接收器编号
  int sock = socket(AF_INET, SOCK_STREAM, 0); 

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof server_addr);
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);
	
	// 将服务器地址信息贴到接收器上
	bind(sock, (struct sockaddr *)&server_addr, sizeof server_addr);

	// 监听接收器
	listen(sock, 128); // 128: 允许同时向服务器发起请求的数量

	cout << "runing..." << endl;

	while	(true) {
		char client_ip[64], buf[256];
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		// 客户端的连接号
		int client = accept(sock, (struct sockaddr *)&client_addr, &client_addr_len);
		cout << "client ip: " << inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof client_ip) << endl;
		cout << "port: " << ntohs(client_addr.sin_port) << endl;

		int len = read(client, buf, sizeof(buf)-1); buf[len] = '\0';
		cout << "recieve: " << buf << endl;

		len = write(client, buf, len);
		cout << "write finished. len: " << len << endl;
		close(client);
	}
  return 0;
}
