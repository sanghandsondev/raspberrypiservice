all: install

install: 
	cd coremgr && $(MAKE) install
	cd recordmgr && $(MAKE) install

cross:
	cd coremgr && $(MAKE) cross
	cd recordmgr && $(MAKE) cross

deploy:
	cd coremgr && $(MAKE) deploy
	cd recordmgr && $(MAKE) deploy

setenv:
	./setenv.sh

clean:
	cd coremgr && $(MAKE) clean
	cd recordmgr && $(MAKE) clean
	rm -rf build

.PHONY: all install cross deploy setenv clean