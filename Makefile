all: install test run

install:
	./install

build: build/libs/VectorSpaceModel-all.jar
build/libs/VectorSpaceModel-all.jar: src/main/java/*.java
	gradle -q fatpack

test: build
	carton exec prove

run:
	echo TODO

clean:
	gradle clean
	rm -rf _Inline

realclean: clean
	rm -rf .gradle local cpanfile.snapshot

.PHONY: all install test run clean realclean
