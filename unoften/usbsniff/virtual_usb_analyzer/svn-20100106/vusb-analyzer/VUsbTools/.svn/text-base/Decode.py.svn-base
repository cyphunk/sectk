#
# VUsbTools.Decode
# Micah Dowty <micah@vmware.com>
#
# Implements decoders and decoder support for vusb-analyzer.
#
# Copyright (C) 2005-2009 VMware, Inc. Licensed under the MIT
# License, please see the README.txt. All rights reserved.
#

import struct, os
from VUsbTools import Types, Struct


class DecoderContext:
    """This object represents the information we have about a device
       or interface when looking for a particular decoder.
       The 'device' descriptor will always be valid. When searching
       for an EP0 decoder, interface and endpoint will be None.
       When searching for an endpoint decoder, they will be valid.

       'descriptors' is a list of other descriptors being processed
       at the same time. This can be used, for example, for a control
       decoder to peek into a device's endpoint list to apply heuristics.

       'devInstance' will always be our Device object.

       This object exists so that new search parameters can be added
       without modifying the decoders themselves.
       """
    def __init__(self, devInstance=None,
                 device=None, interface=None,
                 endpoint=None, descriptors=None):
        self.devInstance = devInstance
        self.device = device
        self.interface = interface
        self.endpoint = endpoint
        self.descriptors = descriptors


class DecoderFactory:
    """Devices use this object to attach new decoders to their
       endpoints. Decoder modules register detector functions here.
       """
    def __init__(self):
        self._detectors = []

    def register(self, detector):
        self._detectors.append(detector)

    def registerModules(self, parentModule):
        """Automatically register all modules found within the supplied parent module"""
        path = os.path.split(parentModule.__file__)[0]
        for name in os.listdir(path):
            if name.lower().endswith(".py") and name[0] not in ('_', '.'):
                name = name[:-3]
                __import__("%s.%s" % (parentModule.__name__, name))

                print "Loaded decoder module %r" % name
                self.register(getattr(parentModule, name).detector)

    def getDecoder(self, context):
        """Return an appropriate decoder object, given the DecoderContext.
           May return None to indicate that no decoder should be used.
           """
        for detector in self._detectors:
            decoder = detector(context)
            if decoder:
                print "Installing decoder %s.%s" % (
                    decoder.__module__, decoder.__class__.__name__)
                return decoder

        if not context.endpoint:
            return ControlDecoder(context.devInstance)


class Bus:
    """Represents the set of all devices. This receives events,
       and dispatches them to individual devices, creating them
       as necessary.
       """
    def __init__(self):
        self.devices = {}
        self.decoders = DecoderFactory()

    def handleEvent(self, event):
        if not isinstance(event, Types.Transaction):
            return
        if event.dev not in self.devices:
            self.devices[event.dev] = Device(self.decoders)
        self.devices[event.dev].handleEvent(event)


class Device:
    """Represents one device on the bus. A separate decoder is
       registered for each endpoint on each interface.
       This queries the provided DecoderFactory when new descriptors
       are seen, in order to associate new decoders with those descriptors.
       """
    def __init__(self, decoderFactory):
        self.decoderFactory = decoderFactory
        self.currentConfig = 1

        # Current altsetting numbers for each interface
        self.altSettings = {}

        # { configValue: (configDescriptor,
        #     { (interfaceNumber, altSetting): (interfaceDescriptor,
        #         { endpointAddress: (endpointDescriptor, decoder) })
        #     })
        # }
        self.configs = {}

        self.deviceDescriptor = None
        self.controlDecoder = self.decoderFactory.getDecoder(DecoderContext(self))
        self._cacheEndpointDecoders()

    def setInterface(self, iface, alt):
        self.altSettings[iface] = alt
        self._cacheEndpointDecoders()

    def setConfig(self, config):
        self.currentConfig = config
        self._cacheEndpointDecoders()

    def storeDescriptors(self, descriptors):
        iface = alt = config = None

        for desc in descriptors:
            if desc.type == 'device':
                self.deviceDescriptor = desc

            elif desc.type == 'config':
                config = desc.bConfigurationValue
                self.configs[config] = (desc, {})
                self.controlDecoder = self.decoderFactory.getDecoder(DecoderContext(
                    self,
                    device = self.deviceDescriptor,
                    descriptors = descriptors,
                    ))

            elif desc.type == 'interface' and config is not None:
                iface = desc.bInterfaceNumber
                alt = desc.bAlternateSetting
                self.configs[config][1][iface, alt] = (desc, {})

            elif desc.type == 'endpoint' and config is not None and iface is not None:
                self.configs[config][1][iface, alt][1][desc.bEndpointAddress] = (
                    desc, self.decoderFactory.getDecoder(DecoderContext(
                    self,
                    device = self.deviceDescriptor,
                    interface = self.configs[config][1][iface, alt][0],
                    endpoint = desc,
                    descriptors = descriptors,
                    )))

        self._cacheEndpointDecoders()

    def _cacheEndpointDecoders(self):
        """Update the endpointDecoders dictionary using the current configuration
           and interface settings.
           """
        self.endpointDecoders = {0: self.controlDecoder}

        try:
            interfaces = self.configs[self.currentConfig][1]
        except KeyError:
            return
        for (iface, alt), (descriptor, endpoints) in interfaces.iteritems():
            if self.altSettings.get(iface, 0) != alt:
                continue

            # We found an active altsetting for this interface.
            # Activate all its endpoint decoders.
            for address, (descriptor, decoder) in endpoints.iteritems():
                if decoder:
                    self.endpointDecoders[address] = decoder

    def handleEvent(self, event):
        if event.endpt in self.endpointDecoders:
            self.endpointDecoders[event.endpt].handleEvent(event)


