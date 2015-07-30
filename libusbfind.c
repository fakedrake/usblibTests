#include <stdio.h>
#include <libusb.h>

#define USBTINY_VENDOR_DEFAULT  0x1781
#define USBTINY_PRODUCT_DEFAULT 0x0C9F

struct dev {
    unsigned short vid, pid;
} devices[] = {
    {0x16c0, 0x05dc},           /* USBASP */
    {0x1781, 0x0C9F},           /* USBTinyISP */
    {0x03eb, 0x2104},           /* AVRISP mkII */
    {0x067b, 0x2303},           /* Some serial thingie */
    {0x0,0x0}
};


#define MAY_FAIL(expr) do {                                     \
        if ((errno=(expr)) != LIBUSB_SUCCESS) {                 \
            fprintf(stderr, "usblib( " #expr " ): %s (%d)\n",   \
                    libusb_error_name(errno), errno);           \
            return errno;                                       \
        }                                                       \
    } while(0)

char match_dev(struct libusb_device_descriptor* desc)
{
    printf("Matching 0x%x:0x%x\n", desc->idVendor, desc->idProduct);
    struct dev *cdev = devices;

    do {
        if (desc->idVendor == cdev->vid &&
            desc->idProduct == cdev->pid)
            return 1;
    } while(*(unsigned int*)(cdev++));

    return 0;
}

#define DUMP_ATTR(a) printf("\t" #a " = 0x%x\n", (unsigned int)(a))
void dump_config_descriptor(struct libusb_config_descriptor * config)
{
    DUMP_ATTR(config->bLength);
    DUMP_ATTR(config->bDescriptorType);
    DUMP_ATTR(config->wTotalLength);
    DUMP_ATTR(config->bNumInterfaces);
    DUMP_ATTR(config->bConfigurationValue);
    DUMP_ATTR(config->iConfiguration);
    DUMP_ATTR(config->bmAttributes);
    DUMP_ATTR(config->MaxPower);
    DUMP_ATTR(config->interface);
    DUMP_ATTR(config->extra);
    DUMP_ATTR(config->extra_length);
}

int action (libusb_context *ctx, struct libusb_device_handle* handle,
            struct libusb_config_descriptor *config)
{
    unsigned char data[] = {1};
    int* transferred ;
    const struct libusb_interface_descriptor * altsetting = config->interface->altsetting;
    printf("Altsetting found %p, %d\n", altsetting,
           config->interface->num_altsetting);
    const struct libusb_endpoint_descriptor* ep = altsetting->endpoint;
    printf("Endpoint found\n");
    int ret = libusb_bulk_transfer(handle, ep->bEndpointAddress, data, sizeof(data), transferred, 0);
    printf("BulkRet: %d\n", ret);
    switch (ret) {
        case (LIBUSB_ERROR_TIMEOUT):
                fprintf(stderr, "USBERROR: LIBUSB_ERROR_TIMEOUT\n");
                return ret;
        case (LIBUSB_ERROR_PIPE):
                fprintf(stderr, "USBERROR: LIBUSB_ERROR_PIPE\n");
                return ret;
        case (LIBUSB_ERROR_OVERFLOW):
                fprintf(stderr, "USBERROR: LIBUSB_ERROR_OVERFLOW\n");
                return ret;
        case (LIBUSB_ERROR_NO_DEVICE):
                fprintf(stderr, "USBERROR: LIBUSB_ERROR_NO_DEVICE\n");
                return ret;
        case (LIBUSB_ERROR_IO):
                fprintf(stderr, "USBERROR: LIBUSB_ERROR\n");
                return ret;
        default: return ret;
    }
}

int main(int argc, char *argv[])
{
    int errno = 1, i, cnt;
    libusb_device_handle* handle = NULL;
    libusb_context* usb_context = NULL;
    struct libusb_config_descriptor *config = NULL;
    struct libusb_device_descriptor desc;

    libusb_device **devs = NULL;

    printf("Setting up\n");
    MAY_FAIL(libusb_init(&usb_context));
    // libusb_set_debug(usb_context, 4);
    printf("Looking for devices\n");
    cnt = libusb_get_device_list(usb_context, &devs);
    printf("Found %d devices\n", cnt);
    for (i=0; i<cnt; i++) {
        libusb_get_device_descriptor(devs[i], &desc);
        printf("DEV: vendor: 0x%x(%d), product:0x%x(%d)\n",
               desc.idVendor, desc.idVendor, desc.idProduct,
               desc.idProduct);

        if (!match_dev(&desc))
            continue;

        MAY_FAIL(libusb_open(devs[i], &handle));

        MAY_FAIL(libusb_set_configuration(handle, 1));
        /* dev->os_priv->dev (darwin_cached_device) ->active_config */
        /* It is set with darwin_set_configuration */
        MAY_FAIL(libusb_get_active_config_descriptor(devs[i], &config));
        dump_config_descriptor(config);
        libusb_free_config_descriptor(config);
        libusb_close(handle);
        printf("Found a good device! %04x:%04x\n",
               desc.idVendor, desc.idProduct);
        errno = action(usb_context, handle, config);
        break;
    }
    libusb_exit(usb_context);
    if (errno == 1) fprintf(stderr, "No known device found!\n");
    return errno;
}
