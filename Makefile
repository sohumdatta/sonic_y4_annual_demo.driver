.PHONY: build clean
all: build

build:
	mkdir -p build
	cd build && cmake .. && make -j9

clean:
	rm -rf build
