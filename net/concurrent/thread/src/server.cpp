/**
 *    author:  mikezheng
 *    created: 02-07-2022 17:17:51
**/

#include <csapp.h>
#include "pool.h"

using namespace std;

#define STDIN 0
#define STDOUT 1
#define STDERR 2

int get_line(int client_id, char* buf, int size) {
	int cnt = 0, len = 0; char cur = '\0';
	while	(cnt < size-1 && cur != '\n') {
		len = read(client_id, &cur, 1);
		if (len == 1) {
			if (cur == '\r') continue;
			else if (cur == '\n') break;
			buf[cnt] = cur; cnt ++ ;
		} else if (len == -1) {
			perror("read faild."); cnt = -1; break;
		} else {
			fprintf(stderr, "client close.\n"); cnt = -1; break;
		}
	}
	if (cnt >= 0) buf[cnt] = '\0';
	return cnt;
}

void internal_error(int client_id) {

}

void not_found(int client_id) {

}

void unimplemented(int client_id) {
	
}

void bad_request(int client_id) {

}

int header(int client_id, FILE* resource) {
	struct stat st; int fileId = 0;
	char tmp[1024], buf[1024]={0};
	strcpy(buf, "HTTP/1.0 200 OK\r\n");
	strcat(buf, "Server: MikeZheng Server\r\n");
	strcat(buf, "Content-Type: text/html\r\n");
	strcat(buf, "Connection: Close\r\n\r\n");

	if(!~send(client_id, buf, sizeof buf, 0)) {
		cerr << "send error!" << endl; return -1;
	}
	return 0;
}

void body(int client_id, FILE* resource) {
	char buf[1024];
	while (!feof(resource)) {
		fgets(buf, sizeof buf, resource);
		int len = write(client_id, buf, strlen(buf));
		if (len < 0) {
			cerr << "send error!" << endl;
			break;
		}
		cout << buf << endl;
	}
}

inline void req_head(int client_id) {
	int len = 0; char buf[1024];
	do {
		len = get_line(client_id, buf, sizeof buf);
		// cout << buf << endl;
	} while (len);
}

void do_http_response(int client_id, const char* path) {
	req_head(client_id);
	FILE* resource = NULL;
	resource = fopen(path, "r");
	if (resource == NULL) {
		not_found(client_id);
		return ;
	}

	int t = header(client_id, resource);
	if (!t) body(client_id, resource);
	fclose(resource);
}

void cant_exe(int client_id) {
	
}

void exe_cgi(int client_id, const char* path, const char* method, const char* query) {
	pid_t pid; int len = 1, content_len = -1; char buf[1024], c; 
	int cgi_output[2], cgi_input[2], status; // 单向传输，匿名管道
	if (!strncasecmp(method, "POST", 4)) {
		do { // 读取请求头部
			len = get_line(client_id, buf, sizeof buf);
			cout << buf << endl; buf[15] = '\0';
			if (!strncasecmp(buf, "Content-Length:", 15)) {
				content_len = atoi(&(buf[16]));
				cout << content_len << endl;
			}
		} while (len);
		if (!~content_len) {
			bad_request(client_id); return ;
		}
	} else if (!strncasecmp(method, "GET", 4)) {
		req_head(client_id);
	} else {
		// ...
	}

	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	send(client_id, buf, sizeof buf, 0);

	if (pipe(cgi_output) < 0 || pipe(cgi_input) < 0 || (pid=fork()) < 0) {
		cant_exe(client_id); return ;
	}

	// 子进程pid=0
	if (!pid) {
		char meth_env[255], len_env[255], query_env[255];

		// 重定向之后输出不会在终端显示 debug notice !
		dup2(cgi_input[0], 0); dup2(cgi_output[1], 1);

	  close(cgi_output[0]); close(cgi_input[1]);		

	  sprintf(meth_env, "REQUEST_METHOD=%s", method);
	  putenv(meth_env);

	  if (!strncasecmp(method, "POST", 4)) {
		  sprintf(len_env, "CONTENT_LENGTH=%d", content_len);
		  putenv(len_env); execl(path, path, NULL);
		} else if (!strncasecmp(method, "GET", 3)) {
		  sprintf(query_env, "QUERY_STRING=%s", query);
			putenv(query_env);
			execl("../html_docs/cgi/get.cgi", "../html_docs/cgi/get.cgi", NULL);
		} else {
			// ... 
		}
		exit(0);
	} else { // 父进程
		close(cgi_output[1]); close(cgi_input[0]);

		if (!strncasecmp(method, "POST", 4)) {
			for (int i = 0; i < content_len; i++) {
				recv(client_id, &c, 1, 0); write(cgi_input[1], &c, 1);
			}
		}

		//读取cgi脚本返回数据
		while (read(cgi_output[0], &c, 1)) { //发送给浏览器
			send(client_id, &c, 1, 0);
		}

		//运行结束关闭
		close(cgi_output[0]); close(cgi_input[1]);
		waitpid(pid, &status, 0);	
	}
}

