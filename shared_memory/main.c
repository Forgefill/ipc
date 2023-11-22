#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define SHARED_MEMORY_SIZE (1024 * 1024 * 1024)  // 1 GB
#define BYTES_TO_MB 1048576  // Number of bytes in a megabyte

int main() {
    HANDLE hMapFile;
    LPVOID pBuf;
    char *data, *readBuffer;
    clock_t startWrite, endWrite, startRead, endRead;
    double writeTime, readTime, writeThroughput, readThroughput;

    // Allocate memory for 1 GB data
    data = (char *)malloc(SHARED_MEMORY_SIZE);
    if (data == NULL) {
        printf("Memory allocation for write buffer failed.\n");
        return 1;
    }

    // Allocate memory for read buffer
    readBuffer = (char *)malloc(SHARED_MEMORY_SIZE);
    if (readBuffer == NULL) {
        printf("Memory allocation for read buffer failed.\n");
        free(data);
        return 1;
    }

    // Initialize data (optional, depending on use case)
    memset(data, 0, SHARED_MEMORY_SIZE);

    // Step 1: Create Shared Memory
    hMapFile = CreateFileMapping(
            INVALID_HANDLE_VALUE,    // Use paging file
            NULL,                    // Default security
            PAGE_READWRITE,          // Read/write access
            0,                       // Maximum object size (high-order DWORD)
            SHARED_MEMORY_SIZE,      // Maximum object size (low-order DWORD)
            "MySharedMemory");       // Name of mapping object

    if (hMapFile == NULL) {
        printf("Could not create file mapping object (%d).\n", GetLastError());
        free(data);
        free(readBuffer);
        return 1;
    }

    pBuf = MapViewOfFile(hMapFile,   // Handle to mapping object
                         FILE_MAP_ALL_ACCESS, // Read/write permission
                         0,
                         0,
                         SHARED_MEMORY_SIZE);

    if (pBuf == NULL) {
        printf("Could not map view of file (%d).\n", GetLastError());
        CloseHandle(hMapFile);
        free(data);
        free(readBuffer);
        return 1;
    }

    // Step 2: Write to Shared Memory
    startWrite = clock();
    CopyMemory(pBuf, data, SHARED_MEMORY_SIZE);
    endWrite = clock();

    // Step 3: Measure Write Latency and Throughput
    writeTime = ((double) (endWrite - startWrite)) / CLOCKS_PER_SEC;
    writeThroughput = (SHARED_MEMORY_SIZE / BYTES_TO_MB) / writeTime;
    printf("Write Time: %f seconds,\nWrite Throughput: %f MB/sec\n", writeTime, writeThroughput);

    // Step 4: Read from Shared Memory
    startRead = clock();
    CopyMemory(readBuffer, pBuf, SHARED_MEMORY_SIZE);
    endRead = clock();

    // Step 5: Measure Read Throughput
    readTime = ((double) (endRead - startRead)) / CLOCKS_PER_SEC;
    readThroughput = (SHARED_MEMORY_SIZE / BYTES_TO_MB) / readTime;
    printf("Read Time: %f seconds,\nRead Throughput: %f MB/sec\n", readTime, readThroughput);

    // Step 6: Cleanup
    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    free(data);
    free(readBuffer);

    return 0;
}
