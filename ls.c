#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

#define MAX_PATH_COUNT 255
#define MAX_PATH_LEN 255

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

const void *uid_to_name(uid_t uid, char *uname) {
    struct passwd *pw_ptr;
    if ((pw_ptr = getpwuid(uid)) == NULL) {
        sprintf(uname, "%d", uid);
    } else
        return pw_ptr->pw_name;
}

const char *gid_to_name(gid_t gid, char *gname) {
    struct group *grp_ptr;
    if ((grp_ptr = getgrgid(gid)) == NULL) {
        sprintf(gname, "%d", gid);
    } else
        return grp_ptr->gr_name;
}

void ls_file(char *file, struct stat *info) {
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

        char mtime[255];
        memset(mtime, '\0', sizeof(mtime));
        strcpy(mtime, ctime(&info->st_mtime));
        mtime[strlen(mtime) - 1] = '\0'; // remove the \n added by ctime

        printf("%10s ", mode);
        printf("%3d ", info->st_nlink);
        printf("%14s ", uname);
        printf("%14s ", gname);
        printf("%8u ", (unsigned int) info->st_size);
        printf("%26s ", mtime); // Www Mmm dd hh:mm:ss yyyy
    }
    printf("%s\n", file);
}

void ls_dir(char *dir, struct stat *info) {
    if (ls_d) {
        ls_file(dir, info);
        return;
    }
}

void ls() {
    for (int i = 0; i < path_argc; i++) {
        struct stat info;
        if (stat(path_argv[i], &info) == -1) {
            fprintf(stderr, "ls: cannot access '%s'", path_argv[i]);
        } else {
            if (S_ISDIR(info.st_mode)) {
                char* dir = path_argv[i];
                if (dir[strlen(dir) - 1] != '/') {
                    char* ptr = dir + strlen(dir);
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