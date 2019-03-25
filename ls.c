#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_PATH_COUNT 256
#define MAX_PATH_LEN 256

char path_argv[MAX_PATH_COUNT][MAX_PATH_LEN];
int path_argc = 0;

int ls_a = 0;
int ls_d = 0;
int ls_i = 0;
int ls_l = 0;
int ls_R = 0;

void decode_args(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') { // option
            int opt_len = (int) strlen(argv[i]);
            for (int j = 1; j < opt_len; j++) {
                switch (argv[i][j]) {
                    case 'a':
                        // list all
                        ls_a = 1;
                        break;
                    case 'd':
                        // list directory itself
                        ls_d = 1;
                        break;
                    case 'i':
                        // list inode number
                        ls_i = 1;
                        break;
                    case 'l':
                        // list info in detail
                        ls_l = 1;
                        break;
                    case 'R':
                        // list recursively
                        ls_R = 1;
                        break;
                    default:
                        break;
                }
            }
        } else { // path
            if (path_argc < MAX_PATH_COUNT)
                strcpy(path_argv[path_argc++], argv[i]);
        }
    }
    if (path_argc == 0) {
        strcpy(path_argv[path_argc++], ".");
    }
}

void format_mode(mode_t st_mode, char *mode) {
    unsigned int mask = 0700;
    static const char *perm[] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};

    char type;
    if (S_ISREG(st_mode))
        type = '-';
    else if (S_ISDIR(st_mode))
        type = 'd';
    else if (S_ISCHR(st_mode))
        type = 'c';
    else if (S_ISLNK(st_mode))
        type = 'l';
    else if (S_ISFIFO(st_mode))
        type = 'p';
    else if (S_ISSOCK(st_mode))
        type = 's';
    else if (S_ISBLK(st_mode))
        type = 'b';
    else
        type = '?';
    mode[0] = type;

    int i = 3;
    char *ptr = mode + 1;
    while (i > 0) {
        *ptr++ = perm[(st_mode & mask) >> (i - 1) * 3][0];
        *ptr++ = perm[(st_mode & mask) >> (i - 1) * 3][1];
        *ptr++ = perm[(st_mode & mask) >> (i - 1) * 3][2];
        i--;
        mask >>= 3;
    }
}

void uid_to_name(uid_t uid, char *uname) {
    struct passwd *pw_ptr;
    if ((pw_ptr = getpwuid(uid)) == NULL)
        sprintf(uname, "%d", uid);
    else
        sprintf(uname, "%s", pw_ptr->pw_name);
}

void gid_to_name(gid_t gid, char *gname) {
    struct group *grp_ptr;
    if ((grp_ptr = getgrgid(gid)) == NULL)
        sprintf(gname, "%d", gid);
    else
        sprintf(gname, "%s", grp_ptr->gr_name);
}

void ls_file(char *file, struct stat *info) {
    struct stat node_info;
    if (info == NULL) {
        if (stat(file, &node_info) == -1) {
            fprintf(stderr, "ls: cannot access '%s'\n", file);
            return;
        } else {
            info = &node_info;
        }
    }

    if (ls_i) {
        printf("%lu ", info->st_ino);
    }

    if (ls_l) {
        char mode[11];
        memset(mode, '\0', sizeof(mode));
        format_mode(info->st_mode, mode);

        char uname[11];
        memset(uname, '\0', sizeof(uname));
        uid_to_name(info->st_uid, uname);

        char gname[11];
        memset(gname, '\0', sizeof(gname));
        gid_to_name(info->st_gid, gname);

        char mtime[64];
        memset(mtime, '\0', sizeof(mtime));
        strcpy(mtime, ctime(&info->st_mtime)); // Www Mmm dd hh:mm:ss yyyy
        mtime[strlen(mtime) - 1] = '\0'; // remove the \n added by ctime

        printf("%10s\t ", mode);
        printf("%3d\t ", info->st_nlink);
        printf("%10s\t ", uname);
        printf("%10s\t ", gname);
        printf("%8lu\t ", info->st_size);
        printf("%26s\t ", mtime);
    }
    printf("%s ", file);
    if (ls_l) printf("\n");
}

void ls_dir(char *dir, struct stat *info) {
    struct stat node_info;
    if (info == NULL) {
        if (stat(dir, &node_info) == -1) {
            fprintf(stderr, "ls: cannot access '%s'\n", dir);
            return;
        } else {
            info = &node_info;
        }
    }

    if (ls_d) {
        ls_file(dir, info);
        return;
    }

    DIR *dp;
    struct dirent *entry;
    if ((dp = opendir(dir)) == NULL) {
        fprintf(stderr, "ls: cannot open '%s'\n", dir);
    } else {
        char child_dir[MAX_PATH_COUNT][MAX_PATH_LEN];
        int child_dir_count = 0;

        if (ls_R) {
            printf("%s:\n", dir);
        }
        while ((entry = readdir(dp)) != NULL) {
            if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) { // . and ..
                if (ls_a) {
                    ls_file(entry->d_name, NULL);
                }
                continue;
            }
            if (entry->d_name[0] == '.' && !ls_a) // hidden entries
                continue;

            char entry_name[MAX_PATH_LEN];
            strcpy(entry_name, dir);
            if (entry_name[strlen(entry_name) - 1] != '/') {
                char *ptr = entry_name + strlen(entry_name);
                *ptr++ = '/';
                *ptr = '\0';
            }
            strcat(entry_name, entry->d_name);

            struct stat entry_info;
            if (stat(entry_name, &entry_info) == -1) {
                fprintf(stderr, "ls: cannot access '%s'\n", entry_name);
            } else {
                if (S_ISDIR(entry_info.st_mode)) {
                    strcpy(child_dir[child_dir_count++], entry_name);
                }
                ls_file(entry->d_name, &entry_info);
            }
        }
        printf("\n\n");

        if (ls_R) {
            for (int i = 0; i < child_dir_count; i++) {
                ls_dir(child_dir[i], NULL);
            }
        }
        closedir(dp);
    }
}

void ls() {
    for (int i = 0; i < path_argc; i++) {
        struct stat info;
        if (stat(path_argv[i], &info) == -1) {
            fprintf(stderr, "ls: cannot access '%s'\n", path_argv[i]);
        } else {
            if (S_ISDIR(info.st_mode)) {
                char *dir = path_argv[i];
                if (dir[strlen(dir) - 1] != '/') {
                    char *ptr = dir + strlen(dir);
                    *ptr++ = '/';
                    *ptr = '\0';
                }
                ls_dir(dir, &info);
            } else {
                ls_file(path_argv[i], &info);
            }
        }
    }
}

int main(int argc, char **argv) {
    decode_args(argc, argv);
    ls();
    return 0;
}