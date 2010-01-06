#
# VUsbTools.Views
# Micah Dowty <micah@vmware.com>
#
# This implements the GTK+ user interface
# for vusb-analyzer, with optional support
# for gnomecanvas.
#
# Copyright (C) 2005-2009 VMware, Inc. Licensed under the MIT
# License, please see the README.txt. All rights reserved.
#

from __future__ import division
import math, Queue, gtk, gobject
from VUsbTools import Types, Style

try:
    import gnomecanvas
except gnomecanvas:
    print "Warning: You don't have gnome-canvas (or its python bindings) installed."
    print "         The happy timing diagram will be disabled."
    gnomecanvas = None


class ViewContainer:
    """A parent for several views. This holds the common hilight
       object, and broadcasts events to all views.
       """
    def __init__(self):
        self.hilight = Types.Observable()
        self.children = []

    def handleEvent(self, event):
        """Add an event to all views"""
        for child in self.children:
            child.handleEvent(event)


class View:
    """Abstract base class for views which listen to the incoming
       event stream. Each view, besides getting a copy of
       the events, gets to listen to and control the common
       hilight.
       """
    def __init__(self, container):
        self.hilight = container.hilight
        self.hilight.observers.append(self.onHilightChanged)
        self.root = self.createWidgets()
        container.children.append(self)

    def createWidgets(self):
        """Subclasses must implement this to create their GTK widgets.
           This returns the root of this view's widget tree.
           """
        raise NotImplementedError

    def handleEvent(self, event):
        """Subclasses may implement this to receive event data"""
        pass

    def onHilightChanged(self, newHilight):
        """Called when the hilight has been changed by another view"""
        pass

    def notifyHilightChanged(self, newHilight):
        """Subclasses call this to notify other views that the hilight has changed"""
        self.hilight.setValue(newHilight, (self.onHilightChanged,))



class CanvasOwner(Types.psyobj):
    """Helper class for objects that own a canvas"""
    width = 1
    height = 1
    expand = (0, 1)

    def __init__(self):
        self.canvas = gnomecanvas.Canvas(aa=True)

    def resize(self, width, height):
        width = int(width + 0.5)
        height = int(height + 0.5)
        self.width = width
        self.height = height
        self.canvas.set_size_request(self.expand[0] * width,
                                     self.expand[1] * height)
        self.canvas.set_scroll_region(0, 0, width, height)


class Ruler(CanvasOwner):
    """A ruler marking time divisions horizontally, using a GnomeCanvas"""
    timeExtent = 0
    width = 0
    absoluteFrame = None
    lastFrame = None
    nextMarkedFrame = None

    # Constants describing the scale's shape
    tickScale = (1.0, 20.0)
    divisions = ((1, 1),
                 (10, 2),
                 (20, 4),
                 (100, 6))
    textOffset = (5.5, 5.5)
    height = 30

    def __init__(self, scale=512, initialExtent=10.0):
        CanvasOwner.__init__(self)
        self.scale = scale
        self.resizer = Resizer(self)
        self.resizeCallbacks = []
        self.frameGroup = self.canvas.root().add(gnomecanvas.CanvasGroup)
        self.extend(initialExtent)

    def drawSecond(self, s):
        """Draw one second worth of tick marks and such on the scale"""
        group = self.canvas.root().add(gnomecanvas.CanvasGroup)

        w = group.add(gnomecanvas.CanvasText,
                      anchor = gtk.ANCHOR_NW,
                      text = "%.01fs" % s,
                      fill_color = 'black',
                      font = 'sans')
        self.resizer.track(w, x=(s, 5.5), y=(0, 5.5))

        for count, scale in self.divisions:
            h = self.tickScale[1] / scale
            w = self.tickScale[0] / scale
            for i in xrange(0, count):
                fraction = i / count
                self.resizer.track(group.add(gnomecanvas.CanvasRect,
                                             y1 = 0, y2 = h,
                                             fill_color = 'black'),
                                   x1=(s+fraction, -w),
                                   x2=(s+fraction, w))

    def extend(self, s):
        """Extend the scale to hold at least 's' seconds"""
        newTimeExtent = int(s + 1 - 1e-10)
        if newTimeExtent > self.timeExtent:
            for s in xrange(self.timeExtent , newTimeExtent):
                self.drawSecond(s)
            self.timeExtent = newTimeExtent
            self.resize(newTimeExtent * self.scale, self.height)
            for f in self.resizeCallbacks:
                f()

    def zoom(self, factor):
        """Zoom in/out by the provided factor, keeping the view centered"""
        adj = self.canvas.get_hadjustment()
        oldCenter = adj.value + adj.page_size // 2

        self.scale *= factor
        self.resizer.rescale()
        self.resize(self.timeExtent * self.scale, self.height)
        for f in self.resizeCallbacks:
            f()

        adj.value = oldCenter * factor - adj.page_size // 2

    def markFrame(self, event):
        """Plot SOF markers on our ruler. We get different sizes of
           markers for one frame, 100 frames, and 1000 frames. The
           users can compare these marks with the real-time ruler
           to assess how fast and how smoothly the virtual frame
           clock is progressing.
           """
        color = Style.frameMarkerColor.gdkString

        if self.absoluteFrame is None:
            # Initialize the system. This is now absolute frame zero,
            # and we're declaring it to be the first marked frame.
            self.absoluteFrame = 0
            self.nextMarkedFrame = [0, 0]
        else:
            # Use the delta in real frame numbers (accounting for rollover)
            # to update our 'absolute' frame number, which never rolls over.
            d = event.frame - self.lastFrame

            if d < 0:
                d += 1024
            elif d == 0:
                # Duplicate frame, mark it in a different color
                color = Style.duplicateFrameColor.gdkString

            self.absoluteFrame += d
        self.lastFrame = event.frame

        # Small marks- default
        h = 3
        w = 0.4

        # Bigger marks every 100 frames
        if self.absoluteFrame > self.nextMarkedFrame[1]:
            self.nextMarkedFrame[1] += 100
            h = 10
            w = 0.5

        # Huge marks every 1000 frames
        if self.absoluteFrame > self.nextMarkedFrame[0]:
            self.nextMarkedFrame[0] += 1000
            h = 25
            w = 0.75

        self.resizer.track(self.frameGroup.add(gnomecanvas.CanvasRect,
                                               y1 = self.height - h,
                                               y2 = self.height,
                                               fill_color = color),
                           x1=(event.timestamp, -w),
                           x2=(event.timestamp, w))


