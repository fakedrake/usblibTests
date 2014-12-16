root = $(CURDIR)/root

# For mac
export DYLD_LIBRARY_PATH=$(root)/lib:$$DYLD_LIBRARY_PATH

# For linux
export LD_LIBRARY_PATH=$(root)/lib:$LD_LIBRARY_PATH
CFLAGS=-Wall -I$(root)/include/libusb-1.0 -g
libusb_h = $(root)/include/libusb-1.0/libusb.h

ifeq$(shell uname), "Darwin")
	libusb = $(CURDIR)/root/lib/libusb-1.0.dylib
else
	libusb = $(CURDIR)/root/lib/libusb-1.0.so
endif

all: libusbfind

.PHONY:
force: ;
$(CURDIR)/libusbfind: libusb.c $(libusb) $(libsb_h)
	gcc $(CFLAGS) libusb.c -lusb-1.0 -o $@

$(libusb-repo):
	git clone https://github.com/libusb/libusb $@

$(libusb): $(libusb-repo)
	cd $(libusb-repo) && ./configure --prefix=$(root)
	$(MAKE) -C $(libusb-repo) CFLAGS="-g"
	$(MAKE) -C $(libusb-repo) install

run-gdb: $(CURDIR)/libusbfind
	gdb --fullname $(CURDIR)/libusbfind

run: $(CURDIR)/libusbfind
	$(CURDIR)/libusbfind
