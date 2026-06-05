#pragma once

//----------------------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------------------

#include <arpa/inet.h> //htons, htonl
#include <cstring>
#include <dirent.h> // opendir, readdir, closedir
#include <errno.h> // strerror
#include <fcntl.h> // fcntl, open
#include <fstream>
#include <iostream>
#include <map>
#include <netdb.h> // getaddrinfo, freeaddrinfo, getprotobyname
#include <optional>
#include <poll.h> //poll
#include <regex>
#include <set>
#include <signal.h> //kill, sginal
#include <sstream>
#include <string>
#include <sys/epoll.h> // epoll
#include <sys/select.h> // select
#include <sys/socket.h> //socketpair, socket, getsockopt, getsockname
#include <sys/stat.h> // stat
#include <sys/time.h>
#include <sys/types.h> //kqueue, accept, listen, send, recv, connect
#include <sys/wait.h> // waipid
#include <unistd.h> //execve, dup, dup2, pipe, close, read, write, access
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sys/stat.h>
#include <signal.h>


//----------------------------------------------------------------------------------------
// Macro Definitions
//----------------------------------------------------------------------------------------

#define MAXLINE 4096
#define BACKLOG	10


//----------------------------------------------------------------------------------------
// Constant Expressions
//----------------------------------------------------------------------------------------

constexpr int success = 0;
constexpr int failure = 1;

extern sig_atomic_t	g_signal_numb;
