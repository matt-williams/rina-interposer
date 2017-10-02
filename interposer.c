#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <dlfcn.h>
 
int socket(int domain, int type, int protocol) {
  static int (*my_socket)(int, int, int) = NULL;
  printf("socket(%d, %d, %d)...\n", domain, type, protocol);
  if (my_socket == NULL) {
     my_socket = dlsym(RTLD_NEXT, "socket");
  }
  int fd = my_socket(domain, type, protocol);
  printf("...returns %d\n", fd);
  return fd;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  static int (*my_connect)(int, const struct sockaddr *, socklen_t) = NULL;
  printf("connect(%d, %p, %d)...\n", sockfd, addr, addrlen);
  char* dif = getenv("RINA_DIF");
  char* local_appl = getenv("RINA_LOCAL_APPL");
  char* remote_appl = getenv("RINA_REMOTE_APPL");
  int rc;
  if ((local_appl != NULL) && (remote_appl != NULL)) {
    printf("  RINA_DIF=%s, RINA_LOCAL_APPL=%s, RINA_REMOTE_APPL=%s => RINA interposer enabled!\n", dif, local_appl, remote_appl);
    printf("  rina_flow_alloc(\"%s\", \"%s\", \"%s\", NULL, 0)...\n", dif, local_appl, remote_appl);
    int rina_fd = rina_flow_alloc(dif,
                                  local_appl,
                                  remote_appl,
                                  NULL,
                                  0);
    printf("  ...returns %d\n", rina_fd);
    if (rina_fd >= 0) {
      printf("  RINA FD = %d - swapping for %d\n", rina_fd, sockfd);
      rc = (dup2(rina_fd, sockfd) > 0) ? 0 : -1;
      close(rina_fd);
    } else {
      perror("  rina_flow_alloc");
      rc = -1;
    }
  } else {
    if (my_connect == NULL) {
       my_connect = dlsym(RTLD_NEXT, "connect");
    }
    rc = my_connect(sockfd, addr, addrlen);
  }
  printf("...returns -1\n");
  return rc;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  static int (*my_bind)(int, const struct sockaddr *, socklen_t) = NULL;
  printf("bind(%d, %p, %d)...\n", sockfd, addr, addrlen);
  char* dif = getenv("RINA_DIF");
  char* local_appl = getenv("RINA_LOCAL_APPL");
  int rc;
  if ((dif != NULL) && (local_appl != NULL)) {
    printf("  RINA_DIF=%s, RINA_LOCAL_APPL=%s => RINA interposer enabled!\n", dif, local_appl);
    rc = 0;
  } else {
    if (my_bind == NULL) {
       my_bind = dlsym(RTLD_NEXT, "bind");
    }
    rc = my_bind(sockfd, addr, addrlen);
  }
  printf("...returns %d\n", rc);
  return rc;
}

int listen(int sockfd, int backlog) {
  static int (*my_listen)(int, int) = NULL;
  printf("listen(%d, %d)...\n", sockfd, backlog);
  char* dif = getenv("RINA_DIF");
  char* local_appl = getenv("RINA_LOCAL_APPL");
  int rc;
  if ((dif != NULL) && (local_appl != NULL)) {
    printf("  RINA_DIF=%s, RINA_LOCAL_APPL=%s => RINA interposer enabled!\n", dif, local_appl);
    printf("  rina_open()...\n");
    int rina_fd = rina_open();
    printf("  ...returns %d\n", rina_fd);
    if (rina_fd >= 0) {
      printf("  rina_register(%d, \"%s\", \"%s\")...\n", rina_fd, dif, local_appl);
      rc = rina_register(rina_fd, dif, local_appl, 0);
      printf("  ...returns %d\n", rc);
      if (rc >= 0) {
        printf("  RINA FD = %d - swapping for %d\n", rina_fd, sockfd);
        rc = (dup2(rina_fd, sockfd) > 0) ? 0 : -1;
        close(rina_fd);
      } else {
        perror("  rina_register");
      }
    } else {
      perror("  rina_open");
      rc = -1;
    }
  } else {
    if (my_listen == NULL) {
       my_listen = dlsym(RTLD_NEXT, "listen");
    }
    rc = my_listen(sockfd, backlog);
  }
  printf("...returns %d\n", rc);
  return rc;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
  static int (*my_accept)(int, struct sockaddr *, socklen_t *) = NULL;
  printf("accept(%d, %p, %p)...\n", sockfd, addr, addrlen);
  char* dif = getenv("RINA_DIF");
  char* local_appl = getenv("RINA_LOCAL_APPL");
  int fd;
  if ((dif != NULL) && (local_appl != NULL)) {
    printf("  RINA_DIF=%s, RINA_LOCAL_APPL=%s => RINA interposer enabled!\n", dif, local_appl);
    sockfd = 4;
    printf("  rina_flow_accept(%d, NULL, NULL, 0)...\n", sockfd);
    fd = rina_flow_accept(sockfd, NULL, NULL, 0);
    printf("  ...returns %d\n", fd);
    if (fd >= 0) {
      struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
      addr_in->sin_family = AF_INET;
      addr_in->sin_port = htons(1234);
      inet_aton("1.2.3.4", &addr_in->sin_addr.s_addr);
      *addrlen = sizeof(struct sockaddr_in);
    } else {
      perror("  rina_flow_accept");
    }
  } else {
    if (my_accept == NULL) {
      my_accept = dlsym(RTLD_NEXT, "accept");
    }
    int fd = my_accept(sockfd, addr, addrlen);
  }
  printf("...returns %d\n", fd);
  return fd;
}
