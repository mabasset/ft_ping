#include "ft_ping.h"

t_ping g_ping = {.sockfd = -1, .payload_size = PING_DEFAULT_PAYLOAD_SIZE};

static void cleanup(void) {
  printf("exit\n");
  if (g_ping.sockfd >= 0)
    close(g_ping.sockfd);
}

static void handle_sigint(int sig) {
  (void)sig;
  g_ping.sigint = 1;
}

static void create_socket() {
  g_ping.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (g_ping.sockfd == -1) {
    if (errno == 1)
      fprintf(stderr, "ping: Lacking privilege for icmp socket.\n");
    else
      perror("ping: socket");
    exit(1);
  }
}

static void drop_sudo() {
  if (setuid(getuid()) != 0) {
    perror("ping: setuid");
    exit(1);
  }
}

static void set_siganction() {
  struct sigaction sa = {0};
  sa.sa_handler = handle_sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
}

int main(int argc, char* argv[]) {
  parse_arguments(argc, argv);
  create_socket();
  atexit(cleanup);
  drop_sudo();
  set_siganction();
  run_ping();

  return 0;
}