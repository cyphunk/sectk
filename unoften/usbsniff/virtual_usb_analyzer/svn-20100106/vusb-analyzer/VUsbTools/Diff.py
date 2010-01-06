#
# VUsbTools.Diff
# Micah Dowty <micah@vmware.com>
#
# Implements UI support for a side-by-side diff mode.
#
# Copyright (C) 2005-2009 VMware, Inc. Licensed under the MIT
# License, please see the README.txt. All rights reserved.
#

from __future__ import division
import threading, difflib, gtk, gobject
from VUsbTools import Views, Types


class BackgroundDiff(threading.Thread):
    """Runs SeuqenceMatcher.get_matching_blocks in the background,
       reporting progress periodically as the operation progresses.
       A callback is run with each match as we find it, in the
       main thread.
       """
    def __init__(self, a, b, progressQueue, callback):
        self.a = a
        self.b = b
        self.matcher = None
        self.progressQueue = progressQueue
        self.callback = callback
        self.finished = False
        self.checkpoint = 0
        threading.Thread.__init__(self)
        self.poll()

    def run(self):
        try:
            self.matcher = difflib.SequenceMatcher(None, self.a, self.b)
            self.matcher.get_matching_blocks()
            self.finished = True
        except KeyboardInterrupt:
            gtk.main_quit()

    def poll(self):
        """This runs periodically in the main thread to check
           on the progress of our diff thread.
           """
        try:
            # Report all new matches to the callback
            if self.matcher and self.matcher.matching_blocks:
                blocks = self.matcher.matching_blocks
                newCheckpoint = len(blocks)
                for i in xrange(self.checkpoint, newCheckpoint):
                    self.callback(blocks[i])
                self.checkpoint = newCheckpoint

            # Update progress
            self.progressQueue.put(("Performing diff...", self.getProgress()))

            # Schedule the next poll if we're not done
            if not self.finished:
                gobject.timeout_add(250, self.poll)
        except KeyboardInterrupt:
            gtk.main_quit()

    def getProgress(self):
        if self.finished:
            return 1.0
        if not self.matcher:
            return 0
        if not self.matcher.matching_blocks:
            return 0
        i, j, n = self.matcher.matching_blocks[-1]
        return (i + j) / (len(self.a) + len(self.b))


class DiffStatusColumn:
    """Uses a TreeView to display '><|' markers indicating
       the status of lines in a side-by-side diff. This expects
       to receive raw matches from difflib, rather than
       our DiffMarker events.
       """
    def __init__(self):
        self.view = gtk.TreeView()
        self.view.set_sensitive(False)
        self.createModel()
        self.createColumns()
        self.i = 0
        self.j = 0

    def createModel(self):
        self.model = gtk.ListStore(gobject.TYPE_STRING,   # 0. Marker text
                                   )
        self.view.set_model(self.model)

    def createColumns(self):
        renderer = gtk.CellRendererText()
        renderer.set_property("xalign", 0.5)
        self.view.append_column(gtk.TreeViewColumn(
            "", renderer, text=0))

    def match(self, i, j, n):
        """Process one match as returned by difflib"""
        si = self.i
        sj = self.j

        # Advance the two pointers in parallel as far as we can
        # without hitting this match. This indicates lines that
        # are different on both sides.
        count = min(i - si, j - sj)
        if count > 0:
            self.mark("|", count)
            si += count
            sj += count

        # Now see if we can advance the individual sides separately,
        # indicating lines on one side that match gaps on the other.
        count = i - si
        if count > 0:
            self.mark("<", count)
            si += count

        count = j - sj
        if count > 0:
            self.mark(">", count)
            sj += count

        # Now the pointers are sync'ed, and we can mark the match itself
        if n > 0:
            self.mark("", n)
            si += n
            sj += n

        self.i = si
        self.j = sj

    def mark(self, label, count):
        """Add 'count' copies of the indicated mark to our list"""
        while count:
            self.model.set(self.model.append(),
                           0, label)
            count -= 1


