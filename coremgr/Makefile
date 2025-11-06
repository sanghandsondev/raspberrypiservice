all: install

build: clean
	mkdir -p build
	cd build && cmake .. 
	cd build && make

install: build
	cd build && sudo make install

cross: clean
	mkdir -p build
	cd build && cmake -DCMAKE_TOOLCHAIN_FILE=../rpi4_toolchain.cmake ..
	cd build && make

deploy:
	./deploy.sh

setenv:
	./setenv.sh

clean:
	rm -rf build

.PHONY: all build install cross deploy setenv clean