class SetupPacket:
    """Represents the SETUP stage of a control transfer.
       This is responsible for decoding the bitfields and
       structure of the packet, but not for defining the meaning
       of a particular request.
       """
    _recipNames = Struct.EnumDict({
        0x00: 'device',
        0x01: 'interface',
        0x02: 'endpoint',
        0x03: 'other',
        })
    _typeNames = Struct.EnumDict({
        0x00: 'standard',
        0x20: 'class',
        0x40: 'vendor',
        0x60: 'reserved',
        })

    def __init__(self, event):
        self.event = event

        (self.bitmap, self.request, self.wValue,
         self.wIndex, self.wLength) = struct.unpack("<BBHHH", event.data[:8])

        # Decode self.bitmap
        self.recip = self._recipNames[self.bitmap & 0x1F]
        self.type = self._typeNames[self.bitmap & 0x60]

        # Split up wValue/wIndex for convenience
        self.wValueHigh = self.wValue >> 8
        self.wValueLow = self.wValue & 0xFF
        self.wIndexHigh = self.wIndex >> 8
        self.wIndexLow = self.wIndex & 0xFF


class DescriptorGroup(Struct.Group):
    """Parses out USB descriptors into a Struct.Group tree.
       This class handles any standard descriptor, but
       subclasses can add class-specific descriptors as needed.
       """
    descriptorTypes = Struct.EnumDict({
        0x01: "device",
        0x02: "config",
        0x03: "string",
        0x04: "interface",
        0x05: "endpoint",
        })

    headerStruct = lambda self: (
        Struct.UInt8("bLength"),
        Struct.UInt8("bDescriptorType"),
        )
    struct_device = lambda self: (
        Struct.UInt16Hex("bcdUSB"),
        Struct.UInt8Hex("bDeviceClass"),
        Struct.UInt8Hex("bDeviceSubClass"),
        Struct.UInt8Hex("bDeviceProtocol"),
        Struct.UInt8("bMaxPacketSize0"),
        Struct.UInt16Hex("idVendor"),
        Struct.UInt16Hex("idProduct"),
        Struct.UInt16Hex("bcdDevice"),
        Struct.UInt8("iManufacturer"),
        Struct.UInt8("iProduct"),
        Struct.UInt8("iSerialNumber"),
        Struct.UInt8("bNumConfigurations"),
        )
    struct_config = lambda self: (
        Struct.UInt16("wTotalLength"),
        Struct.UInt8("bNumInterfaces"),
        Struct.UInt8("bConfigurationValue"),
        Struct.UInt8("iConfiguration"),
        Struct.UInt8Hex("bmAttributes"),
        Struct.UInt8("MaxPower"),
        )
    struct_string = lambda self: (
        Struct.Utf16String("string"),
        )
    struct_interface = lambda self: (
        Struct.UInt8("bInterfaceNumber"),
        Struct.UInt8("bAlternateSetting"),
        Struct.UInt8("bNumEndpoints"),
        Struct.UInt8Hex("bInterfaceClass"),
        Struct.UInt8Hex("bInterfaceSubClass"),
        Struct.UInt8Hex("bInterfaceProtocol"),
        Struct.UInt8("iInterface"),
        )
    struct_endpoint = lambda self: (
        Struct.UInt8Hex("bEndpointAddress"),
        Struct.UInt8Hex("bmAttributes"),
        Struct.UInt16("wMaxPacketSize"),
        Struct.UInt8("bInterval"),
        )

    def __init__(self):
        Struct.Group.__init__(self, "descriptors")

    def decode(self, buffer):
        # Common descriptor header
        buffer = Struct.Group.decode(self, buffer, self.headerStruct())

        # Decode descriptor type
        self.type = self.descriptorTypes[self.bDescriptorType]

        # Make sure that we eat exactly the right number of bytes,
        # according to the descriptor header
        descriptor = buffer[:self.bLength - 2]
        buffer = buffer[self.bLength - 2:]

        # The rest of the decoding is done by a hander, in the form of a child item list.
        Struct.Group.decode(self, descriptor,
                            getattr(self, "struct_%s" % self.type, lambda: None)())
        return buffer