class TimingRow(Types.psyobj):
    """One row of boxes in the timing diagram. This stores canvas coordinates
       for the row, and keeps a simple collision detection hash.

       The collision detection hash describes blocks of 1-D spacs using booleans.
       The hash keys are (origin, size) tuples, where the size must be a power of two.
       The values are a non-None tag object to indicate a completely occupied block,
       or None to indicate a block that may be partially occupied.
       """
    def __init__(self, top, bottom):
        self.hash = {}
        self.top = top
        self.bottom = bottom

        # Determines the depth of our 'tree', and the number of discrete collision
        # detection steps we see in each second.
        self.quanta = 0x10000

        # This is the largest size we ever record partially-occupied blocks at.
        # Values that are too large just waste space in the hash, values that
        # are too small cause us to spend lots of time scanning the hash for
        # blocks. Must be a power of two.
        #
        # Default scan is 1 second at a time
        self.scanDepth = self.quanta

    def markInterval(self, a, b, tag=True):
        """Mark the entire timespan between A and B as occupied"""

        # Quantize the interval. The quantized numbers can be thought of
        # as a path to a leaf on our BSP tree, each bit representing a
        # decision in the tree.
        i = int(self.quanta * a)
        j = int(self.quanta * b)

        # Mark parents of the endpoints themselves as partially full.
        # We don't need to worry about marking partial blocks below,
        # because all partial blocks we could possibly get occur
        # above the endpoints. (I don't have a good way to explain
        # this without a diagram yet ;)
        for ep in (i, j):
            pl = 1
            while pl < self.scanDepth:
                pl <<= 1
                self.hash[ep & ~(pl - 1), pl] = None

        # If our interval is actually less than one quanta, fudge things
        # a bit and expand it to exactly one quanta. This also gives
        # us a fast path for these common tiny intervals
        if i == j:
            self.hash[i, 1] = tag
            return

        l = 1
        while 1:
            # Pop up one level if we've cleared all the nonzero
            # bits from this level, and it wouldn't cause us to step
            # past the end of our interval.
            while (i & l) == 0 and (i + l) < j and l < self.scanDepth:
                l = l << 1

            # Venture deeper in the tree as we approach the end
            # of the interval, so we don't step past it.
            while (i + l) > j and l:
                l = l >> 1

            # Our stopping condition is when we can't move any
            # further without going past the end of the interval.
            if not l:
                break

            # This block we just found is completely full.
            self.hash[i, l] = tag
            i += l

    def intervalOccupied(self, a, b, scale=1):
        """Return the first tag object found if anyone else has
           marked intervals that overlap the supplied interval.
           Otherwise, returns None.
           """
        i = int(self.quanta * a)
        j = int(self.quanta * b)
        l = self.scanDepth

        while i < j:
            y = self.hash.get((i & ~(l-1), l), False)

            if y is False:
                # Nothing here, align to the beginning of the next block.
                # If we've cleared another zero, move up a level so we
                # can skip this empty space faster.
                i = (i & ~(l-1)) + l
                l2 = l << 1
                if (i & (l2-1)) == 0:
                    l = l2

            elif y is None:
                # This may or may not be a collision, scan deeper
                if l > 1:
                    l = l >> 1

            else:
                # We found a collision, bail out now
                return y


class TimeCursor:
    """Observes a cursor value, updating the position of a marker widget
       attached to an arbitrary CanvasOwner.
       """
    previous = 0

    def __init__(self, canvasOwner, ruler, cursor):
        self.ruler = ruler
        self.cursor = cursor

        self.widget = canvasOwner.canvas.root().add(
            gnomecanvas.CanvasRect,
            x1 = 0, x2 = 1,
            y1 = 0, y2 = 10000,
            fill_color = 'red')

        cursor.observers.append(self.move)
        self.move(cursor.value)

    def move(self, t):
        x = self.ruler.scale * t
        self.widget.move(x - self.previous, 0)
        self.previous = x


