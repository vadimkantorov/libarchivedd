libarchive primer of printing byte offsets/sizes for uncompressed entries in formats: TAR, ZIP, ISO

On 29 august 2024 [libarchive](https://github.com/libarchive/libarchive) does not support built-in calculation of byte offsets of uncompressed entries. So a hack is needed. This feature might be added to libarchive in the future, see issues referenced below.

[`libarhivedd.c`](./libarchivedd.c) is modeled after https://github.com/libarchive/libarchive/blob/master/examples/untar.c and https://github.com/libarchive/libarchive/wiki/Examples#A_Complete_Extractor, another useful example is https://github.com/libarchive/libarchive/blob/master/contrib/untar.c

```shell
make

# sudo apt install -y zip mkisofs
make libarchivedd.tar libarchivedd.zip libarchivedd.iso

./libarchivedd libarchivedd.tar
./libarchivedd libarchivedd.zip
./libarchivedd libarchivedd.iso
```


# References
- https://github.com/libarchive/libarchive/issues/2295
- https://github.com/libarchive/libarchive/issues/2283
