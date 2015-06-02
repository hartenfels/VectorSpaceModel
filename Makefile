all: install test run

install:
	./install

build: build/libs/VectorSpaceModel-all.jar
build/libs/VectorSpaceModel-all.jar: src/main/java/*.java
	gradle -q fatpack

test: build
	carton exec prove

run: build
	carton exec ./vsm

clean:
	rm -rf _Inline *.stash
	gradle -q clean

realclean: clean
	rm -rf .gradle local cpanfile.snapshot

.PHONY: all install test run clean realclean