class TimingRowStack(CanvasOwner):
    """A stack of TimingRows, rendered in a GnomeCanvas"""
    width = 0
    height = 0
    rowHeight = 10
    rowGap = 5
    allocHeight = None

    def __init__(self, ruler):
        CanvasOwner.__init__(self)
        self.ruler = ruler
        self.rows = []

        self.ruler.resizeCallbacks.append(self.onRulerResized)
        self.onRulerResized()
        self.newRow()

    def extend(self, s):
        """Ensure that our widget can hold at least 's' seconds"""
        self.ruler.extend(s)

    def onRulerResized(self):
        """Called by the ruler when any object resizes it. Change our width to match."""
        self.resize(self.ruler.width, self.height)

    def newRow(self):
        """Allocate a new row unconditionally"""
        row = self.newPrivateRow(self.rowHeight)
        self.rows.append(row)
        return row

    def newPrivateRow(self, height):
        """Allocate a row of specified height, don't store it in self.rows.
           This is used internally, plus it can be used to allocate 'special'
           rows that shouldn't show up in pickRow().
           """
        if self.allocHeight is None:
            top = self.rowGap // 2
        else:
            top = self.allocHeight + self.rowGap
        bottom = top + height

        row = TimingRow(top, bottom)
        self.allocHeight = bottom

        h = bottom + self.rowGap
        if h > self.height:
            self.resize(self.width, h)

        return row

    def pickRow(self, a, b, tag=True):
        """Pick a row that has the specified interval free, then
           allocate that interval and return the row.
           """
        for row in self.rows:
            if not row.intervalOccupied(a, b):
                row.markInterval(a, b, tag)
                return row

        row = self.newRow()
        row.markInterval(a, b, tag)
        return row


class ScrollContainer(gtk.Table):
    """This is similar to gdk.ScrollWindow, but allows any number of
       scroll axes in the horizontal and vertical directions. With the
       default parameters it can be used to provide an alternative to
       gtk.ScrollWindow that doesn't require a viewport. It can also
       be used for more advanced collections of widgets that scroll together.

       Each entry in hAxes/vAxes defines a position in a grid. The boolean
       within indicates whether a scroll bar will be present at that position.
       If not, the adjustment and scroll bar for that position will be None.
       """
    def __init__(self, hAxes=(True,), vAxes=(True,)):
        gtk.Table.__init__(self, len(hAxes)+1, len(vAxes)+1, False)
        self.attach(gtk.Label(),
                    len(hAxes)+1, len(hAxes)+2,
                    len(vAxes)+1, len(vAxes)+2,
                    0, 0)

        self.hAdjust = []
        self.vAdjust = []
        self.hScroll = []
        self.vScroll = []

        for i, enable in enumerate(hAxes):
            if enable:
                adj = gtk.Adjustment()
                self.hAdjust.append(adj)

                scroll = gtk.HScrollbar()
                scroll.set_adjustment(adj)
                self.hScroll.append(scroll)

                self.attach(scroll, i+1, i+2,
                            len(vAxes)+1, len(vAxes)+2,
                            gtk.FILL | gtk.SHRINK, 0)
            else:
                self.hAdjust.append(None)
                self.hScroll.append(None)

        for i, enable in enumerate(vAxes):
            if enable:
                adj = gtk.Adjustment()
                self.vAdjust.append(adj)

                scroll = gtk.VScrollbar()
                scroll.set_adjustment(adj)
                self.vScroll.append(scroll)

                self.attach(scroll, len(hAxes)+1, len(hAxes)+2,
                            i+1, i+2, 0, gtk.FILL | gtk.SHRINK)
            else:
                self.vAdjust.append(None)
                self.vScroll.append(None)

    def add(self, child, x=0, y=0,
            xflags=gtk.FILL | gtk.EXPAND | gtk.SHRINK,
            yflags=gtk.FILL | gtk.EXPAND | gtk.SHRINK):
        self.attach(child, x+1, x+2, y+1, y+2, xflags, yflags)

    def attachEvent(self, widget, axis=0, horizontal=False):
        if horizontal:
            adj = self.hAdjust[axis]
        else:
            adj = self.vAdjust[axis]
        widget.add_events(gtk.gdk.SCROLL_MASK)
        widget.connect('scroll-event', self._scrollEvent, adj)

    def _scrollEvent(self, widget, event, adj):
        """Process a scroll wheel event against the supplied GtkAdjustment"""
        if event.direction in (gtk.gdk.SCROLL_UP, gtk.gdk.SCROLL_LEFT):
            inc = -adj.step_increment
        elif event.direction in (gtk.gdk.SCROLL_DOWN, gtk.gdk.SCROLL_RIGHT):
            inc = adj.step_increment
        else:
            inc = 0
        adj.set_value(min(adj.upper - adj.page_size, adj.value + inc))
        return False


class Resizer:
    """Since we can't seem to trust gnome-canvas to do anything right,
       this object tracks gnome-canvas widgets that depend on the ruler
       scale. When the scale changes, this recalculates their position
       appropriately.
       """
    def __init__(self, ruler):
        self.map = {}
        self.ruler = ruler

    def track(self, widget, **kw):
        self.map[widget] = kw
        s = self.ruler.scale
        for prop, (a, b) in kw.iteritems():
            widget.set_property(prop, a*s + b)

    def rescale(self):
        s = self.ruler.scale
        for widget, kw in self.map.iteritems():
            for prop, (a, b) in kw.iteritems():
                widget.set_property(prop, a*s + b)


