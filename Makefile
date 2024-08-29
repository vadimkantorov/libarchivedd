libarchivedd: libarchive/.libs/libarchive.a
	cc -static -Wall -o $@ libarchivedd.c -larchive -L../libarchive/.libs -I../libarchive -I../libarchive/libarchive

libarchive/.libs/libarchive.a:
	cd libarchive && sh build/autogen.sh && sh configure --without-zlib --without-bz2lib  --without-libb2 --without-iconv --without-lz4  --without-zstd --without-lzma --without-cng  --without-xml2 --without-expat --without-openssl && $(MAKE)

libarchivedd.tar:
	tar -cf $@ libarchivedd.c Makefile

libarchivedd.zip:
	zip -0 $@ libarchivedd.c Makefile
