#pragma once
#include "common.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

namespace qtfb{
    class ClientConnection {
    public:
        ClientConnection(FBKey framebufferID, uint8_t shmType);
        ~ClientConnection();
        void sendCompleteUpdate();
        void sendPartialUpdate(int x, int y, int w, int h);
        unsigned char *shm;
    private:
        int fd;
        size_t shmSize;
        void _send(const struct ClientMessage &message);
    };
}