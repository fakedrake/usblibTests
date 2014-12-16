root = $(CURDIR)/root

# For mac
export DYLD_LIBRARY_PATH=$(root)/lib:$$DYLD_LIBRARY_PATH

# For linux
export LD_LIBRARY_PATH=$(root)/lib:$LD_LIBRARY_PATH
CFLAGS=-Wall -I$(root)/include/libusb-1.0 -g
libusb_h = $(root)/include/libusb-1.0/libusb.h
libusb-repo = $(CURDIR)/libusb

ifeq ($(shell uname), Darwin)
	libusb = $(CURDIR)/root/lib/libusb-1.0.dylib
else
	libusb = $(CURDIR)/root/lib/libusb-1.0.so
endif

all: $(CURDIR)/libusbfind

.PHONY:
force: ;
$(CURDIR)/libusbfind: libusbfind.c $(libusb) $(libsb_h)
	gcc $(CFLAGS) $< -lusb-1.0 -o $@

$(libusb-repo):
	git clone https://github.com/libusb/libusb $@

$(libusb-repo)/Makefile:
	cd $(libusb-repo) && ./bootstrap.sh --prefix=$(root) --enable-debug-log


$(libusb): $(libusb-repo) $(libusb-repo)/Makefile
	@echo "Generating $(libusb) $(shell uname)"
	$(MAKE) -C $(libusb-repo) CFLAGS="-g"
	$(MAKE) -C $(libusb-repo) install

run-gdb: $(CURDIR)/libusbfind
	gdb --fullname $(CURDIR)/libusbfind

run: $(CURDIR)/libusbfind
	$(CURDIR)/libusbfind
