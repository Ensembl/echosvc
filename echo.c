#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF 4096

void die(const char *str) {
  perror(str);
  exit(255);
}

int main() {
  int lfd,cfd,r,s,on=1;
  struct sockaddr_in addr;
  char buf[BUF];

  lfd = socket(AF_INET,SOCK_STREAM,0);
  if(lfd==-1) { die("Socket failed"); }
  if(setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))) {
    die("setsockopt");
  }
  memset(&addr,0,sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(7);
  if(bind(lfd,(struct sockaddr *)&addr,sizeof(struct sockaddr))) {
    die("Bind failed");
  }
  if(listen(lfd,16)) { die("Listen failed"); }
  while(1) {
    cfd = accept(lfd,0,0);
    if(cfd<0) { continue; }
    while(1) {
      r = read(cfd,buf,BUF);
      if(r<1) { break; }
      s = 0;
      while(r) {
        s = write(cfd,buf,r);
        if(s<0) { r=0; } else { r-=s; }
      }
      if(s==-1) { break; }
    }
    close(cfd);
  }
  return 0;
}
