#
# VUsbTools.Struct
# Micah Dowty <micah@vmware.com>
#
# Utilities for decoding structures and trees of structures
# into something human-readable.
#
# Copyright (C) 2005-2009 VMware, Inc. Licensed under the MIT
# License, please see the README.txt. All rights reserved.
#

import struct
from VUsbTools import Types


class EnumDict(dict):
    """A dictionary mapping assigned numbers to names.
       Currently the only difference between this and a normal
       dictionary is that unknown keys 'pass through'.
       """
    format = "0x%02x"

    def __getitem__(self, key):
        try:
            return dict.__getitem__(self, key)
        except:
            try:
                return self.format % key
            except TypeError:
                return str(key)


class Item:
    """This is an abstract base class for items that
       can be decoded from an arbitrary string of bytes.
       After decoding, the 'value' attribute should be
       valid. str() should return the most meaningful
       human-readable representation.
       """
    _strFormat = "%s"
    _format = None

    def __init__(self, name):
        self._name = name

    def decode(self, buffer):
        """The default decoder makes use of struct"""
        size = struct.calcsize(self._format)
        item = buffer[:size]
        buffer = buffer[size:]
        if len(item) != size:
            self._value = None
        else:
            self._value = struct.unpack(self._format, item)[0]
        return buffer

    def __str__(self):
        if self._value is None:
            return "None"
        else:
            return self._strFormat % self._value

class UInt8(Item):
    _format = "<B"

class UInt16(Item):
    _format = "<H"

class UInt32(Item):
    _format = "<I"

class UInt16BE(Item):
    _format = ">H"

class UInt32BE(Item):
    _format = ">I"

class UInt8Hex(UInt8):
    _strFormat = "0x%02X"

class UInt16Hex(UInt16):
    _strFormat = "0x%04X"

class UInt32Hex(UInt32):
    _strFormat = "0x%08X"

class Utf16String(Item):
    def decode(self, buffer):
        l = len(buffer) & ~1
        self._value = unicode(buffer[:l], 'utf16')
        return buffer[l:]

    def __str__(self):
        return repr(self._value)[1:]


class Group(Item):
    """An item built from several child items. During decoding,
       a Group will receive attributes containing the values of
       all children.

       Children can all be defined up-front, or they can be
       done during decoding (for structures whose content depends
       on header values)
       """
    def __init__(self, name, *children):
        Item.__init__(self, name)
        self._childTemplate = children
        self._children = []

    def decode(self, buffer, children=None):
        # This can optionally be called directly to override childTemplate.
        for child in children or self._childTemplate:
            self._children.append(child)
            buffer = child.decode(buffer)
            try:
                setattr(self, child._name, child._value)
            except AttributeError:
                setattr(self, child._name, child)
        return buffer

    def _iterChildren(self):
        """Walk among all children recursively, yielding
           (path, item) tuples, where path is a tuple of names.
           """
        for child in self._children:
            if isinstance(child, Group):
                for path, grandchild in child._iterChildren():
                    yield (child._name,) + path, grandchild
            else:
                yield (child._name,), child

    def __str__(self):
        lines = []
        pathMax = 0
        for path, item in self._iterChildren():
            path = ".".join(path)
            pathMax = max(pathMax, len(path))
            lines.append((path, item))
        return "\n".join([ "  %-*s = %s" % (pathMax, p, i) for p, i in lines ])
