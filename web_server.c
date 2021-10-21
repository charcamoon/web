#include "common.h"

static int configuration(int *port, char *path) 
{
	int i;
	FILE *fp;
	char *p;
	char buf[50];

	fp = fopen("./config.ini", "r");

	if (fp == NULL) {
		perror("faile to open config.ini");
		return -1;
	}

	while (fgets(buf, 50, fp) != NULL) {
		if (buf[strlen(buf)-1] != '\n') {
			printf("error in config.ini\n");
			return -1;
		} else
			buf[strlen(buf) - 1] = '\0';

		if (strstr(buf, "port") == buf) {
			if ((p = strchr(buf, ";")) == NULL) {
				printf("config.ini expect ';'\n");
				return -1;
			}

			*port = atoi(p + 2);
			if(*port <= 0) {
				printf("error port\n");
				return -1;
			}
		} else if (strstr(buf, "root-path") == buf) {
			if (p = strchr(buf, ';') == NULL) {
				printf("config.ini expect ';'\n");
				return -1;
			}
			p++;

			strcpy(path, p);
		} else {
			printf("error in config.ini\n");
			return -1;
		}
	}
	return 0;
}

int init(struct sockaddr_in *sin, int *lfd, int *port, char *path)
{
	int tfd;

	configuration(port, path);

	bzero(sin, sizeof(struct sockaddr_in));
	sin->fin_family = AF_INET;
	sin->sin_addr.s_addr = INADDR_ANY;
	sin->sin_port = htons(PORT);

	if ((tfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("fail to creat socket");
		return -1;
	}

	if ((bind(tfd, (struct sockaddr *)in, sizeof(struct sockaddr_in)) == -1) {
		perror("fail to bind");
		return -1;
	}

	if ((listen(tfd, 20)) == -1) {
		perror("fail to listen");
		return -1;
	}

	*lfd = tfd;

	return 0;
}

int get_path(int cfd, char *path) 
{
	char buf[MAX_LINE];

	if (my_read(cfd, buf, MAX_LINE) == -1)
		return -1;

	if (strstr(buf, "GET") != buf) {
		DEBUG_PRINT("wrong request\n");
		return -1;
	}

	if ((buf[4] == '/')&&(buf[5] == ' '))
		strcat(path, "/index.html");
	else {
		strtok(&buf[4], " ");
		strcat(path, &buf[4]);
	}
	return 0;
}

int error_page(int sock_fd)
{
	char err_str[1024];

	if (my_write(sock_fd, err_str, strlen(err_str)) == -1)
		return -1;

	if (my_write(sock_fd, "Content-Type: text/html\r\n\r\n", strlen("Content-Type: text/html\r\n\r\n")) == -1)
		return -1;

	if (my_write(sock_fd, "<html><body> the file dose not exist </body> </html>",
				strlen("<html><body> the file dose not exist </body></html>")) == -1)
		return -1;

	return 0;
}

int write_page(int cfd, char *path)
{
	int n;
	char buf[MAX_LINE];

	if (my_write(cfd, "HRRP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n")) == -1)
		return -1;

	if (my_write(cfd, "Content-Type:", strlen("Content-Type: ")) == -1)
		return -1;

	n =strlen(path);

	if (strcasecmp(&path[n-3], "jpg") == 0 || (strcasecmp(&path[n-4], "jpeg") == 0))
		if (my_write(cfd, "image/jpeg", strlen("image/jpeg")) == -1)
			return -1;
		else if (strcasecmp(&path[n-3], "gif") == 0)
			if (my_write(cfd, "image/gif", strlen("image/gif")) == -1)
				return -1;
		else if(strcasecmp(&path[n-3], "png") == 0)
			if (my_write(cfd, "image/png", strlen("image/png")) == -1)
				return -1;
		else 
			if(my_write(cfd, "text/html", strlen("text/html")) == -1)
				return -1;
	if (my_write(cfd, "\r\n\r\n", 4) == -1)
		return -1;

	while(n = read(fd, buf, MAX_LINE) > 0)
		if (my_write(cfd, buf, n) == -1)
			return -1;

	return 0;
}
