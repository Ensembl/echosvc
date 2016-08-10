#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF 4096
#define MAXCONN 32

static int use_syslog = 0;

static void report(const char *str) {
  if(use_syslog) {
    syslog(LOG_DAEMON|LOG_NOTICE,"%s\n",str);
  } else {
    fprintf(stderr,"%s\n",str);
  }
}

static void error(const char *str) {
  if(use_syslog) {
    syslog(LOG_DAEMON|LOG_ERR,"%s: %m\n",str);
  } else {
    fprintf(stderr,"%s: %s\n",str,strerror(errno));
  }
}

static void die(const char *str) {
  error(str);
  exit(255);
}

static void daemonize(void) {
  pid_t pid;

  pid = fork();
  if(pid==-1) { die("Cannot fork during daemonizing"); }
  if(pid>0) { exit(0); }
  use_syslog = 1;
  if(setsid()<0) { die("Cannot setsid"); }
  if(chdir("/")<0) { die("Cannot chdir to /"); }
  if(close(0) || close(1) || close(2)) { die("Cannot close fds"); }
  if(open("/dev/null",O_RDWR)<0) { die("Cannot open stdin to /dev/null"); }
  if(dup(0)<0 || dup(0)<0) { die("Cannot open stdout/err to /dev/null"); }
}

static void process(int cfd) {
  int r,s;
  char buf[BUF];

  if(cfd<0) { return; }
  while(1) {
    r = read(cfd,buf,BUF);
    if(r<1) { return; }
    while(r) {
      s = write(cfd,buf,r);
      r -= s;
      if(s<0) { return; }
    }
  }
}

sig_atomic_t kids;

void sigchld_handler(int sig) {
  int oerrno = errno;
  while(waitpid(-1,0,WNOHANG)>0) { kids--; }
  errno = oerrno;
}

static void handle(int lfd,int cfd) {
  pid_t pid;

  pid = fork();
  if(pid==-1) { error("couldn't fork"); return; }
  if(pid>0) { close(cfd); return; /* parent */ }
  close(lfd);
  process(cfd);
  close(cfd);
  exit(0);
}

int main() {
  int lfd,cfd,i,r,on=1;
  struct sockaddr_in addr;
  struct sigaction sa;

  daemonize();
  report("echosvc started");
  lfd = socket(AF_INET,SOCK_STREAM,0);
  if(lfd==-1) { die("Socket failed"); }
  /* Reusable */
  if(setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))) {
    die("setsockopt");
  }
  /* Zombie reaping */
  sa.sa_handler = &sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART|SA_NOCLDSTOP;
  if (sigaction(SIGCHLD,&sa,0)<0) { die("SIGCHLD handler"); }
  /* Bind */
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
    if(kids>MAXCONN) {
      error("Too many connections");
      close(cfd);
      continue;
    }
    kids++;
    handle(lfd,cfd);
  }
  return 0;
}
