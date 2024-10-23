cc_directives := -std=c++20

all: bin/preprocess

run: test-preprocess

bin/preprocess: src/preprocess.cc src/helpers.cc src/language.h
	g++ ${cc_directives} src/helpers.cc src/preprocess.cc -o bin/preprocess

clean:
	rm build/preprocess

test-preprocess: bin/preprocess
	bin/preprocess test/test.c

lint:
# Requires cpplint to be installed
# 	See: https://github.com/cpplint/cpplint
	cpplint src/preprocess.cc