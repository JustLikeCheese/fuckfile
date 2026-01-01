#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define PATH_SEPARATOR '\\'
    #define stat _stat
    #define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#else
    #include <dirent.h>
    #include <utime.h>
    #include <unistd.h>
    #define PATH_SEPARATOR '/'
#endif

int confirm_fuck() {
    char input[256];
    int count = 0;
    
    printf("\nWARNING: This will DESTROY file contents!\n");
    printf("Type 'fuck' three times to confirm:\n");
    
    for (int i = 0; i < 3; i++) {
        printf("%d/3: ", i + 1);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            return 0;
        }
        
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "fuck") == 0) {
            count++;
        } else {
            printf("Cancelled.\n");
            return 0;
        }
    }
    
    return count == 3;
}

int fuck_file(const char* path) {
    struct stat st;
    
    if (stat(path, &st) != 0) {
        fprintf(stderr, "Cannot stat file: %s\n", path);
        return 1;
    }
    
    if (S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Skipping directory: %s\n", path);
        return 0;
    }
    
    FILE* f = fopen(path, "rb+");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", path);
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    size_t buffer_size = 1024 * 1024;
    unsigned char* buffer = (unsigned char*)calloc(buffer_size, 1);
    if (!buffer) {
        fclose(f);
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    long remaining = size;
    while (remaining > 0) {
        size_t to_write = remaining > buffer_size ? buffer_size : remaining;
        if (fwrite(buffer, 1, to_write, f) != to_write) {
            fprintf(stderr, "Write failed for: %s\n", path);
            free(buffer);
            fclose(f);
            return 1;
        }
        remaining -= to_write;
    }
    
    free(buffer);
    fclose(f);
    
#ifdef _WIN32
    HANDLE hFile = CreateFileA(path, FILE_WRITE_ATTRIBUTES, 
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        FILETIME ft;
        SYSTEMTIME st_sys;
        struct tm* timeinfo = localtime(&st.st_mtime);
        
        st_sys.wYear = timeinfo->tm_year + 1900;
        st_sys.wMonth = timeinfo->tm_mon + 1;
        st_sys.wDay = timeinfo->tm_mday;
        st_sys.wHour = timeinfo->tm_hour;
        st_sys.wMinute = timeinfo->tm_min;
        st_sys.wSecond = timeinfo->tm_sec;
        st_sys.wMilliseconds = 0;
        
        SystemTimeToFileTime(&st_sys, &ft);
        SetFileTime(hFile, NULL, NULL, &ft);
        CloseHandle(hFile);
    }
#else
    struct utimbuf times;
    times.actime = st.st_atime;
    times.modtime = st.st_mtime;
    utime(path, &times);
#endif
    
    printf("Fucked: %s (%ld bytes)\n", path, size);
    return 0;
}

int fuck_directory(const char* path) {
#ifdef _WIN32
    WIN32_FIND_DATAA find_data;
    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", path);
    
    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Cannot open directory: %s\n", path);
        return 1;
    }
    
    do {
        if (strcmp(find_data.cFileName, ".") == 0 || 
            strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }
        
        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s\\%s", path, find_data.cFileName);
        
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            fuck_directory(full_path);
        } else {
            fuck_file(full_path);
        }
    } while (FindNextFileA(hFind, &find_data));
    
    FindClose(hFind);
#else
    DIR* dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "Cannot open directory: %s\n", path);
        return 1;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || 
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                fuck_directory(full_path);
            } else {
                fuck_file(full_path);
            }
        }
    }
    
    closedir(dir);
#endif
    return 0;
}

void print_usage(const char* prog) {
    printf("fuckfile - Overwrite files with NULL bytes\n");
    printf("Usage: %s [-r] <file|directory>\n", prog);
    printf("  -r    Recursively process directories\n");
}

int main(int argc, char* argv[]) {
    int recursive = 0;
    char* target = NULL;
    
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0) {
            recursive = 1;
        } else {
            target = argv[i];
        }
    }
    
    if (!target) {
        print_usage(argv[0]);
        return 1;
    }
    
    if (!confirm_fuck()) {
        return 0;
    }
    
    struct stat st;
    if (stat(target, &st) != 0) {
        fprintf(stderr, "Cannot access: %s\n", target);
        return 1;
    }
    
    if (S_ISDIR(st.st_mode)) {
        if (!recursive) {
            fprintf(stderr, "Use -r flag to process directories\n");
            return 1;
        }
        return fuck_directory(target);
    } else {
        return fuck_file(target);
    }
    
    return 0;
}