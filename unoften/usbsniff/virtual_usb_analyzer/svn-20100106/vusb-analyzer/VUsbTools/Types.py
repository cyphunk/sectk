#
# VUsbTools.Types
# Micah Dowty <micah@vmware.com>
#
# This package holds Event and its subclasses,
# the primitives used to exchange USB log information
# between the log parsers, user interface, and decoders.
# It's also a bit of a dumping ground for other tiny
# objects that don't seem to deserve a separate module
# yet: Color, Observable and psyobj.
#
# Copyright (C) 2005-2009 VMware, Inc. Licensed under the MIT
# License, please see the README.txt. All rights reserved.
#

import binascii

try:
    import gnomecanvas
except gnomecanvas:
    print "Warning: You don't have gnome-canvas (or its python bindings) installed."
    print "         The happy timing diagram will be disabled."
    gnomecanvas = None

try:
    from psyco.classes import psyobj
    from psyco import bind as psycoBind
except ImportError:
    print "Warning: psyco not found, install it for a nice speed boost."
    psyobj = object
    psycoBind = lambda _: None


class Color(psyobj):
    """A simple color abstraction, supports linear interpolation.
       We store individual rgba values, as well as a 32-bit packed
       RGBA representation and an html/gdk-style string.
       """
    def __init__(self, r, g, b, a=0xFF):
        self.r = r
        self.g = g
        self.b = b
        self.a = a

        self.gdkString = "#%02X%02X%02X" % (int(self.r + 0.5),
                                            int(self.g + 0.5),
                                            int(self.b + 0.5))

        self.rgba = ((int(self.r + 0.5) << 24) |
                     (int(self.g + 0.5) << 16) |
                     (int(self.b + 0.5) << 8) |
                     int(self.a + 0.5))

    def lerp(self, a, other):
        """For a=0, returns a copy of 'self'. For a=1, returns
           a copy of 'other'. Other values return a new interpolated
           color. Values are clamped to [0,1], so this will not
           extrapolate new colors.
           """
        a = min(1, max(0, a))
        b = 1.0 - a
        return self.__class__(self.r * b + other.r * a,
                              self.g * b + other.g * a,
                              self.b * b + other.b * a)


class Observable(psyobj):
    """A value with an associated set of listener functions that
       are notified on change.
       """
    def __init__(self, default=None):
        self._value = default
        self.observers = []

    def setValue(self, value, quiet=()):
        self._value = value
        for f in self.observers:
            if f not in quiet:
                f(value)

    def getValue(self):
        return self._value

    value = property(getValue, setValue)


class Event(psyobj):
    """A generic USB log event, containing only timestamp information"""
    def __init__(self, timestamp=None, frame=None, lineNumber=None):
        self.timestamp = timestamp
        self.frame = frame
        self.lineNumber = lineNumber


def hexDump(data, width=16, ascii=True, addrs=True, lineLimit=-1,
            asciiTable = '.' * 32 + ''.join(map(chr, range(32, 127))) + '.' * 129
            ):
    """Create a hex dump of the provided string. Optionally
       prefixes each line with an address, and appends to each
       line an ASCII dump. If 'lines' is specified, limits the
       number of lines this will generate before returning.
       """
    results = []
    addr = 0
    while data and lineLimit != 0:
        if results:
            results.append("\n")
        l, data = data[:width], data[width:]
        if addrs:
            results.append("%04X: " % addr)
        results.append(' '.join(["%02X" % ord(c) for c in l]))
        if ascii:
            results.append(' '.join(["  " for i in xrange(len(l), width + 1)]))
            results.append(l.translate(asciiTable))
        lineLimit -= 1
        addr += len(l)
    return ''.join(results)


class Transaction(Event):
    """A container for USB transaction data"""
    dir = None
    endpt = None
    dev = None
    data = ''
    datalen = 0
    decoded = ''
    decodedSummary = ''

    def appendHexData(self, data):
        """Append data to this packet, given as a whitespace-separated
           string of hexadecimal bytes.
           """
        self.data += binascii.a2b_hex(data.replace(' ', ''))

        # Increase datalen if we need to. Since the log might not
        # include complete data captures while it does include the
        # correct length, this will never decrease datalen.
        datalen = len(self.data)
        if self.hasSetupData():
            datalen -= 8
        self.datalen = max(self.datalen or 0, datalen)

    def appendDecoded(self, line):
        """Append one line of decoder information"""
        if self.decoded:
            self.decoded += "\n" + line
        else:
            self.decodedSummary = line
            self.decoded = line

    def pushDecoded(self, line):
        """Prepend a line of decoder info to the buffer. The new line
           will always become this transaction's decode summary.
           """
        if self.decoded:
            self.decoded = line + "\n" + self.decoded
        else:
            self.decoded = line
        self.decodedSummary = line

    def getTransferString(self):
        """Return a string naming the endpoint and direction of the transfer"""
        if self.endpt is None:
            return "None"
        elif self.endpt == 0:
            # We don't get separate IN/OUT endpoints for EP0,
            # make this clear to the user.
            return "EP0"
        elif self.endpt & 0x80:
            return "EP%d IN" % (self.endpt & 0x7F)
        else:
            return "EP%d OUT" % self.endpt

    def hasSetupData(self):
        return self.endpt == 0

    def getHexSetup(self):
        """Return a hex dump of this transaction's SETUP packet"""
        if self.hasSetupData():
            return hexDump(self.data[:8], addrs=False, ascii=False)

    def getHexDump(self, summarize=False):
        """Return a hex dump of this transaction's data, not including
           SETUP. If 'summarize' is true, this returns only the first
           line, without addresses.
           """
        if self.hasSetupData():
            data = self.data[8:]
        else:
            data = self.data
        if summarize:
            return hexDump(data, addrs=False, lineLimit=1)
        else:
            return hexDump(data)

    def isDataTransaction(self):
        """Returns True if this transaction should have a useful data stage.
           Normally it's true for Down transactions on OUT endpoints,
           and Up transactions on IN endpoints. On the control pipe, we have
           to check the setup packet.
           """
        if self.data and self.endpt == 0:
            dir = ord(self.data[0])
        else:
            dir = self.endpt

        if dir & 0x80:
            return self.dir == 'Up'
        else:
            return self.dir == 'Down'

    def getDiffSummary(self):
        """Returns a tuple which is used to compare Events during a diff operation.
           This should include details which are easy to compare, and identify
           a transaction uniquely among multiple data capture sessions.
           """
        if self.isDataTransaction():
            datalen = self.datalen
        else:
            datalen = None
        return (self.dir, self.endpt, datalen, self.data[:32])


class SOFMarker(Event):
    """An event marking the beginning of a frame"""
    pass


class DiffMarker(Event):
    """An event containing a contiguous list of other events that
       are part of a diff match. The event list is guaranteed to be
       sorted in chronological order, and it must be non-empty.

       'matchedWith' is a parallel list of events that were matched
       in the view we're diffing against.
       """
    def __init__(self, matches, matchedWith):
        self.matches = matches
        self.matchedWith = matchedWith
        Event.__init__(self, timestamp=matches[0].timestamp)