class ControlDecoder:
    """Decodes standard control requests, and tries to detect
       additional class decoders based on descriptors we see.

       Control requests first have their names looked up in
       a table of the form <type>Requests. They may then go to
       a default decoder, or we may look up one of the form
       decode_<name> from this class.

       This class defines all standard requests. Subclasses
       may define extra class- or vendor-specific requests.
       """
    standardRequests = Struct.EnumDict({
        0x00: 'GetStatus',
        0x01: 'ClearFeature',
        0x03: 'SetFeature',
        0x05: 'SetAddress',
        0x06: 'GetDescriptor',
        0x07: 'SetDescriptor',
        0x08: 'GetConfiguration',
        0x09: 'SetConfiguration',
        0x0A: 'GetInterface',
        0x0B: 'SetInterface',
        0x0C: 'SynchFrame',
        })
    standardFeatures = Struct.EnumDict({
        0x00: 'ENDPOINT_HALT',
        0x01: 'DEVICE_REMOTE_WAKEUP',
        })
    descriptorClass = DescriptorGroup

    def __init__(self, device):
        self.device = device

    def handleEvent(self, event):
        if not event.isDataTransaction():
            return
        setup = SetupPacket(event)

        # Look up the request name
        setup.requestName = getattr(self, "%sRequests" % setup.type,
                                    Struct.EnumDict())[setup.request]

        # Look up a corresponding decoder
        getattr(self, "decode_%s" % setup.requestName, self.decodeGeneric)(setup)

    def decodeGeneric(self, setup):
        """Generic decoder for control requests"""
        setup.event.pushDecoded("%s %s %s(wValue=0x%04x, wIndex=0x%04x)" % (
            setup.type, setup.recip, setup.requestName,
            setup.wValue, setup.wIndex))

    def decode_SetAddress(self, setup):
        setup.event.pushDecoded("SetAddress(%d)" % setup.wValue)

    def decode_SetConfiguration(self, setup):
        setup.event.pushDecoded("SetConfiguration(%d)" % setup.wValue)
        self.device.setConfig(setup.wValue)

    def decode_SetDescriptor(self, setup):
        # Display the get/set descriptor request itself
        setup.event.pushDecoded("%s(%s, %s%s)" % (
            setup.requestName,
            self.descriptorClass.descriptorTypes[setup.wValueHigh],
            setup.wValueLow,
            ("", ", lang=%04X" % setup.wIndex)[setup.wIndex != 0],
            ))

        # Parse out all descriptors
        descriptors = []
        buffer = setup.event.data[8:]
        while buffer:
            desc = self.descriptorClass()
            buffer = desc.decode(buffer)
            setup.event.appendDecoded("\n%s descriptor:\n%s" % (desc.type, desc))
            descriptors.append(desc)

        self.device.storeDescriptors(descriptors)

    decode_GetDescriptor = decode_SetDescriptor

    def decode_SetFeature(self, setup):
        feature = getattr(self, "%sFeatures" % setup.type,
                          Struct.EnumDict())[setup.wValue]
        setup.event.pushDecoded("%s %s(%s, %s=0x%02x)" % (
            setup.type, setup.requestName, feature,
            setup.recip, setup.wIndex))

    decode_ClearFeature = decode_SetFeature

    def decode_SetInterface(self, setup):
        setup.event.pushDecoded("SetInterface(alt=%d, iface=%d)" %
                                  (setup.wValue, setup.wIndex))
        self.device.setInterface(setup.wIndex, setup.wValue)


def attachView(viewContainer):
    """Add decoding capabilities to a ViewContainer. """
    from VUsbTools import Decoders
    bus = Bus()
    viewContainer.children.insert(0, bus)
    bus.decoders.registerModules(Decoders)
