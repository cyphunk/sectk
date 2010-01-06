#
# VUsbTools.Views
# Micah Dowty <micah@vmware.com>
#
# A container for color and font preferences
#
# Copyright (C) 2005-2009 VMware, Inc. Licensed under the MIT
# License, please see the README.txt. All rights reserved.
#

import math
from VUsbTools.Types import Color

#
# Use the default monospace font.  If this is too large/small for your
# tastes, you can try a specific font name and size like "courier 9".
#
monospaceFont = "monospace"

def toMonospaceMarkup(text):
    """Convert arbitrary text to pango markup in our monospace font"""
    return '<span font_desc="%s">%s</span>' % (
        monospaceFont,
        text.replace("&", "&amp;").replace("<", "&lt;"))

directionColors = {
    "Up":   Color(0x00, 0x00, 0xFF),
    "Down": Color(0x00, 0x80, 0x00),
    }

directionIcons = {
    "Down": "gtk-go-forward",
    "Up":   "gtk-go-back",
    }

errorMarkerColor = Color(0xFF, 0x00, 0x00, 0x80)
diffMarkerColor = Color(0x00, 0xA0, 0x00, 0x30)
diffBorderColor = Color(0x00, 0xA0, 0x00, 0xA0)
emptyTransactionColor = Color(0x80, 0x80, 0x80)
smallTransactionColor = Color(0x80, 0x80, 0xFF)
largeTransactionColor = Color(0xFF, 0xFF, 0x80)
frameMarkerColor = Color(0x00, 0x00, 0xFF)
duplicateFrameColor = Color(0x80, 0x00, 0x00)

def getBarColor(transaction):
    """Get the color to use for a transaction's bar on the timing diagram.
    This implementation bases the color on a transaction's size.
    """
    s = transaction.datalen or 0
    if not s:
        return emptyTransactionColor

    # For non-empty transactions, the color is actually proportional
    # to size on a logarithmic scale.
    return smallTransactionColor.lerp(
        math.log(s) / math.log(4096), largeTransactionColor)
