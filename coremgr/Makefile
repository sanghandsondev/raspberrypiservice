all: install

install: clean
	mkdir -p build
	cd build && cmake .. 
	cd build && make

deploy: clean
	mkdir -p build
	cd build && cmake -DCMAKE_TOOLCHAIN_FILE=../../rpi4_toolchain.cmake ..
	cd build && make
	./deploy.sh

clean:
	rm -rf build

.PHONY: all install deploy clean