class DiffWindow:
    """This is an alternate main window used to compare two log files"""
    def __init__(self):
        self.views = (Views.ViewContainer(), Views.ViewContainer())

        self.status = Views.StatusMonitor()
        self.needDiff = True
        self.diffMatches = {}
        self.status.completionCallbacks.append(self.loadingFinished)

        # Two scrolling columns for our transaction lists,
        # with one non-scrolled column in the middle for our diff markers.
        # Everything scrolls vertically in unison.
        listScroll = Views.ScrollContainer(hAxes = (True, False, True))

        self.diffStatus = DiffStatusColumn()
        self.diffStatus.view.set_size_request(20, 1)
        self.diffStatus.view.set_vadjustment(listScroll.vAdjust[0])
        frame = gtk.Frame()
        frame.add(self.diffStatus.view)
        listScroll.add(frame, 1, 0, 0)

        timingBox = gtk.VBox(True)

        paned = gtk.VPaned()
        if Views.gnomecanvas:
            paned.add1(timingBox)
        paned.add2(listScroll)
        paned.set_position(250)

        self.events = {}
        self.summaries = {}

        transactionLists = []
        for i, view in (
            (0, self.views[0]),
            (2, self.views[1]),
            ):
            self.events[view] = []
            self.summaries[view] = []

            tlist = Views.TransactionList(view)
            transactionLists.append(tlist)
            tlist.view.set_hadjustment(listScroll.hAdjust[i])
            tlist.view.set_vadjustment(listScroll.vAdjust[0])
            tlist.root.set_size_request(1, 1)
            listScroll.attachEvent(tlist.root)

            frame = gtk.Frame()
            frame.add(tlist.root)
            listScroll.add(frame, i)

            Views.TransactionDetailWindow(view).connectToList(tlist.view)

            timing = Views.TimingDiagram(view)
            self.status.watchCursor(timing.cursor)
            timingBox.pack_start(timing.root)

            # When a transaction is hilighted in one view,
            # try to find a matching transaction in the other view
            view.hilight.observers.append(self.matchHilights)

        transactionLists[0].diffPartner = transactionLists[1]
        transactionLists[1].diffPartner = transactionLists[0]

        mainvbox = gtk.VBox(False)
        mainvbox.pack_start(paned, True)
        mainvbox.pack_start(self.status.statusbar, False)

        self.window = gtk.Window()
        self.window.set_default_size(1200,900)
        self.window.add(mainvbox)
        self.window.show_all()

    def handleEvent(self, event, view):
        if isinstance(event, Types.Transaction):
            self.events[view].append(event)
            self.summaries[view].append(event.getDiffSummary())
        view.handleEvent(event)

    def loadingFinished(self):
        """We're idle. If we still need a diff, start one"""
        if self.needDiff:
            self.needDiff = False
            self.bgDiff = BackgroundDiff(self.summaries[self.views[0]],
                                         self.summaries[self.views[1]],
                                         self.status.queue,
                                         self.diffCallback)
            self.bgDiff.start()

    def diffCallback(self, (i, j, n)):
        """This is called by BackgroundDiff when a new matching
           block is discovered. The block starts at [i] and [j]
           in our events/summaries lists, and extends for 'n' elements.
           """
        self.diffStatus.match(i, j, n)

        if n < 1:
            return

        for view, offset, otherView, otherOffset in (
            (self.views[0], i, self.views[1], j),
            (self.views[1], j, self.views[0], i),
            ):

            # Generate DiffMarkers to show this match visually
            view.handleEvent(Types.DiffMarker(
                (self.events[view][offset],
                 self.events[view][offset + n - 1]),
                (self.events[otherView][otherOffset],
                 self.events[otherView][otherOffset + n - 1])))

            # Correlate each transaction with its match
            for index in xrange(n):
                self.diffMatches[self.events[view][offset + index]] = (
                    otherView, self.events[otherView][otherOffset + index])

    def matchHilights(self, transaction):
        try:
            otherView, match = self.diffMatches[transaction]
        except KeyError:
            pass
        else:
            otherView.hilight.setValue(match, (self.matchHilights,))
