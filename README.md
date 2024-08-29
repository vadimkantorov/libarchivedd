libarchive primer of printing byte offsets/sizes for uncompressed entries in formats: TAR, ISO, ZIP

On 29 august 2024 [libarchive](https://github.com/libarchive/libarchive) does not support built-in calculation of byte offsets of uncompressed entries. So a hack is needed. This feature might be added to libarchive in the future, see issues referenced below.

```shell
make

./libarchivedd libarchivedd.tar
./libarchivedd libarchivedd.iso
./libarchivedd libarchivedd.zip
```


# References
- https://github.com/libarchive/libarchive/issues/2295
- https://github.com/libarchive/libarchive/issues/2283
