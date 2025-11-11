all: install

install: clean
	mkdir -p build
	cd build && cmake .. 
	cd build && make

cross: clean
	mkdir -p build
	cd build && cmake -DCMAKE_TOOLCHAIN_FILE=../../rpi4_toolchain.cmake ..
	cd build && make

deploy:
	./deploy.sh

setenv:
	../setenv.sh

clean:
	rm -rf build

.PHONY: all install cross deploy setenv clean