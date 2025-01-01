#include <iostream>
#include "../../clients/cpp/qtfb-client.h"

using namespace std;
unsigned char *readFile(const char *filename, size_t *size) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Failed to seek end of file");
        fclose(file);
        return NULL;
    }

    long fileSize = ftell(file);
    if (fileSize == -1) {
        perror("Failed to determine file size");
        fclose(file);
        return NULL;
    }

    rewind(file);

    unsigned char *buffer = (unsigned char *)malloc(fileSize);
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    size_t readSize = fread(buffer, 1, fileSize, file);
    if (readSize != fileSize) {
        perror("Failed to read file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);

    if (size) {
        *size = fileSize;
    }

    return buffer;
}

int main(){
    cout << "Creating socket..." << endl;
    qtfb::ClientConnection conn(245209899, FBFMT_RMPP_RGB888);

    int startOffset = 0;
    size_t bfrSize;
    unsigned char *bfr = readFile("a.raw", &bfrSize);
    memcpy(conn.shm + startOffset, bfr, bfrSize);

    conn.sendCompleteUpdate();

    sleep(10);
    return 0;
}
