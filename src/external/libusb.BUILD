# Description:
#   This library provides us with abstractions for working with USB devices

cc_library(
    name = "libusb",
    srcs = glob(["usr/lib/libusb*.so",
                 "usr/lib/x86_64-linux-gnu/libusb*.so"]),
    hdrs = glob(["usr/include/libusb-1.0/*",
                 "usr/local/include/libusb-1.0/*"]),
    includes = ["usr/include/libusb-1.0",
                "usr/local/include/libusb-1.0"],
    visibility = ["//visibility:public"],
)
