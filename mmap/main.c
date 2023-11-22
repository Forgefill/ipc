#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <time.h>

#define BUFFER_SIZE (1024 * 1024 * 1024)  // 1 GB buffer size
const TCHAR *fileName = TEXT("mmap_file");

void createCharArray(char *buffer, size_t size) {
    for (size_t i = 0; i < size - 1; ++i) {
        buffer[i] = 'A';  // Fill with character 'A'
    }
    buffer[size - 1] = '\0';  // Null-terminate the string
}

int main() {
    HANDLE hMapFile, hFile;
    LPVOID pBuf;
    double elapsedTime, throughput;

    // Create or open file
    hFile = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        _tprintf(TEXT("Could not create file (%d).\n"), GetLastError());
        return 1;
    }

    // Set file size
    LARGE_INTEGER fileSize;
    fileSize.QuadPart = BUFFER_SIZE;
    if (!SetFilePointerEx(hFile, fileSize, NULL, FILE_BEGIN)) {
        _tprintf(TEXT("Error setting file pointer (%d).\n"), GetLastError());
        CloseHandle(hFile);
        return 1;
    }
    if (!SetEndOfFile(hFile)) {
        _tprintf(TEXT("Error setting end of file (%d).\n"), GetLastError());
        CloseHandle(hFile);
        return 1;
    }

    // Create a file mapping object
    hMapFile = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, BUFFER_SIZE, TEXT("SharedFileMapping"));
    if (hMapFile == NULL) {
        _tprintf(TEXT("Could not create file mapping object (%d).\n"), GetLastError());
        CloseHandle(hFile);
        return 1;
    }

    // Map a view of the file into the address space of the calling process
    pBuf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    if (pBuf == NULL) {
        _tprintf(TEXT("Could not map view of file (%d).\n"), GetLastError());
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return 1;
    }

    // Allocate buffer for writing
    char *writeBuffer = (char *)malloc(BUFFER_SIZE);
    if (writeBuffer == NULL) {
        _tprintf(TEXT("Failed to allocate memory for write buffer.\n"));
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return 1;
    }
    createCharArray(writeBuffer, BUFFER_SIZE);

    // Measure write time
    clock_t start = clock();

    CopyMemory(pBuf, writeBuffer, BUFFER_SIZE);

    clock_t end = clock();
    elapsedTime = (double)(end - start) / CLOCKS_PER_SEC;
    throughput = (BUFFER_SIZE / elapsedTime);  // bytes per second

    _tprintf(TEXT("Write Latency: %lf seconds\n"), elapsedTime);
    _tprintf(TEXT("Write Throughput: %lf MB/second\n"), throughput / (1024 * 1024));


    // Allocate buffer for reading
    char *readBuffer = (char *)malloc(BUFFER_SIZE);
    if (readBuffer == NULL) {
        _tprintf(TEXT("Failed to allocate memory for read buffer.\n"));
        free(writeBuffer);
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return 1;
    }

    // Measure read time
    start = clock();

    CopyMemory(readBuffer, pBuf, BUFFER_SIZE);

    end = clock();
    elapsedTime = (double)(end - start) / CLOCKS_PER_SEC;
    throughput = (BUFFER_SIZE / elapsedTime);  // bytes per second

    _tprintf(TEXT("Read Latency: %lf seconds\n"), elapsedTime);
    _tprintf(TEXT("Read Throughput: %lf MB/second\n"), throughput / (1024 * 1024));


    // Cleanup
    free(writeBuffer);
    free(readBuffer);
    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    CloseHandle(hFile);

    return 0;
}
