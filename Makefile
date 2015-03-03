SRCDIR=src
BUILDDIR=build
BUILDBINDIR=$(BUILDDIR)/bin


all: $(BUILDBINDIR)/tlauncher.exe $(BUILDBINDIR)/ulauncher.exe

$(BUILDBINDIR)/tlauncher.exe: $(BUILDBINDIR) $(SRCDIR)/tlauncher.exe
	cp $(SRCDIR)/tlauncher.exe $(BUILDBINDIR)

$(BUILDBINDIR)/ulauncher.exe: $(BUILDBINDIR) $(SRCDIR)/ulauncher.exe
	cp $(SRCDIR)/ulauncher.exe $(BUILDBINDIR)

$(BUILDBINDIR):
	mkdir -p $(BUILDBINDIR)

$(SRCDIR)/tlauncher.exe:
	make -C $(SRCDIR)

$(SRCDIR)/ulauncher.exe:
	make -C $(SRCDIR)


clean:
	rm -f -r $(BUILDDIR)
	make clean -C $(SRCDIR)

