# this Makefile is for UNIX

# targets:

all: libs
	(cd src; $(MAKE) all)

libs:
	(cd lib; $(MAKE) all)

lint:
	(cd lib; $(MAKE) $@)
	(cd src; $(MAKE) $@)


# clean up:

clean:
	$(RM) *~

distclean: clean
	(cd lib; $(MAKE) $@)
	(cd src; $(MAKE) $@)
