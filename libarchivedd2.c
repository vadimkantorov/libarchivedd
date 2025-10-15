#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>

#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <archive.h>
#include <archive_entry.h>

struct my_data
{
    int fd;
    size_t block_size;
    uint8_t buffer[1024 * 4];
};

void* last_file_buff;
size_t last_file_block_size;
size_t last_file_offset;

typedef int64_t la_seek_t;

int64_t my_seek_callback(struct archive *a, void *client_data, int64_t request, int whence)
{
    // https://github.com/libarchive/libarchive/blob/master/libarchive/archive_read_open_fd.c
    struct my_data *mine = (struct my_data *)client_data;
    la_seek_t seek = (la_seek_t)request;
    int64_t r;
    int seek_bits = sizeof(seek) * 8 - 1;  /* off_t is a signed type. */

    /* We use off_t here because lseek() is declared that way. */

    /* Do not perform a seek which cannot be fulfilled. */
    if (sizeof(request) > sizeof(seek)) {
            const int64_t max_seek =
                (((int64_t)1 << (seek_bits - 1)) - 1) * 2 + 1;
            const int64_t min_seek = ~max_seek;
            if (request < min_seek || request > max_seek) {
                    errno = EOVERFLOW;
                    goto err;
            }
    }

    r = lseek(mine->fd, seek, whence);
    if (r >= 0)
            return r;

err:
    if (errno == ESPIPE) {
            archive_set_error(a, errno,
                "A file descriptor(%d) is not seekable(PIPE)", mine->fd);
            return (ARCHIVE_FAILED);
    } else {
            /* If the input is corrupted or truncated, fail. */
            archive_set_error(a, errno,
                "Error seeking in a file descriptor(%d)", mine->fd);
            return (ARCHIVE_FATAL);
    }
}


ssize_t my_read_callback(struct archive *a, void *client_data, const void **buff)
{
    // https://github.com/libarchive/libarchive/blob/master/libarchive/archive_read_open_fd.c
    struct my_data *mine = (struct my_data *)client_data;
    ssize_t bytes_read;
    
    last_file_buff = mine->buffer;
    last_file_block_size = mine->block_size;
    last_file_offset = my_seek_callback(a, client_data, 0, SEEK_CUR);

    *buff = mine->buffer;

    for (;;)
    {
            bytes_read = read(mine->fd, mine->buffer, mine->block_size);
            if (bytes_read < 0)
            {
                    if (errno == EINTR)
                            continue;
                    archive_set_error(a, errno, "Error reading fd %d",
                        mine->fd);
            }
            return (bytes_read);
    }
    return 0;
}

int main(int argc, const char **argv)
{
    // https://github.com/libarchive/libarchive/issues/2283

    if(argc < 2)
        return 1;

    const char *filename = argv[1];

    struct archive *a = archive_read_new();
    archive_read_support_format_tar(a);
    archive_read_support_format_iso9660(a);
    archive_read_support_format_zip(a);
    
    struct my_data mydata;
    mydata.fd = open(filename, O_RDONLY);
    mydata.block_size = sizeof(mydata.buffer);
    archive_read_set_seek_callback(a, my_seek_callback);
    archive_read_set_read_callback(a, my_read_callback);
    archive_read_set_callback_data(a, &mydata);

    int r = archive_read_open1(a);
    if (r != ARCHIVE_OK) { fprintf(stderr, "#%s\n", archive_error_string(a)); return r; }
    
    for(;;)
    {
        struct archive_entry *entry;
        int r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) break;
        if (r != ARCHIVE_OK) { fprintf(stderr, "#%s\n", archive_error_string(a)); return r; }

        const void* firstblock_buff;
        size_t firstblock_len;
        int64_t firstblock_offset;
        r = archive_read_data_block(a, &firstblock_buff, &firstblock_len, &firstblock_offset);
        
        int filetype = archive_entry_filetype(entry);
        if(filetype == AE_IFREG && archive_entry_size_is_set(entry) != 0 && last_file_buff != NULL && last_file_buff <= firstblock_buff && firstblock_buff < last_file_buff + last_file_block_size)
        {
            size_t byte_size = (size_t)archive_entry_size(entry);
            size_t byte_offset = last_file_offset + (size_t)(firstblock_buff - last_file_buff);
            printf("#dd if=\"%s\" of=\"%s\" bs=1 skip=%zu count=%zu\n", filename, archive_entry_pathname(entry), byte_offset, byte_size);
        }
        else
        {
            fprintf(stderr, "#false #%s %d = %s\n", archive_entry_pathname(entry), filetype, filetype == AE_IFMT ? "AE_IFMT" : filetype == AE_IFREG ? "AE_IFREG" : filetype == AE_IFLNK ? "AE_IFLNK" : filetype == AE_IFSOCK ? "AE_IFSOCK" : filetype == AE_IFCHR ? "AE_IFCHR" : filetype == AE_IFBLK ? "AE_IFBLK" : filetype == AE_IFDIR ? "AE_IFDIR" : filetype == AE_IFIFO ? "AE_IFIFO" : "archive_entry_pathname(entry) value is unknown");
        }
        
        r = archive_read_data_skip(a);
        if (r == ARCHIVE_EOF) break;
        if (r != ARCHIVE_OK) { fprintf(stderr, "#%s\n", archive_error_string(a)); return r; }
    }
    r = archive_read_close(a);
    if (r != ARCHIVE_OK) { fprintf(stderr, "#%s\n", archive_error_string(a)); return r; }
    r = archive_read_free(a);
    if (r != ARCHIVE_OK) { fprintf(stderr, "#%s\n", archive_error_string(a)); return r; }
    return 0;
}
