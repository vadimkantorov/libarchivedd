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
