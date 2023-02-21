/**
 *    author:  mikezheng
 *    created: 04-07-2022 11:03:43
**/

#include <csapp.h>

using namespace std;

#define SERVER_PORT 666
#define SERVER_IP "192.168.209.1"

signed main(int argc, char *argv[]) {
  if (argc != 2) {
		fputs("", stderr);
		exit(1);
	}
	char* message = argv[1];
	cout << message << endl;

	// 传输管道id
	int sockid = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
	server_addr.sin_port = htons(SERVER_PORT);

	connect(sockid, (struct sockaddr*)&server_addr, sizeof server_addr);

	write(sockid, message, strlen(message));

	char buf[64];
	int len = read(sockid, buf, sizeof(buf)-1);
	if (len > 0) {
		buf[len] = '\0';
		cout << buf << endl;
	} else {
		perror("ERROR!"); // https://www.runoob.com/cprogramming/c-function-perror.html
	}
	cout << "finished." << endl;
	close(sockid);

  return 0;
}
