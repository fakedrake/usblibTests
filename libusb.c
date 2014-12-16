#include <stdio.h>
#include <libusb.h>

#define USBTINY_VENDOR_DEFAULT  0x1781
#define USBTINY_PRODUCT_DEFAULT 0x0C9F

#define MAY_FAIL(expr) do {                                     \
        if ((errno=(expr)) != LIBUSB_SUCCESS) {                 \
            fprintf(stderr, "usblib( " #expr " ): %s (%d)\n",   \
                    libusb_error_name(errno), errno);           \
            return errno;                                       \
        }                                                       \
    } while(0)

char match_dev(libusb_device * dev, int vid, int pid)
{
    struct libusb_device_descriptor desc;
    libusb_get_device_descriptor(dev, &desc);
    printf("Matching 0x%x:0x%x\n", desc.idVendor, desc.idProduct);

    return (desc.idVendor == vid && desc.idProduct == pid);
}

int main(int argc, char *argv[])
{
    int errno = 0, i, cnt;
    libusb_device_handle* handle;
    libusb_context* usb_context;
    struct libusb_config_descriptor *config;
    libusb_device **devs;
    libusb_device * dev;

    printf("Setting up\n");

    MAY_FAIL(libusb_init(&usb_context));
    libusb_set_debug(usb_context, 4);
    printf("Looking for devices\n");
    cnt=libusb_get_device_list(usb_context, &devs);
    printf("Found %d devices\n", cnt);
    for (i=0; i<cnt; i++) {
        if (!match_dev(devs[i], USBTINY_VENDOR_DEFAULT, USBTINY_PRODUCT_DEFAULT))
            continue;

        dev = devs[i];
        MAY_FAIL(libusb_open(devs[i], &handle));

        /* dev->os_priv->dev (darwin_cached_device) ->active_config */
        /* It is set with darwin_set_configuration */
        MAY_FAIL(libusb_get_active_config_descriptor(devs[i], &config));
        libusb_free_config_descriptor(config);
        libusb_close(handle);
    }

    fprintf(stderr, "No device found");
    libusb_exit(usb_context);
    return 1;
}
