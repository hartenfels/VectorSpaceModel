all: install test run

install:
	carton install

test:
	carton exec prove

run:
	carton exec ./vsm

clean:
	rm -rf _Inline *.stash

realclean: clean
	rm -rf local cpanfile.snapshot

.PHONY: all install test run clean realclean