class TimingDiagramPipe:
    """Represents one endpoint on one device in the timing diagram.
       Graphically, this is managed by a TimingRowStack. This class
       is responsible for the transaction decoding necessary to
       convert a stream of transactions into canvas widgets within
       a TimingRowStack. Subclasses may perform additional class decoding
       and display that data graphically.
       """
    def __init__(self, view):
        self.view = view
        self.stack = TimingRowStack(self.view.ruler)
        TimeCursor(self.stack, self.view.ruler, self.view.cursor)

        # Each endpoint should always queue transactions in FIFO order.
        # We use this fact to pair up and down transactions- each endpoint
        # gets a queue of 'Down' transactions which we pair up when we get
        # the corresponding 'Up'.
        self.queue = Queue.Queue()

    def handleEvent(self, transaction):
        """Add a single transaction to this pipe. We queue up and
           pair transactions, delivering matched down/up pairs to
           addPair.
           """
        if transaction.dir == 'Down':
            self.queue.put(transaction)
        elif transaction.dir == 'Up':
            try:
                self.addPair(self.queue.get(False), transaction)
            except Queue.Empty:
                print "*** Warning, found an 'Up' transaction with no matching 'Down'."
                print "    This should only occur when reading partial logfiles."

    def getDataTransaction(self, down, up):
        """The 'data transaction' is the one we expect to see a useful
           data stage on. Normally it's Down for OUT endpoints and Up for IN,
           but on the control pipe this depends on the request type.
           The exception is that on the contro
           """
        if down.isDataTransaction():
            return down
        else:
            return up

    def addPair(self, down, up):
        """Once we have completed a full transaction, with 'down' and
           'up' events, it ends up here.
           """
        dataTransaction = self.getDataTransaction(down, up)
        self.stack.extend(up.timestamp)

        # Tag the interval we've chosen. The tag is a list that will
        # be updated with the canvas item reference once we have it.
        tag = [None]
        row = self.stack.pickRow(down.timestamp, up.timestamp, tag)

        w = self.stack.canvas.root().add(
            gnomecanvas.CanvasRect,
            y1 = row.top,
            y2 = row.bottom,
            fill_color = Style.getBarColor(dataTransaction).gdkString,
            outline_color = 'black',
            width_units = 1.0)
        self.view.ruler.resizer.track(w, x1=(down.timestamp, 0), x2=(up.timestamp, 0))
        tag[0] = w

        if up.status:
            # An error occurred, mark it in red
            x = 2
            err = self.stack.canvas.root().add(
                gnomecanvas.CanvasRect,
                y1 = row.top - x,
                y2 = row.bottom + x,
                fill_color_rgba = Style.errorMarkerColor.rgba)
            self.view.ruler.resizer.track(err,
                                          x1=(up.timestamp, -x),
                                          x2=(up.timestamp, x))

        # Set up links between the up/down transactions and the widget
        w.downTransaction = down
        w.upTransaction = up
        w.dataTransaction = dataTransaction
        self.view.transactionWidgets[down] = w
        self.view.transactionWidgets[up] = w


def detectPipeClass(dev, endpoint):
    """Autodetect a TimingDiagramPipe for a particular device and endpoint.
       This is a very early and crude start at doing graphical class-specific
       decoding in here.
       """
    # XXX: Very Experimental, I'm using this to track feedback values on
    #      USB audio devices.
    #if endpoint == 0x81:
    #    return IntegerDecoderPipe

    return TimingDiagramPipe


class IntegerDecoderPipe(TimingDiagramPipe):
    """An experimental sort of graphical class decoder. This one
       creates a line graph of an integer value decoded from each packet.
       """
    def __init__(self, view, range=(0x150000, 0x200000)):
        TimingDiagramPipe.__init__(self, view)
        self.range = range
        self.graphRow = self.stack.newPrivateRow(128)

    def addPair(self, down, up):
        TimingDiagramPipe.addPair(self, down, up)

        # Little endian
        bytes = up.hexData.split()
        bytes.reverse()
        value = int(''.join(bytes), 16)
        f = (value - self.range[0]) / (self.range[1] - self.range[0])

        # White background above the transaction itself
        self.stack.canvas.root().add(
            gnomecanvas.CanvasRect,
            x1 = self.view.ruler.scale * down.timestamp,
            x2 = self.view.ruler.scale * up.timestamp,
            y1 = self.graphRow.top,
            y2 = self.graphRow.bottom,
            fill_color = 'white')

        # Dot marking this vertex's position
        vertex = (self.view.ruler.scale * up.timestamp,
                  self.graphRow.bottom + f * (self.graphRow.top - self.graphRow.bottom))
        self.stack.canvas.root().add(
            gnomecanvas.CanvasRect,
            x1 = vertex[0] - 2,
            x2 = vertex[0] + 2,
            y1 = vertex[1] - 2,
            y2 = vertex[1] + 2,
            fill_color = 'black')


