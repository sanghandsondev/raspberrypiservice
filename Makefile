# Define module mappings and default behavior
ifeq ($(module),core)
    TARGET_MODULES = coremgr

else ifeq ($(module),record)
    TARGET_MODULES = recordmgr

else ifeq ($(module),hardware)
    TARGET_MODULES = hardwaremgr

else ifeq ($(module),)
    TARGET_MODULES = coremgr recordmgr hardwaremgr
else
    $(error "Unknown module '$(module)'. Use 'core', 'record', or 'hardware'.")
endif

all: install

install: 
	@for mod in $(TARGET_MODULES); do \
		echo "--- Building module: $$mod ---"; \
		$(MAKE) -C $$mod install; \
	done

deploy:
	@for mod in $(TARGET_MODULES); do \
		echo "--- Deploying module: $$mod ---"; \
		$(MAKE) -C $$mod deploy; \
	done

setenv:
	./setenv.sh

clean:
	@for mod in $(TARGET_MODULES); do \
		echo "--- Cleaning module: $$mod ---"; \
		$(MAKE) -C $$mod clean; \
	done
	@if [ -z "$(module)" ]; then \
		echo "--- Cleaning root build directory ---"; \
		rm -rf build; \
	fi

.PHONY: all install deploy setenv clean