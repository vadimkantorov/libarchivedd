libarchive primer of printing byte offsets/sizes for uncompressed entries in formats: TAR, ZIP, ISO

On 29 august 2024 [libarchive](https://github.com/libarchive/libarchive) does not support built-in calculation of byte offsets of uncompressed entries. So a hack is needed. This feature might be added to libarchive in the future, see issues referenced below.

[`libarhivedd.c`](./libarchivedd.c) is modeled after https://github.com/libarchive/libarchive/blob/master/examples/untar.c and https://github.com/libarchive/libarchive/wiki/Examples#A_Complete_Extractor, another useful example is https://github.com/libarchive/libarchive/blob/master/contrib/untar.c

```shell
make

# sudo apt install -y zip mkisofs
make libarchivedd.tar libarchivedd.zip libarchivedd.iso

./libarchivedd libarchivedd.tar
#dd if="libarchivedd.tar" of="libarchivedd.c" bs=1 skip=512 count=3756
#dd if="libarchivedd.tar" of="Makefile" bs=1 skip=5120 count=616

./libarchivedd libarchivedd.zip
#dd if="libarchivedd.zip" of="libarchivedd.c" bs=1 skip=72 count=3756
#dd if="libarchivedd.zip" of="Makefile" bs=1 skip=3894 count=616

./libarchivedd libarchivedd.iso
#false #. AE_IFDIR : 16384
#dd if="libarchivedd.iso" of="libarchivedd.c" bs=1 skip=63488 count=3756
#dd if="libarchivedd.iso" of="Makefile" bs=1 skip=67584 count=616
```

# References
- https://github.com/libarchive/libarchive/issues/2295
- https://github.com/libarchive/libarchive/issues/2283

## Bonus: helpers of extracting an entry to fd, `FILE*` and mem buffer

```cpp
size_t extract_to_fd(struct archive* a, const void* firstblock_buff, size_t firstblock_size, int64_t firstblock_offset, int fd)
{
    (void)firstblock_offset;
    size_t size = write(fd, firstblock_buff, firstblock_size);
    for(;;)
    {
        int r = archive_read_data_block(a, &firstblock_buff, &firstblock_size, &firstblock_offset);
        if (r == ARCHIVE_EOF || r != ARCHIVE_OK)
	    break;
        assert(r == ARCHIVE_OK);
        size += write(fd, firstblock_buff, firstblock_size);
    }
    return size;
}

size_t extract_to_file(struct archive* a, const void* firstblock_buff, size_t firstblock_size, int64_t firstblock_offset, FILE* stream)
{
    (void)firstblock_offset;
    size_t size = fwrite(firstblock_buff, 1, firstblock_size, stream);
    for(;;)
    {
        int r = archive_read_data_block(a, &firstblock_buff, &firstblock_size, &firstblock_offset);
        if (r == ARCHIVE_EOF || r != ARCHIVE_OK)
	    break;
        assert(r == ARCHIVE_OK);
        size += fwrite(firstblock_buff, 1, firstblock_size, stream);
    }
    return size;
}

size_t extract_to_mem(struct archive* a, const void* firstblock_buff, size_t firstblock_size, int64_t firstblock_offset, void* buf, size_t cnt)
{
    (void)firstblock_offset;
    size_t size = firstblock_size <= cnt ? firstblock_size : cnt;
    memcpy(buf, firstblock_buff, size);
    cnt -= size;
    for(;;)
    {
        int r = archive_read_data_block(a, &firstblock_buff, &firstblock_size, &firstblock_offset);
        if (r == ARCHIVE_EOF || r != ARCHIVE_OK)
	    break;
        assert(r == ARCHIVE_OK);
        size_t w = firstblock_size <= cnt ? firstblock_size : cnt;
        memcpy((char*)buf + size, firstblock_buff, w);
        cnt -= w;
    }
    return size;
}

// ... 

//extract_to_fd(a, firstblock_buff, firstblock_len, firstblock_offset, 2);
//extract_to_file(a, firstblock_buff, firstblock_len, firstblock_offset, stderr);
//char tmp[1024]; size_t size = extract_to_mem(a, firstblock_buff, firstblock_len, firstblock_offset, tmp, 1024); tmp[size <= 1023 ? size : 1023] = '\0'; fprintf(stderr, "%s", tmp);
```
