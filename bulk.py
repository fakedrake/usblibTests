#!/usr/bin/env python

import usb1

usbasp = {"vid": 0x16c0, "pid": 0x05dc}
usbtiny = {"vid": 0x1781, "pid": 0x0C9F}
mkii = {"vid": 0x03eb, "pid": 0x2104}

context = usb1.USBContext()
ghandle = None

def handle(dev):
    global ghandle

    if not ghandle:
        ghandle = context.openByVendorIDAndProductID(
            dev['vid'], dev['pid'],
            skip_on_error=True)

    return ghandle

def bulkWriteThenRead(dev):
    hndl = handle(dev)
    if not hndl:
        return hndl

    cfg = next(hndl.getDevice().iterConfigurations())
    iface = next(cfg.iterInterfaces())
    # Not claiming the interface causes LIBUSB_ERROR_IO on read
    hndl.claimInterface(0)
    setting = next(iface.iterSettings())
    ep = next(setting.iterEndpoints())
    hndl.bulkWrite(ep.getAddress(), '\x01')
    return hndl.bulkRead(ep.getAddress(), 64)

if __name__ == '__main__':
    data = bulkWriteThenRead(mkii)
    print "Wrote [0x01] and got '%s'" % repr(data)
