#ifndef CONN_H
#define CONN_H

int initServerSocket(char *name);
int connectToServer(char *name);
int readLine(int fd, char *str);
void sendComponentName(int clientFd, char *name);
void sendC(int clientFd, char *buffer);
struct Component connectToComponent(int centralFd);
void sendMsg(int clientFd, char *msg);
void sendOk(int clientFd);
void waitOk(int clientFd);

#endif
