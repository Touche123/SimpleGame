#include "io.h"
#include <stdio.h>

char* loadTextFile(char* path) {
    FILE *file;
    long length;
    char *content;

    file = fopen(path, "r");
    if (file == NULL) {
        printf("File not found\n");
        return NULL;
    }

    // Get the file length
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory
    content = (char*)malloc(length + 1); // +1 for including null-terminator
    if (content == NULL) {
        printf("Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    // Read the file
    fread(content, 1, length, file);
    content[length] = '\0';  // Add null-terminator

    fclose(file);

    if (length >= 3 && (unsigned char)content[0] == 0xEF && (unsigned char)content[1] == 0xBB && (unsigned char)content[2] == 0xBF) {
        // Shift the content to remove BOM
        memmove(content, content + 3, length - 2);
        content[length - 3] = '\0'; // Null-terminate after removing BOM
    }

    int i = length - 1;
    while (i >= 0 && (content[i] == 0 || content[i] < 32 || content[i] > 126)) {
        content[i] = '\0';  // Null-terminate at the last valid character
        i--;
    }

    return content;
}