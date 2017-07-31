.PHONY: all clean test
REFDIR=project/ref/linux64/
COMPILERNAME=TextMiningCompiler
APPNAME=TextMiningApp
DICTNAME=dict.bin
REFCOMPILERNAME=$(REFDIR)$(COMPILERNAME)
REFAPPNAME=$(REFDIR)$(APPNAME)
REFDICTNAME=$(REFDIR)$(DICTNAME)
WORDSOURCE=project/words.txt

MK=$(MAKE) -s -C build
ARCHIVE_NAME=xu_e-dehlav_a-textmining

all: build
	$(MK)

$(DICTNAME): $(COMPILERNAME) $(WORDSOURCE)
	$(COMPILERNAME) $(WORDSOURCE) $(DICTNAME)

$(REFDICTNAME): $(REFCOMPILERNAME) $(WORDSOURCE)
	$(REFCOMPILERNAME) $(WORDSOURCE) $(REFDICTNAME)

dict:$(DICTNAME) $(REFDICTNAME)

check: dict $(WORDSOURCE)
	set e; \
	for i in `seq 0 3`; do \
		tests/check.sh $(WORDSOURCE) $$i $(APPNAME) $(REFAPPNAME) $(DICTNAME) $(REFDICTNAME); \
	done

bench: all $(DICTNAME) $(REFDICTNAME)
	tests/bench.sh $(APPNAME) $(WORDSOURCE) $(DICTNAME) $(REFAPPNAME) $(REFDICTNAME)

benchref:$(REFDICTNAME)
	tests/bench.sh $(REFAPPNAME) $(WORDSOURCE) $(REFDICTNAME)

doc: Doxyfile
	doxygen Doxyfile

export:
	git archive HEAD --prefix=$(ARCHIVE_NAME)/ \
		| bzip2 > $(ARCHIVE_NAME).tar.bz2

clean:
	rm -f TextMiningApp TextMiningCompiler
	rm -rf build
	rm -f $(DICTNAME)
	rm -f $(REFDICTNAME)

build: build/Makefile

build/Makefile: CMakeLists.txt
	mkdir -p build
	cd build && cmake ..
