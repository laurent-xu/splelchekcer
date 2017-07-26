.PHONY: all clean test

MK=$(MAKE) -s -C build
ARCHIVE_NAME=xu_e-dehlav_a-textmining

all: build
	$(MK)

export:
	git archive HEAD --prefix=$(ARCHIVE_NAME)/ \
		| bzip2 > $(ARCHIVE_NAME).tar.bz2

clean:
	rm -f TextMiningApp TextMiningCompiler
	rm -rf build

build: build/Makefile

build/Makefile: CMakeLists.txt
	mkdir -p build
	cd build && cmake ..
