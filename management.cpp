#include "management.h"
#include <iostream>
/*
My implementation of the shared QT-framebuffer idea works by:
- Having all the clients communicate with the server via a UNIX socket, which governs
  shared memory usage.
- After connecting via the master socket, the client will need to:
  * Define what framebuffer it wants to connect to by ID
  * Define what format it will use for the shared memory.
  After it defines all that, it will receive a global shared-memory key, which will get
  it access to the shared memory region.
- In order to paint data on the screen, the client will write the data into the SHM, then
  send an update request via the unix socket. That will cause the server to read the SHM
  and force a repaint.
- Upon framebuffer detaching, the server will send the client a packet telling it to stop
  writing data to the SHM, and disconnect. The server will delete the shared memory and
  detach everything.
*/

void tryToMatchUp(qtfb::FBKey key){
    // Try to find the client in the clients' list
    if(qtfb::management::connections.find(key) == qtfb::management::connections.end()) {
        return; // Client not found
    }
    if(qtfb::management::framebuffers.find(key) == qtfb::management::framebuffers.end()) {
        return; // Framebuffer not found
    }
    // Both of them found! Assign one to another.
    qtfb::management::ClientConnection *connection = qtfb::management::connections[key];
    QPointer<FBController> controller = qtfb::management::framebuffers[key];
    if(connection->shm == NULL || controller.isNull()){
        CERR << "Invalid state: Cannot have an partially-associated connection in the connections map!" << std::endl;
        return;
    }
    controller->associateSHM(connection->image);
    CERR << "Associated connection <==> framebuffer " << key << std::endl;
}

#define SEND(message) send(connection->clientFD, &message, sizeof(message), 0)

void qtfb::management::registerController(FBKey key, QPointer<FBController> controller){
    if(qtfb::management::framebuffers.find(key) != qtfb::management::framebuffers.end()) {
        CERR << "Violation: Tried to attach to an already defined framebuffer. Please change the framebufferID!" << std::endl;
        return;
    }

    CERR << "Registered framebuffer controller ID: " << key << std::endl;
    framebuffers.emplace(key, controller);
    tryToMatchUp(key);
}

bool qtfb::management::isControllerAssociated(FBKey key) {
    auto position = qtfb::management::connections.find(key);
    return position != qtfb::management::connections.end();
}

void qtfb::management::unregisterController(FBKey key) {
    auto position = qtfb::management::framebuffers.find(key);
    if(position != qtfb::management::framebuffers.end()) {
        qtfb::management::framebuffers.erase(position);
    }
    CERR << "Unregistered framebuffer controller ID: " << key << std::endl;
}

