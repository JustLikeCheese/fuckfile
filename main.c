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

typedef struct {
    int recursive;
    int force;
    int preserve_time;
    int preserve_size;
    int dry_run;
} Options;

int confirm_fuck(const char* path, Options* opts) {
    char input[256];
    
    if (opts->force) {
        return 1;
    }
    
    printf("Do you want fuck the file `%s` (y/n)? ", path);
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return 0;
    }
    
    input[strcspn(input, "\n")] = 0;
    
    if (strcmp(input, "y") != 0 && strcmp(input, "Y") != 0) {
        printf("Cancelled.\n");
        return 0;
    }
    
    printf("Please type 'fuck' to confirm: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return 0;
    }
    
    input[strcspn(input, "\n")] = 0;
    
    if (strcmp(input, "fuck") != 0) {
        printf("Cancelled.\n");
        return 0;
    }
    
    return 1;
}

int fuck_file(const char* path, Options* opts) {
    struct stat st;
    
    if (stat(path, &st) != 0) {
        fprintf(stderr, "Cannot stat file: %s\n", path);
        return 1;
    }
    
    if (S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Skipping directory: %s\n", path);
        return 0;
    }
    
    if (opts->dry_run) {
        char abs_path[4096];
#ifdef _WIN32
        if (_fullpath(abs_path, path, sizeof(abs_path)) != NULL) {
            printf("[DRY RUN] Would fuck: %s\n", abs_path);
        } else {
            printf("[DRY RUN] Would fuck: %s\n", path);
        }
#else
        if (realpath(path, abs_path) != NULL) {
            printf("[DRY RUN] Would fuck: %s\n", abs_path);
        } else {
            printf("[DRY RUN] Would fuck: %s\n", path);
        }
#endif
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
    
    long write_size = opts->preserve_size ? size : 0;
    
    if (write_size > 0) {
        size_t buffer_size = 1024 * 1024;
        unsigned char* buffer = (unsigned char*)calloc(buffer_size, 1);
        if (!buffer) {
            fclose(f);
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
        
        long remaining = write_size;
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
    }
    
    fclose(f);
    
    if (!opts->preserve_size && write_size == 0) {
#ifdef _WIN32
        HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);
        }
#else
        truncate(path, 0);
#endif
    }
    
    if (opts->preserve_time) {
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
    }
    
    printf("Fucked: %s (%ld bytes)\n", path, opts->preserve_size ? size : 0);
    return 0;
}

int fuck_directory(const char* path, Options* opts) {
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
            fuck_directory(full_path, opts);
        } else {
            fuck_file(full_path, opts);
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
                fuck_directory(full_path, opts);
            } else {
                fuck_file(full_path, opts);
            }
        }
    }
    
    closedir(dir);
#endif
    return 0;
}

void print_usage(const char* prog) {
    printf("fuckfile - Overwrite files with NULL bytes\n");
    printf("Usage: %s [options] <file|directory>\n\n", prog);
    printf("Options:\n");
    printf("  -r, -R    Recursively process directories\n");
    printf("  -f, -F    Force mode, no confirmation prompts\n");
    printf("  -t, -T    Do not preserve file modification time\n");
    printf("  -s, -S    Do not preserve file size (truncate to 0 bytes)\n");
    printf("  -a, -A    Aggressive mode (equivalent to -t -s)\n");
    printf("  -d, -D    Dry run, show what would be done without actually doing it\n");
}

int main(int argc, char* argv[]) {
    Options opts = {0, 0, 1, 1, 0};
    char* target = NULL;
    
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "-R") == 0) {
            opts.recursive = 1;
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "-F") == 0) {
            opts.force = 1;
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "-T") == 0) {
            opts.preserve_time = 0;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "-S") == 0) {
            opts.preserve_size = 0;
        } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "-A") == 0) {
            opts.preserve_time = 0;
            opts.preserve_size = 0;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "-D") == 0) {
            opts.dry_run = 1;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        } else {
            target = argv[i];
        }
    }
    
    if (!target) {
        print_usage(argv[0]);
        return 1;
    }
    
    struct stat st;
    if (stat(target, &st) != 0) {
        fprintf(stderr, "Cannot access: %s\n", target);
        return 1;
    }
    
    if (S_ISDIR(st.st_mode)) {
        if (!opts.recursive) {
            fprintf(stderr, "Use -r flag to process directories\n");
            return 1;
        }
        
        if (!confirm_fuck(target, &opts)) {
            return 0;
        }
        
        return fuck_directory(target, &opts);
    } else {
        if (!confirm_fuck(target, &opts)) {
            return 0;
        }
        
        return fuck_file(target, &opts);
    }
    
    return 0;
}