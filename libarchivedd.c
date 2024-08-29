#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <archive.h>
#include <archive_entry.h>

#define __LIBARCHIVE_BUILD
#include <archive_read_private.h>

archive_read_callback* old_file_read;
archive_seek_callback* old_file_seek;
void* last_file_buff;
size_t last_file_block_size;
size_t last_file_offset;

// struct read_file_data copied from https://github.com/libarchive/libarchive/blob/master/libarchive/archive_read_open_filename.c
struct read_file_data {
	int	 fd;
	size_t	 block_size;
	void	*buffer;
	//mode_t	 st_mode;  /* Mode bits for opened file. */
	//char	 use_lseek;
	//enum fnt_e { FNT_STDIN, FNT_MBS, FNT_WCS } filename_type;
	//union {
	//	char	 m[1];/* MBS filename. */
	//	wchar_t	 w[1];/* WCS filename. */
	//} filename; /* Must be last! */
};

static ssize_t
new_file_read(struct archive *a, void *client_data, const void **buff)
{
    struct read_file_data *mine = (struct read_file_data *)client_data;
    last_file_buff = mine->buffer;
    last_file_block_size = mine->block_size;
    last_file_offset = old_file_seek(a, client_data, 0, SEEK_CUR);
    
    return old_file_read(a, client_data, buff);
}


int
main(int argc, const char **argv)
{
    if(argc < 2)
        return 1;

    const char *filename = argv[1];

    struct archive *a = archive_read_new();
    archive_read_support_format_tar(a);
    archive_read_support_format_iso9660(a);
    archive_read_support_format_zip(a);
    
    int r = archive_read_open_filename(a, filename, 10240);
    if (r)
    {
        fprintf(stderr, "%s\n", archive_error_string(a));
        return r;
    }
    
    // struct archive_read in https://github.com/libarchive/libarchive/blob/master/libarchive/archive_read_private.h
    struct archive_read *_a = ((struct archive_read *)a);
    old_file_read = _a->client.reader;
    old_file_seek = _a->client.seeker;
    
    a->state = ARCHIVE_STATE_NEW;
    archive_read_set_read_callback(a, new_file_read);
    r = archive_read_open1(a);
    if (r)
    {
        fprintf(stderr, "%s\n", archive_error_string(a));
        return r;
    }
    
    while(true)
    {
        struct archive_entry *entry;
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r != ARCHIVE_OK)
        {
            fprintf(stderr, "%s\n", archive_error_string(a));
            return r;
        }

        printf("x %s\n", archive_entry_pathname(entry));
        const void* buff;
        size_t len;
        int64_t offset;
        archive_read_data_block(a, &buff, &len, &offset);
        
        if(archive_entry_size_is_set(entry) != 0 && last_file_buff != NULL && last_file_buff <= buff && buff < last_file_buff + last_file_block_size)
            printf("    dd if=\"%s\" of=\"%s\" bs=1 skip=%zu count=%zu", argv[1], archive_entry_pathname(entry), last_file_offset + (size_t)(buff - last_file_buff), (size_t)archive_entry_size(entry));
        
        archive_read_data_skip(a);
        printf("\n");
    }
    archive_read_close(a);
    archive_read_free(a);
    return 0;
}