static bool createSHM(qtfb::management::ClientConnection *connection, int shmType) {
    size_t shmSize;
    switch(shmType) {
        case FBFMT_RM2FB:
            shmSize = RM2_HEIGHT * RM2_WIDTH * 2;
            break;
        case FBFMT_RMPP_RGB888:
            shmSize = RMPP_HEIGHT * RMPP_WIDTH * 3;
            break;
        case FBFMT_RMPP_RGBA8888:
            shmSize = RMPP_HEIGHT * RMPP_WIDTH * 4;
            break;
        case FBFMT_RMPP_RGB565:
            shmSize = RMPP_HEIGHT * RMPP_WIDTH * 2;
            break;
        default:
            CERR << "Unknown SHM type" << shmType << std::endl;
            return false;
    }

    connection->shmSize = shmSize;
    connection->shmType = shmType;
    connection->shmKey = rand() & ~0x80000000;
    FORMAT_SHM(shmText, connection->shmKey);
    shm_unlink(shmText);
    connection->shmFD = shm_open(shmText, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(connection->shmFD == -1) {
        CERR << "Failed to shm_open()!" << std::endl;
        return false;
    }
    if (ftruncate(connection->shmFD, shmSize) == -1) {
        CERR << "Failed to truncate the file!" << std::endl;
        shm_unlink(shmText);
        return false;
    }
    connection->shm = (unsigned char *) mmap(NULL, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, connection->shmFD, 0);
    if(connection->shm == MAP_FAILED) {
        CERR << "Failed to mmap() the SHM for framebuffer!" << std::endl;
        shm_unlink(shmText);
        connection->shmFD = -1;
        connection->shm = NULL;
        connection->shmSize = 0;
        connection->shmType = -1;
        connection->shmKey = -1;
        return false;
    }
    CERR << "Defined SHM size" << shmSize << "at" << connection->shm << std::endl;
    // We have the SHM defined.
    QImage::Format format;
    int width, height;
    unsigned char *shm;
    switch(shmType){
        case FBFMT_RM2FB:
            format = QImage::Format::Format_RGB16;
            width = RM2_WIDTH;
            height = RM2_HEIGHT;
            shm = connection->shm;
            break;
        case FBFMT_RMPP_RGB888:
            format = QImage::Format::Format_RGB888;
            width = RMPP_WIDTH;
            height = RMPP_HEIGHT;
            shm = connection->shm;
            break;
        case FBFMT_RMPP_RGBA8888:
            format = QImage::Format::Format_RGBA8888;
            width = RMPP_WIDTH;
            height = RMPP_HEIGHT;
            shm = connection->shm;
            break;
        case FBFMT_RMPP_RGB565:
            format = QImage::Format::Format_RGB16;
            width = RMPP_WIDTH;
            height = RMPP_HEIGHT;
            shm = connection->shm;
            break;
    }
    connection->image = new QImage(shm, width, height, format);

    return true;
}

static int handleInitialize(qtfb::management::ClientConnection *connection, qtfb::ClientMessage *inbound) {
    if(qtfb::management::connections.find(inbound->init.framebufferKey) != qtfb::management::connections.end()) {
        CERR << "Violation: Tried to connect to an already used framebuffer." << std::endl;
        return RESP_ERR;
    }
    connection->fbKey = inbound->init.framebufferKey;
    if(!createSHM(connection, inbound->init.framebufferType)) {
        return RESP_ERR;
    }
    // Send the SHM key over to the client
    qtfb::ServerMessage outbound = {
        .type = MESSAGE_INITIALIZE,
        .init = {
            .shmKeyDefined = connection->shmKey,
            .shmSize = connection->shmSize,
        },
    };
    SEND(outbound);
    qtfb::management::connections[connection->fbKey] = connection;
    tryToMatchUp(connection->fbKey);
    return RESP_OK;
}

static int handleUpdateRegion(qtfb::management::ClientConnection *connection, qtfb::ClientMessage *inbound) {
    if(connection->fbKey == -1){
        CERR << "Cannot update region of an uninitialized connection!" << std::endl;
        return RESP_ERR;
    }
    if(qtfb::management::framebuffers.find(connection->fbKey) != qtfb::management::framebuffers.end()) {
        // Does this require translation?
        QPointer<FBController> controller = qtfb::management::framebuffers[connection->fbKey];
        if(controller.isNull()) {
            return RESP_OK;
        }
        int x, y, w, h;
        switch(inbound->update.type) {
            case UPDATE_ALL:
                CERR << "Updated all of framebuffer" << connection->fbKey << std::endl;
                QMetaObject::invokeMethod(controller, [controller]() {
                    controller->markedUpdate();
                }, Qt::QueuedConnection);
                break;
            case UPDATE_PARTIAL:
                CERR << "Updated region " << inbound->update.x << " " << inbound->update.y << " " << inbound->update.w << " " << inbound->update.h << " of framebuffer " << connection->fbKey << std::endl;
                x = inbound->update.x, y = inbound->update.y, w = inbound->update.w, h = inbound->update.h;
                QMetaObject::invokeMethod(controller, [controller, x, y, w, h]() {
                    controller->markedUpdate(QRect(
                        x,
                        y,
                        w,
                        h
                    ));
                }, Qt::QueuedConnection);
                break;
            default:
                CERR << "Unknown update method! " << inbound->update.type << " <-- " << inbound->update.x << " " << inbound->update.y << " " << inbound->update.w << " " << inbound->update.h << " of framebuffer " << connection->fbKey << std::endl;
                
        }
    } else {
        CERR << "Could not find the framebuffer to update." << std::endl;
    }

    return RESP_OK;
}

static void managementClientThread(int incomingFD){
    qtfb::management::ClientConnection connection;
    connection.clientFD = incomingFD;

    CERR << "Connection established from client. Sock FD is " << incomingFD << std::endl;
    qtfb::ClientMessage inboundMessage;
    for(;;){
        if(recv(incomingFD, &inboundMessage, sizeof(inboundMessage), 0) < 1) break;
        int status;
        switch(inboundMessage.type) {
            case MESSAGE_INITIALIZE:
                status = handleInitialize(&connection, &inboundMessage);
                break;
            case MESSAGE_UPDATE:
                status = handleUpdateRegion(&connection, &inboundMessage);
                break;
            default:
                CERR << "Client has tried to send a message with an invalid type: " << inboundMessage.type << std::endl;
                goto close;
        }
        if(status == RESP_ERR) {
            CERR << "Request-specific error. Disconnecting..." << std::endl;
            goto close;
        }
    }
    close:
    if(connection.fbKey != -1) {
        auto position = qtfb::management::connections.find(connection.fbKey);
        if(position != qtfb::management::connections.end()){
            qtfb::management::connections.erase(position);
        }
    }

    if(connection.fbKey != -1){
        if(qtfb::management::framebuffers.find(connection.fbKey) != qtfb::management::framebuffers.end()) {
            // The client that died was associated with a framebuffer!
            QPointer<FBController> controller = qtfb::management::framebuffers[connection.fbKey];
            if(!controller.isNull()) {
                // Disassociate
                while(controller->isMidPaint) {
                    usleep(1000);
                    if(controller.isNull()) {
                        // The controller had been removed as we were waiting for the repaint to stop
                        goto continueDeletion;
                    }
                }

                controller->associateSHM(NULL);
                CERR << "Disassociating framebuffer " << connection.fbKey << std::endl;
            }
        }
    }

    continueDeletion:

    delete connection.image;
    if(connection.shm != NULL) {
        munmap(connection.shm, connection.shmSize);
    }
    if(connection.translationShm != NULL){
        delete[] connection.translationShm;
    }
    if(connection.shmKey != -1) {
        FORMAT_SHM(shmName, connection.shmKey);
        shm_unlink(shmName);
    }
    CERR << "Closing client socket " << incomingFD << std::endl;
    close(incomingFD);
}

static void managementMainThread(){
    CERR << "In main management thread." << std::endl;
    CERR << "Creating socket..." << std::endl;
    int serverSocket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if(serverSocket == -1) {
        CERR << "Failed to initialize the socket!" << std::endl;
        return;
    }
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    unlink(SOCKET_PATH);
    if (bind(serverSocket, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        CERR << "Failed to bind the socket!" << std::endl;
        return;
    }
    if(listen(serverSocket, SOCKET_BACKLOG) == -1) {
        CERR << "Failed to listen on the socket! " << errno << std::endl;
        return;
    }
    for(;;) {
        CERR << "Awaiting incoming connections to" << SOCKET_PATH << std::endl;
        int incomingFD;
        if((incomingFD = accept(serverSocket, NULL, NULL)) == -1){
            CERR << "Failed to accept connection on the socket!" << std::endl;
            continue;
        }
        std::thread clientThread(managementClientThread, incomingFD);
        clientThread.detach();
    }
}

void qtfb::management::start(){
    srand(time(NULL));
    std::thread thread(managementMainThread);
    thread.detach();
}