class TimingDiagram(View):
    """A view that draws a graphical representation of the time interval
       occupied by each USB transaction. This object is responsible for
       the overall layout of the diagram, and for tracking hilights, cursors,
       and scale. The actual graphical rendering is done for each endpoint
       by a TimingDiagramPipe.
       """
    hilightWidget = None
    vPadding = 3
    labelPadding = 3

    def createWidgets(self):
        self.transactionWidgets = {}
        self.pipes = {}

        self.scroll = ScrollContainer()
        self.scroll.hAdjust[0].set_property("step-increment", 20)
        self.cursor = Types.Observable(-1)

        # The ruler at the top is in charge of our time scale
        self.ruler = Ruler()
        TimeCursor(self.ruler, self.ruler, self.cursor)
        self.ruler.canvas.set_hadjustment(self.scroll.hAdjust[0])

        # The top heading holds the ruler, and a spacer aligned with the left heading
        leftHeadingWidth = gtk.SizeGroup(gtk.SIZE_GROUP_HORIZONTAL)
        spacer = gtk.Label()
        leftHeadingWidth.add_widget(spacer)
        topHeading = gtk.HBox(False)
        topHeading.pack_start(spacer, False, padding=self.labelPadding)
        topHeading.pack_start(gtk.VSeparator(), False)
        topHeading.pack_start(self.ruler.canvas, True)

        # Everything except the top heading scrolls vertically using a viewport
        viewport = gtk.Viewport(None, self.scroll.vAdjust[0])
        viewport.set_shadow_type(gtk.SHADOW_NONE)
        viewport.set_size_request(1, 1)
        viewportBox = gtk.VBox(False)
        viewportBox.pack_start(topHeading, False)
        viewportBox.pack_start(gtk.HSeparator(), False)
        viewportBox.pack_start(viewport, True)
        self.scroll.add(viewportBox)

        # Gnome Canvas has really silly event grabbing semantics. To work around
        # all this, we'll just grab all events before gnome-canvas sees them.
        for widget in (viewport, self.ruler.canvas):
            widget.add_events(gtk.gdk.POINTER_MOTION_HINT_MASK |
                              gtk.gdk.POINTER_MOTION_MASK |
                              gtk.gdk.BUTTON_PRESS_MASK)
            for event in ('motion-notify-event', 'button-press-event', 'scroll-event'):
                widget.connect_after(event, self.mouseEvent)
            self.scroll.attachEvent(widget, horizontal=True)

        # The left heading holds labels for each canvas in the stack
        self.leftHeading = gtk.VBox(False)
        leftHeadingWidth.add_widget(self.leftHeading)

        # The viewport holds the left heading, then the main canvas stack
        scrolledBox = gtk.HBox(False)
        self.canvasList = []
        self.canvasStack = gtk.VBox(False)
        scrolledBox.pack_start(self.leftHeading, False, padding=self.labelPadding)
        scrolledBox.pack_start(gtk.VSeparator(), False)
        scrolledBox.pack_start(self.canvasStack, True)
        viewport.add(scrolledBox)

        return self.scroll

    def mouseEvent(self, widget, event):
        """Mouse motion events all get processed here. We move the cursor,
           then if the mouse button is down we hilight the canvas item under it.
           This provides two useful user interaction scenarios: either the user
           can click on a single item they want more information on, or they
           can drag the mouse through the diagram and see each transaction
           hilighted in real-time.
           """
        # Zoom in and out with the middle and right mouse buttons
        if event.type == gtk.gdk.BUTTON_PRESS:
            if event.button == 2:
                self.ruler.zoom(2)
            elif event.button == 3:
                self.ruler.zoom(0.5)

        # Use the ruler widget's coordinate system to update the time cursor
        x, y, mask = self.ruler.canvas.window.get_pointer()
        scroll = self.ruler.canvas.get_scroll_offsets()[0]
        t = (x + scroll) / self.ruler.scale
        self.cursor.value = t

        # If the mouse button is down, try to find the canvas item under the cursor.
        # We use the row's collision detection tree for this, for the same reason
        # we use it for everything else: gnome-canvas' built-in collision detection
        # works poorly on very small items.
        if mask & gtk.gdk.BUTTON1_MASK:

            # Search every row in every canvas
            for obj in self.canvasList:
                y = obj.canvas.get_pointer()[1]
                for row in obj.rows:
                    if y >= row.top and y <= row.bottom:

                        # Give a few pixels of slack on either side
                        slack = 2.0 / self.ruler.scale
                        cursorInterval = (t - slack, t + slack)

                        # The mouse is in this row. Use the row's collision detection
                        # to find a nearby item.
                        tag = row.intervalOccupied(*cursorInterval)
                        if tag and tag[0] != self.hilightWidget:
                            self.notifyHilightChanged(tag[0].dataTransaction)
                            self.setHilightWidget(tag[0])
                        return False
        return False

    def appendCanvas(self, label, obj):
        """Add a new canvas-bearing object to our stack of scrolling widgets"""
        obj.canvas.set_hadjustment(self.scroll.hAdjust[0])

        labelWidget = gtk.Label(label)
        labelWidget.set_alignment(0, 0)

        group = gtk.SizeGroup(gtk.SIZE_GROUP_VERTICAL)
        group.add_widget(labelWidget)
        group.add_widget(obj.canvas)

        self.leftHeading.pack_start(labelWidget, False, padding=self.vPadding)
        self.canvasStack.pack_start(obj.canvas, False, padding=self.vPadding)

        self.leftHeading.pack_start(gtk.HSeparator(), False)
        self.canvasStack.pack_start(gtk.HSeparator(), False)

        self.leftHeading.show_all()
        self.canvasStack.show_all()
        self.canvasList.append(obj)

    def handleEvent(self, event):
        # Find a pipe to send transactions to
        if isinstance(event, Types.Transaction):
            key = event.dev, event.endpt
            try:
                pipe = self.pipes[key]
            except KeyError:
                pipe = self.createPipe(event)
                self.pipes[key] = pipe
            pipe.handleEvent(event)

        # Let the ruler display SOF markers
        elif isinstance(event, Types.SOFMarker):
            self.ruler.markFrame(event)

        # Mark matching regions in diffs. We have a translucent fill over
        # the entire matching area, and hard edges at the matching area's
        # boundaries. This makes it easy to see subtle boundaries in adjacent
        # diff matches.
        elif isinstance(event, Types.DiffMarker):
            for obj in self.canvasList:
                self.ruler.resizer.track(obj.canvas.root().add(
                    gnomecanvas.CanvasRect,
                    y1 = -50, y2 = 10000,
                    fill_color_rgba = Style.diffMarkerColor.rgba,
                    outline_color_rgba = Style.diffBorderColor.rgba,
                    width_units = 1.0,
                    ),
                    x1 = (event.timestamp, -1.5),
                    x2 = (event.matches[-1].timestamp, 1.5))

    def createPipe(self, transaction):
        """Return a new pipe created for the given transaction"""
        pipe = detectPipeClass(transaction.dev, transaction.endpt)(self)
        name = "Dev %s, %s" % (transaction.dev, transaction.getTransferString())
        self.appendCanvas(name, pipe.stack)
        return pipe

    def onHilightChanged(self, hilight):
        if hilight:
            # Move the cursor, and scroll to the hilighted
            # location. We try to keep at least 'margin' pixels
            # between the cursor and the edge of the viewport.
            self.cursor.value = hilight.timestamp
            x = int(self.ruler.scale * hilight.timestamp)
            margin = 50
            self.scroll.hAdjust[0].clamp_page(x-margin, x+margin)

            # If this transaction corresponds to a widget, hilight that
            try:
                w = self.transactionWidgets[hilight]
            except KeyError:
                pass
            else:
                self.setHilightWidget(w)

    def setHilightWidget(self, widget):
        if self.hilightWidget:
            self.hilightWidget.set(width_units=1)
        self.hilightWidget = widget
        self.hilightWidget.set(width_units=3)


