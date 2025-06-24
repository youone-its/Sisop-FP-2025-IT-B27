#define FUSE_USE_VERSION 35

#include <fuse3/fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <time.h>
#include <limits.h>

static const char *real_root = "/home/juanz/Documents/matkul/sisop/Sisop-FP-2025-IT-B27/real";

static void fullpath(char fpath[PATH_MAX], const char *path) {
    if (path[0] == '/') {
        snprintf(fpath, PATH_MAX, "%s%s", real_root, path);
    } else {
        snprintf(fpath, PATH_MAX, "%s/%s", real_root, path);
    }
}

static int command_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    char fpath[PATH_MAX];
    fullpath(fpath, path);

    int res = lstat(fpath, stbuf);
    if (res == -1)
        return -errno;
    return 0;
}

static int command_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    char fpath[PATH_MAX];
    fullpath(fpath, path);
    DIR *dp = opendir(fpath);
    if (dp == NULL)
        return -errno;

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0, 0))
            break;
    }
    closedir(dp);
    return 0;
}

static int command_open(const char *path, struct fuse_file_info *fi) {
    char fpath[PATH_MAX];
    fullpath(fpath, path);

    int fd = open(fpath, fi->flags);
    if (fd == -1)
        return -errno;

    close(fd);
    return 0;
}

static int command_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi) {
    char fpath[PATH_MAX];
    fullpath(fpath, path);

    int fd = open(fpath, O_RDONLY);
    if (fd == -1)
        return -errno;

    int res = pread(fd, buf, size, offset);
    if (res == -1)
        res = -errno;
    close(fd);
    return res;
}

static int command_unlink(const char *path) {
    char source[PATH_MAX];
    fullpath(source, path);

    const char *home = getenv("HOME");
    if (!home) return -EIO;
    char trash_dir[PATH_MAX];
    snprintf(trash_dir, sizeof(trash_dir), "%s/.trash", home);
    mkdir(trash_dir, 0755);

    char *path_dup = strdup(path);
    char *filename = basename(path_dup);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char timestamp[64];
    snprintf(timestamp, sizeof(timestamp), "_%04d%02d%02d%02d%02d%02d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec);

    char dest[PATH_MAX * 2];
    if (strlen(trash_dir) + strlen(filename) + strlen(timestamp) + 2 >= sizeof(dest)) {
        fprintf(stderr, "Path terlalu panjang, file tidak dipindahkan.\n");
        free(path_dup);
        return -ENAMETOOLONG;
    }
    snprintf(dest, sizeof(dest), "%s/%s%s", trash_dir, filename, timestamp);

    int res = rename(source, dest);
    if (res == -1) {
        if (errno == ENOENT) {
            fprintf(stderr, "File tidak ditemukan: %s\n", source);
        } else {
            perror("Gagal memindahkan file");
        }
        free(path_dup);
        return -errno;
    }
    free(path_dup);
    return 0;
}

static int command_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char fpath[PATH_MAX];
    fullpath(fpath, path);

    int fd = creat(fpath, mode);
    if (fd == -1)
        return -errno;

    fi->fh = fd;
    return 0;
}

static int command_write(const char *path, const char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi) {
    char fpath[PATH_MAX];
    fullpath(fpath, path);

    int fd = open(fpath, O_WRONLY);
    if (fd == -1)
        return -errno;

    int res = pwrite(fd, buf, size, offset);
    if (res == -1)
        res = -errno;
    close(fd);
    return res;
}

static int command_ioctl(const char *path, unsigned int cmd, void *arg,
                     struct fuse_file_info *fi, unsigned int flags, void *data) {
    if (cmd == 0x12345678) {
        const char *src = (const char *)arg;
        const char *home = getenv("HOME");
        if (!home) return -EIO;
        
        char trash_path[PATH_MAX*2];
        snprintf(trash_path, sizeof(trash_path), "%s/%s", home, src);
        char dest[PATH_MAX*2];
        snprintf(dest, sizeof(dest), "%s/%s", real_root, src);
        if (rename(trash_path, dest) == -1)
            return -errno;
        return 0;
    }
    return -EINVAL;
}

static struct fuse_operations command_oper = {
    .getattr = command_getattr,
    .readdir = command_readdir,
    .open    = command_open,
    .read    = command_read,
    .unlink  = command_unlink,
    .write   = command_write,
    .create  = command_create,
    .ioctl   = command_ioctl,
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <mountpoint>\n", argv[0]);
        return 1;
    }

    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "HOME environment variable not set.\n");
        return 1;
    }
    char trash_dir[PATH_MAX];
    snprintf(trash_dir, sizeof(trash_dir), "%s/.trash", home);
    mkdir(trash_dir, 0755);

    return fuse_main(argc, argv, &command_oper, NULL);
}
