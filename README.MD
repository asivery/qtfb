# QTFB

**Archived repository: QTFB has been merged into [appload](https://github.com/asivery/rm-appload)**

An implementation of a QT-based "framebuffer" (in reality it's just a QPainter drawing a shared memory region)

Example applications and clients are available in the `example-apps` and `clients` directories.

## Versions

- The root of this repository contains QTFB's PC emulator, useful for testing out extensions which rely on it
- The `xovi` directory contains the version which can be used in conjunction with [xovi](https://github.com/asivery/xovi) on the rMPP tablet.
- The `appload-test-ext` directory is home to the version of qtfb which can be used as a `testing_extensions` module for the [AppLoad](https://github.com/asivery/rmpp-appload) PC emulator. It is useful for writing native AppLoad applications which sometimes utilize QTFB's Framebuffer functionality.

## QML

To add a QTFB object in QML, start by importing `net.asivery.Framebuffer 1.0`. Then create an `FBController` object.
Such an object has the following properties:

- framebufferID (required) - the ID of the framebuffer. This value will be used by the client to define which framebuffer it wants to access
- allowScaling (optional; default: false) - whether or not the image should be scaled to fit the parent object

