SUBDIRS         = lib_paho src

all:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $1; \
	done

.PHONY: clean
clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