void* do_http_request(void* clientpt) {
	int len = 0, cgi = 0; const char* query;
	struct stat fst; char buf[256], method[64];
	// char path[256], url[256];
	string path, url; int client_id = *(int*)clientpt;

	// 按行读取http请求
	len = get_line(client_id, buf, sizeof buf);
	// cout << len << endl;

	int i = 0, j = 0;
	if (len > 0) {
		// 请求方式
		while	(!isspace(buf[j]) && (i < sizeof(method)-1)) {
			method[i++] = buf[j++];
		}
		method[i] = '\0';
		cout << "request method: " << method << endl;

		// url
  	if (!strncasecmp(method, "GET", i) || !strncasecmp(method, "POST", i)) {
			if(!strncasecmp(method, "POST", i)) cgi = 1;
	  	while (isspace(buf[j++]));
  		while (!isspace(buf[j])) {
	  		url += buf[j++];
	  	}
			cout << "url: " << url << endl;
   	} else {
			cerr << method << " Not Implemented!" << endl;
			unimplemented(client_id);
		}
		
		// char *pos = strchr(url, '?');
		// if (pos) {*pos='\1';}
		if (url.find("?") != url.npos) {
			cgi = 1; query = url.c_str();
			query = query + url.find("?") + 1;
			url = url.substr(0, url.find("?"));
		}
		if (url[url.size()-1] == '/') url = url.substr(0, url.size()-1);

		path = "/usr/local/share/html_docs/" + url;
		const char* pathpt = path.c_str();
		// sprintf(path, "./html_docs/%s", url);
		cout << pathpt << endl;
		
		if (!~stat(pathpt, &fst)) {
			req_head(client_id);
			not_found(client_id);
		} else {
			if (S_ISDIR(fst.st_mode)) {
				path += "/index.html";
				pathpt = path.c_str();
				cout << pathpt << endl;
			}

			//文件可执行
			if ((fst.st_mode & S_IXUSR) ||
				(fst.st_mode & S_IXGRP) ||
				(fst.st_mode & S_IXOTH)) {
				//S_IXUSR:文件所有者具可执行权限
				//S_IXGRP:用户组具可执行权限
				//S_IXOTH:其他用户具可读取权限
					cgi = 1;
					cout << "exe" << endl;
			}

			cout << "cgi: " << cgi << endl;
			cout << "query: " << query << endl;

			if (!cgi) do_http_response(client_id, pathpt);
			else exe_cgi(client_id, pathpt, method, query);
		}
	} else {
		// http 请求格式错误
		bad_request(client_id);
	}

	close(client_id); 
	if (clientpt) delete(clientpt);
	return NULL;
}

// #define u_short unsigned short
bool is_spare_port(u_short port) {
	char pcPort[6], buf[256]; sprintf(pcPort, "%hu", port);
	const char* cmd = "netstat -ano";
	// cout << cmd << endl;
	FILE* pipe = popen(cmd, "r");
	if (!pipe) throw runtime_error("popen() failed!");
	try {
		while (!feof(pipe)) {
			if (fgets(buf, 128, pipe) != NULL) {
				string t = string(buf);
				// cout << t << endl;
				if (t.find(":"+string(pcPort)) != string::npos && t.find("LISTEN") != string::npos) {
					return false;
					// cout << t << endl;
				}
			}
		}
	} catch (...) {
		pclose(pipe);
		throw;
	}
	pclose(pipe);
	return true;
}

signed main(int argc,char *argv[]) {
	u_short port = 666;
	// 接收器 返回值为接收器编号
  int sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof server_addr);

	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);
	socklen_t addr_len = sizeof(server_addr);

	if (!is_spare_port(port)) server_addr.sin_port = 0;

	// 将服务器地址信息贴到接收器上
	if (bind(sock, (struct sockaddr *)&server_addr, sizeof server_addr) < 0) cerr << "bind error." << endl;

	// 随机分配地址->快速地址再分配
	if (!~getsockname(sock, (struct sockaddr*)&server_addr, &addr_len)) cerr << "port error." << endl;
	cout << ntohs(server_addr.sin_port) << endl;

	// 监听接收器
	if (listen(sock, 128)) cerr << "listen error." << endl; // 128: 允许同时向服务器发起请求的数量

	cout << "running..." << endl;
	
	void* tpool = tpool_init("/usr/local/etc/tpoolconf.json");
	if (tpool == NULL) {
		cerr << "create tpool error!" << endl; return 0;
	}

	while	(true) {
		char client_ip[64], buf[256];
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		// 客户端的连接号
		int client = accept(sock, (struct sockaddr *)&client_addr, &client_addr_len);
		cout << "client ip: " << inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof client_ip) << endl;
		cout << "port: " << ntohs(client_addr.sin_port) << endl;

		if (!~client) continue;
		
		// 非并发
		// do_http_request(client);
		// close(client);
		
		/*
		// 非线程池
		pthread_t id; 
		int* clientpt = new int(client); // 深拷贝
		if (pthread_create(&id, NULL, do_http_request, clientpt)) continue;
		*/
		
		// 线程池
		int* clientpt = new int(client);
		if (tpool_add_work(tpool, do_http_request, clientpt) < 0) {
			tpool_destroy(tpool, 0);
			cerr << "add work error!" << endl; return 1;
		}
	}
	tpool_destroy(tpool, 1);
	close(sock);
  return 0;
}