class TransactionList(View):
    """A view that displays a table, each line showing a
       summary of a single transaction.
       """
    settingCursor = False
    diffPartner = None
    followLog = False

    def createWidgets(self):
        self.view = gtk.TreeView()
        self.view.set_rules_hint(True)
        self.view.connect("cursor-changed", self.onCursorChanged)

        self.selectionInfo = Types.Observable()
        self.onSelectionChanged(self.view.get_selection())

        # Allow multiple select, and hook up a context menu
        self.view.get_selection().set_mode(gtk.SELECTION_MULTIPLE)
        self.view.get_selection().connect("changed", self.onSelectionChanged)
        self.view.connect("button-press-event", self.onButtonPressed)

        self.createModel()
        self.createColumns()

        self.eventToIter = {}
        self.pipes = {}

        return self.view

    def onCursorChanged(self, view):
        """This is called when the cursor (the single row the user is focused on)
           changes. Propagate that change to other views.
           """
        if not self.settingCursor:
            row = view.get_cursor()[0]
            i = self.model.get_iter(row)
            event = self.model.get(i, 9)[0]
            self.notifyHilightChanged(event)

    def onSelectionChanged(self, selection):
        """When the set of selected rows change, update the summary data in the
           progress bar.
           """
        rows = selection.get_selected_rows()[1]
        if not rows:
            self.selectionInfo.value = "No Selection"
            return

        firstEvent = self.model.get(self.model.get_iter(rows[0]), 9)[0]
        lastEvent = self.model.get(self.model.get_iter(rows[-1]), 9)[0]
        timespan = lastEvent.timestamp - firstEvent.timestamp

        # Calculate the total amount of data. For consistency, this
        # ignores SETUP packets just like saveSelectedData() does.
        dataSize = 0
        for row in self.view.get_selection().get_selected_rows()[1]:
            iter = self.model.get_iter(row)
            event = self.model.get(iter, 9)[0]
            if event.isDataTransaction():
                dataSize += event.datalen or 0
        kb = dataSize / 1024.0

        if timespan > 0:
            bandwidth = "%.03f" % (kb / timespan)
        else:
            bandwidth = 'inf'

        self.selectionInfo.value = "%.03f kB, %.06f s, %s kB/s" % (
            kb, timespan, bandwidth)

    def onHilightChanged(self, hilight):
        self.settingCursor = True
        iter = self.eventToIter[hilight]
        self.view.set_cursor(self.model.get_path(iter))
        self.settingCursor = False

    def onButtonPressed(self, view, event):
        if event.button == 3:
            self.createMenu().popup(None, None, None, event.button, event.time)
            return True
        return False

    def createMenu(self):
        menu = gtk.Menu()

        item = gtk.MenuItem("Select _All")
        menu.append(item)
        item.connect("activate", self.selectAll)
        item.show()

        item = gtk.MenuItem("Save Selected Data...")
        menu.append(item)
        item.connect("activate", self.saveSelectedData)
        item.show()

        item = gtk.CheckMenuItem("Follow Log")
        item.set_active(self.followLog)
        menu.append(item)
        item.connect("activate", self.followLogToggled)
        item.show()

        item = gtk.MenuItem("Filter")
        item.set_submenu(self.createFilterMenu())
        menu.append(item)
        item.show()

        return menu

    def selectAll(self, widget):
        self.view.get_selection().select_all()

    def followLogToggled(self, widget):
        self.followLog = widget.get_active()

    def saveSelectedData(self, widget):
        dialog = gtk.FileChooserDialog(
            title = "Save Raw Transaction Data",
            action = gtk.FILE_CHOOSER_ACTION_SAVE,
            buttons = (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                       gtk.STOCK_SAVE, gtk.RESPONSE_ACCEPT))
        dialog.set_default_response(gtk.RESPONSE_ACCEPT)

        if dialog.run() == gtk.RESPONSE_ACCEPT:
            f = open(dialog.get_filename(), "wb")

            for row in self.view.get_selection().get_selected_rows()[1]:
                iter = self.model.get_iter(row)
                event = self.model.get(iter, 9)[0]

                if event.hasSetupData():
                    f.write(event.data[8:])
                else:
                    f.write(event.data)

        dialog.destroy()

    def createFilterMenu(self):
        menu = gtk.Menu()

        for dir in ('Down', 'Up'):
            item = gtk.MenuItem(dir)
            menu.append(item)
            item.connect("activate", self.filterSelection,
                         lambda event, dir=dir: event.dir == dir)
            item.show()

        pipes = self.pipes.keys()
        pipes.sort()
        for dev, transfer, endpt in pipes:
            item = gtk.MenuItem("Dev %s, %s" % (dev, transfer))
            menu.append(item)
            item.connect("activate", self.filterSelection,
                         lambda event, dev=dev, endpt=endpt:
                         event.dev == dev and event.endpt == endpt)
            item.show()

        return menu

    def filterSelection(self, widget, callback):
        """Filter our current selection through a callback function that
           accepts a Transaction and returns a boolean indicating whether
           it should still be selected.
           """
        for row in self.view.get_selection().get_selected_rows()[1]:
            iter = self.model.get_iter(row)
            event = self.model.get(iter, 9)[0]
            if not callback(event):
                self.view.get_selection().unselect_iter(iter)

    def createModel(self):
        self.model = gtk.ListStore(gobject.TYPE_STRING,   # 0. Time
                                   gobject.TYPE_STRING,   # 1. Device
                                   gobject.TYPE_STRING,   # 2. Transfer
                                   gobject.TYPE_STRING,   # 3. Transfer icon
                                   gobject.TYPE_STRING,   # 4. Setup
                                   gobject.TYPE_STRING,   # 5. Data (summary)
                                   gobject.TYPE_STRING,   # 6. Decoded (summary)
                                   gobject.TYPE_STRING,   # 7. Color name
                                   gobject.TYPE_STRING,   # 8. Data length
                                   gobject.TYPE_PYOBJECT, # 9. Event object
                                   )
        self.view.set_model(self.model)

    def createColumns(self):
        column = gtk.TreeViewColumn("Transfer")
        renderer = gtk.CellRendererPixbuf()
        column.pack_start(renderer, False)
        column.set_attributes(renderer, stock_id=3)
        renderer = gtk.CellRendererText()
        column.pack_start(renderer, True)
        column.set_attributes(renderer, text=2)
        self.view.append_column(column)

        self.view.append_column(gtk.TreeViewColumn(
            "Time", gtk.CellRendererText(), text=0))

        self.view.append_column(gtk.TreeViewColumn(
            "Device", gtk.CellRendererText(), text=1))

        self.view.append_column(gtk.TreeViewColumn(
            "Length", gtk.CellRendererText(), text=8))

        renderer = gtk.CellRendererText()
        renderer.set_property("font", Style.monospaceFont)
        self.view.append_column(gtk.TreeViewColumn(
            "Setup", renderer, text=4))

        renderer = gtk.CellRendererText()
        renderer.set_property("font", Style.monospaceFont)
        self.view.append_column(gtk.TreeViewColumn(
            "Data", renderer, text=5, foreground=7))

        renderer = gtk.CellRendererText()
        renderer.set_property("font", Style.monospaceFont)
        self.view.append_column(gtk.TreeViewColumn(
            "Decoded", renderer, text=6))

    def handleEvent(self, event):
        if isinstance(event, Types.Transaction):
            transfer = event.getTransferString()
            self.pipes[event.dev, transfer, event.endpt] = True

            timeString = " %.06fs " % event.timestamp
            if event.frame is not None:
                timeString += "fr.%d " % event.frame
            if event.lineNumber is not None:
                timeString += ":%d " % event.lineNumber

            if event.status and event.dir == 'Up':
                # Error! Right now we don't have anywhere better to
                # display this than the data field.
                dataSummary = 'Status: %s' % event.status
                color = Style.errorMarkerColor.gdkString
            else:
                dataSummary = event.getHexDump(summarize=True)
                color = Style.directionColors[event.dir].gdkString

            rowIter = self.model.append()
            self.model.set(rowIter,
                           0, timeString,
                           1, str(event.dev),
                           2, transfer,
                           3, Style.directionIcons[event.dir],
                           4, (event.getHexSetup() or '') + "  ",
                           5, dataSummary,
                           6, event.decodedSummary,
                           7, color,
                           8, "0x%04X" % (event.datalen or 0),
                           9, event,
                           )

            # Save a mapping from event to row iter, which we
            # expect to remain valid in a GtkListStore.
            self.eventToIter[event] = rowIter

            if self.followLog:
                self.hilight.setValue(event)

        # Any time we get a diff marker, check whether we need
        # to add padding to catch up to our partner's position.
        elif isinstance(event, Types.DiffMarker) and self.diffPartner:
            self.alignEvents(event.matches[0], event.matchedWith[0],
                             self.diffPartner)

    def alignEvents(self, ourEvent, otherEvent, otherView):
        """Align ourEvent in this view with otherEvent in the
           otherView by inserting blank lines.
           If ourEvent is already ahead of otherEvent, this
           does nothing- it's the otherView's responsibility
           to catch up.
           """
        ourIter = self.eventToIter[ourEvent]
        ourRow = self.model.get_path(ourIter)[0]
        otherRow = otherView.model.get_path(otherView.eventToIter[otherEvent])[0]
        padding = otherRow - ourRow

        while padding > 0:
            self.model.insert_before(ourIter)
            padding -= 1


