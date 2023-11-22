#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FILE_SIZE (1024 * 1024 * 1024)  // 1 GB
#define FILE_NAME "testfile.bin"        // File name

int main() {
    FILE *file;
    char *writeData, *readData;
    clock_t start, end;
    double writeTime, readTime, writeThroughput, readThroughput;

    // Allocate memory for 1 GB write data
    writeData = (char *)malloc(FILE_SIZE);
    if (writeData == NULL) {
        printf("Memory allocation for write data failed.\n");
        return 1;
    }

    // Allocate memory for 1 GB read data
    readData = (char *)malloc(FILE_SIZE);
    if (readData == NULL) {
        printf("Memory allocation for read data failed.\n");
        free(writeData);
        return 1;
    }

    // Initialize write data (optional)
    memset(writeData, 0, FILE_SIZE);

    // Step 1: Write Data to File
    file = fopen(FILE_NAME, "wb");
    if (file == NULL) {
        printf("Unable to create file.\n");
        free(writeData);
        free(readData);
        return 1;
    }

    start = clock();
    if (fwrite(writeData, 1, FILE_SIZE, file) < FILE_SIZE) {
        printf("Error writing to file.\n");
        fclose(file);
        free(writeData);
        free(readData);
        return 1;
    }
    end = clock();

    // Measure Write Throughput
    writeTime = ((double)(end - start)) / CLOCKS_PER_SEC;
    writeThroughput = FILE_SIZE / writeTime / (1024 * 1024);
    printf("Write Time: %f seconds, Write Throughput: %f MB/sec\n", writeTime, writeThroughput);

    fclose(file);

    // Step 2: Read Data from File
    file = fopen(FILE_NAME, "rb");
    if (file == NULL) {
        printf("Unable to open file for reading.\n");
        free(writeData);
        free(readData);
        return 1;
    }

    start = clock();
    if (fread(readData, 1, FILE_SIZE, file) < FILE_SIZE) {
        printf("Error reading from file.\n");
        fclose(file);
        free(writeData);
        free(readData);
        return 1;
    }
    end = clock();

    // Measure Read Throughput
    readTime = ((double)(end - start)) / CLOCKS_PER_SEC;
    readThroughput = FILE_SIZE / readTime / (1024 * 1024);
    printf("Read Time: %f seconds, Read Throughput: %f MB/sec\n", readTime, readThroughput);

    fclose(file);

    // Delete File
    if (remove(FILE_NAME) != 0) {
        printf("Error deleting file.\n");
    } else {
        printf("File successfully deleted.\n");
    }

    free(writeData);
    free(readData);

    return 0;
}
