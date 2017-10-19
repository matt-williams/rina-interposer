#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <dlfcn.h>
#include <rina/api.h>
 
int socket(int domain, int type, int protocol) {
  static int (*my_socket)(int, int, int) = NULL;
  char* verbose = getenv("RINA_VERBOSE");
  if (verbose) printf("socket(%d, %d, %d)...\n", domain, type, protocol);
  if (my_socket == NULL) {
    my_socket = dlsym(RTLD_NEXT, "socket");
  }
  int fd = my_socket(domain, type, protocol);
  if (verbose) printf("...returns %d\n", fd);
  return fd;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  static int (*my_connect)(int, const struct sockaddr *, socklen_t) = NULL;
  char* verbose = getenv("RINA_VERBOSE");
  if (verbose) printf("connect(%d, %p, %d)...\n", sockfd, addr, addrlen);
  char* dif = getenv("RINA_DIF");
  char* local_appl = getenv("RINA_LOCAL_APPL");
  char* remote_appl = getenv("RINA_REMOTE_APPL");
  int rc;
  struct rina_flow_spec flow_spec;
  if ((local_appl != NULL) && (remote_appl != NULL)) {
    if (verbose) printf("  RINA_DIF=%s, RINA_LOCAL_APPL=%s, RINA_REMOTE_APPL=%s => RINA interposer enabled!\n", dif, local_appl, remote_appl);
    if (verbose) printf("  rina_flow_alloc(\"%s\", \"%s\", \"%s\", NULL, 0)...\n", dif, local_appl, remote_appl);
    flow_spec.in_order_delivery = 1;
    flow_spec.msg_boundaries = 0;
    flow_spec.max_loss = 0;
    int rina_fd = rina_flow_alloc(dif,
                                  local_appl,
                                  remote_appl,
                                  &flow_spec,
                                  0);
    if (verbose) printf("  ...returns %d\n", rina_fd);
    if (rina_fd >= 0) {
      if (verbose) printf("  RINA FD = %d - swapping for %d\n", rina_fd, sockfd);
      rc = (dup2(rina_fd, sockfd) > 0) ? 0 : -1;
      close(rina_fd);
    } else {
      if (verbose) perror("  rina_flow_alloc");
      rc = -1;
    }
  } else {
    if (my_connect == NULL) {
      my_connect = dlsym(RTLD_NEXT, "connect");
    }
    rc = my_connect(sockfd, addr, addrlen);
  }
  if (verbose) printf("...returns -1\n");
  return rc;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  static int (*my_bind)(int, const struct sockaddr *, socklen_t) = NULL;
  char* verbose = getenv("RINA_VERBOSE");
  if (verbose) printf("bind(%d, %p, %d)...\n", sockfd, addr, addrlen);
  char* dif = getenv("RINA_DIF");
  char* local_appl = getenv("RINA_LOCAL_APPL");
  int rc;
  if ((dif != NULL) && (local_appl != NULL)) {
    if (verbose) printf("  RINA_DIF=%s, RINA_LOCAL_APPL=%s => RINA interposer enabled!\n", dif, local_appl);
    rc = 0;
  } else {
    if (my_bind == NULL) {
      my_bind = dlsym(RTLD_NEXT, "bind");
    }
    rc = my_bind(sockfd, addr, addrlen);
  }
  if (verbose) printf("...returns %d\n", rc);
  return rc;
}

int listen(int sockfd, int backlog) {
  static int (*my_listen)(int, int) = NULL;
  char* verbose = getenv("RINA_VERBOSE");
  if (verbose) printf("listen(%d, %d)...\n", sockfd, backlog);
  char* dif = getenv("RINA_DIF");
  char* local_appl = getenv("RINA_LOCAL_APPL");
  int rc;
  if ((dif != NULL) && (local_appl != NULL)) {
    if (verbose) printf("  RINA_DIF=%s, RINA_LOCAL_APPL=%s => RINA interposer enabled!\n", dif, local_appl);
    if (verbose) printf("  rina_open()...\n");
    int rina_fd = rina_open();
    if (verbose) printf("  ...returns %d\n", rina_fd);
    if (rina_fd >= 0) {
      if (verbose) printf("  rina_register(%d, \"%s\", \"%s\")...\n", rina_fd, dif, local_appl);
      rc = rina_register(rina_fd, dif, local_appl, 0);
      if (verbose) printf("  ...returns %d\n", rc);
      if (rc >= 0) {
        if (verbose) printf("  RINA FD = %d - swapping for %d\n", rina_fd, sockfd);
        rc = (dup2(rina_fd, sockfd) > 0) ? 0 : -1;
        close(rina_fd);
      } else {
        if (verbose) perror("  rina_register");
      }
    } else {
      if (verbose) perror("  rina_open");
      rc = -1;
    }
  } else {
    if (my_listen == NULL) {
      my_listen = dlsym(RTLD_NEXT, "listen");
    }
    rc = my_listen(sockfd, backlog);
  }
  if (verbose) printf("...returns %d\n", rc);
  return rc;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
  static int (*my_accept)(int, struct sockaddr *, socklen_t *) = NULL;
  char* verbose = getenv("RINA_VERBOSE");
  if (verbose) printf("accept(%d, %p, %p)...\n", sockfd, addr, addrlen);
  char* dif = getenv("RINA_DIF");
  char* local_appl = getenv("RINA_LOCAL_APPL");
  int fd;
  if ((dif != NULL) && (local_appl != NULL)) {
    if (verbose) printf("  RINA_DIF=%s, RINA_LOCAL_APPL=%s => RINA interposer enabled!\n", dif, local_appl);
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(sockfd, &read_fds);
    if (verbose) printf("  select(%d, %p, NULL, NULL, NULL)...\n", sockfd + 1, &read_fds);
    int rc = select(sockfd + 1, &read_fds, NULL, NULL, NULL);
    if (verbose) printf("  returns %d\n", rc);
    if (verbose) printf("  rina_flow_accept(%d, NULL, NULL, 0)...\n", sockfd);
    fd = rina_flow_accept(sockfd, NULL, NULL, 0);
    if (verbose) printf("  ...returns %d\n", fd);
    if (fd >= 0) {
      struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
      addr_in->sin_family = AF_INET;
      addr_in->sin_port = htons(1234);
      inet_aton("127.0.0.1", &addr_in->sin_addr.s_addr);
      *addrlen = sizeof(struct sockaddr_in);
    } else {
      if (verbose) perror("  rina_flow_accept");
    }
  } else {
    if (my_accept == NULL) {
      my_accept = dlsym(RTLD_NEXT, "accept");
    }
    int fd = my_accept(sockfd, addr, addrlen);
  }
  if (verbose) printf("...returns %d\n", fd);
  return fd;
}

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
  static int (*my_getaddrinfo)(const char *, const char *, const struct addrinfo *, struct addrinfo **) = NULL;
  char* verbose = getenv("RINA_VERBOSE");
  if (verbose) printf("getaddrinfo(%p, %p, %p, %p)...\n", node, service, hints, res);
  if (my_getaddrinfo == NULL) {
    my_getaddrinfo = dlsym(RTLD_NEXT, "getaddrinfo");
  }
  int rc = my_getaddrinfo(node, service, hints, res);
  if (verbose) printf("...returns %d\n", rc);
  return rc;
}