class ScrolledTransactionList(TransactionList):
    """Adds a ScrolledWindow to TransactionList"""
    def createWidgets(self):
        view = TransactionList.createWidgets(self)

        root = gtk.ScrolledWindow()
        root.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        root.add(view)

        # Give ourselves focus by default- currently there's nothing else
        # usefully focusable, and this lets the user navigate between
        # transactions (even when they're only paying attention to other views)
        # using the keyboard.
        view.grab_focus()
        return root


class TransactionDetail(View):
    """A view that displays only the hilighted transaction,
       but in full detail.
       """
    def createWidgets(self):
        self.dataDetail = gtk.Label()
        self.dataDetail.set_alignment(0, 0)
        self.dataDetail.set_selectable(True)

        self.decodedDetail = gtk.Label()
        self.decodedDetail.set_alignment(0, 0)
        self.decodedDetail.set_selectable(True)

        scroll1 = gtk.ScrolledWindow()
        scroll1.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        scroll1.add_with_viewport(self.dataDetail)

        scroll2 = gtk.ScrolledWindow()
        scroll2.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        scroll2.add_with_viewport(self.decodedDetail)

        paned = gtk.HPaned()
        paned.add1(scroll1)
        paned.add2(scroll2)
        paned.set_position(600)
        return paned

    def onHilightChanged(self, transaction):
        self.dataDetail.set_markup(Style.toMonospaceMarkup(transaction.getHexDump()))
        self.decodedDetail.set_markup(Style.toMonospaceMarkup(transaction.decoded))


class TransactionDetailWindow(TransactionDetail):
    """A TransactionDetail view that opens in a separate window"""
    def createWidgets(self):
        detail = TransactionDetail.createWidgets(self)
        detail.show_all()

        window = gtk.Window()
        window.set_default_size(900, 200)
        window.add(detail)
        window.connect("delete-event", self.onDelete)
        window.set_title("Transaction Detail - VUsb Analyzer")

        self.visible = False
        return window

    def onDelete(self, widget, event):
        self.hide()
        return True

    def show(self):
        self.visible = True
        self.onHilightChanged(self.hilight.value)
        self.root.present()

    def hide(self):
        self.visible = False
        self.root.hide()

    def onHilightChanged(self, transaction):
        if self.visible:
            TransactionDetail.onHilightChanged(self, transaction)

    def connectToList(self, view):
        """When a row in the provided treeview is activated, show the detail window"""
        view.connect("row-activated", lambda widget, row, column: self.show())


class StatusMonitor:
    """Manages a status bar and progress bar. Progress events from any number
       of sources, from any thread, are processed by a queue here.

       Events are of the form (source, progress) where source is a descriptive
       name identifying the progress reporter, and 'progress' is a number
       in the range [0, 1].
       """
    interval = 500

    def __init__(self):
        self.queue = Queue.Queue()
        self.sources = {}
        self.completionCallbacks = []

        self.statusbar = gtk.Statusbar()

        self.progress = gtk.ProgressBar()
        self.progressContext = self.statusbar.get_context_id("Progress")
        self.statusbar.pack_end(self.progress, False)

        self.cursorLabel = gtk.Label()
        frame = gtk.Frame()
        frame.add(self.cursorLabel)
        self.statusbar.pack_end(frame, False)

        self.selectionLabel = gtk.Label()
        frame = gtk.Frame()
        frame.add(self.selectionLabel)
        self.statusbar.pack_end(frame, False)

        self.poll()

    def poll(self):
        try:
            needUpdate = False
            try:
                while 1:
                    source, progress = self.queue.get(False)
                    self.sources[source] = progress
                    needUpdate = True
            except Queue.Empty:
                pass
            if needUpdate:
                self.update()
            gobject.timeout_add(self.interval, self.poll)
        except KeyboardInterrupt:
            gtk.main_quit()

    def update(self):
        overall = sum(self.sources.itervalues()) / len(self.sources)
        busySources = [source for source, progress in self.sources.iteritems()
                       if progress < 1.0]

        self.statusbar.pop(self.progressContext)
        if busySources:
            self.progress.show()
            self.progress.set_fraction(overall)
            self.statusbar.push(self.progressContext, ", ".join(busySources))
        else:
            # Finished with everything we were doing. Hide the progress bar,
            # clear out old progress information, and call completion handlers.
            self.progress.hide()
            self.sources = {}
            for f in self.completionCallbacks:
                f()

    def watchCursor(self, cursor):
        """Show the position of this cursor on the status bar"""
        cursor.observers.append(self._cursorCallback)

    def _cursorCallback(self, t):
        self.cursorLabel.set_markup(Style.toMonospaceMarkup(
            " %.04f s " % t))

    def watchSelection(self, sel):
        """Watch an Observable value indicating the current selection status"""
        sel.observers.append(self.selectionLabel.set_text)


class MainWindow(ViewContainer):
    """The Main Window is a container for all our included Views"""
    def __init__(self):
        ViewContainer.__init__(self)

        tlist = ScrolledTransactionList(self)
        TransactionDetailWindow(self).connectToList(tlist.view)
        self.status = StatusMonitor()
        self.status.watchSelection(tlist.selectionInfo)

        paned = gtk.VPaned()
        if gnomecanvas:
            timing = TimingDiagram(self)
            self.status.watchCursor(timing.cursor)
            paned.add1(timing.root)
        paned.add2(tlist.root)
        paned.set_position(250)

        mainvbox = gtk.VBox(False)
        mainvbox.pack_start(paned, True)
        mainvbox.pack_start(self.status.statusbar, False)

        self.window = gtk.Window()
        self.window.set_default_size(1200, 900)
        self.window.add(mainvbox)
        self.window.show_all()
