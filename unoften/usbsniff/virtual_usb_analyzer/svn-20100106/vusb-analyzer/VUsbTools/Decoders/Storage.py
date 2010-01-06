#
# VUsbTools.Decoders.Storage
# Micah Dowty <micah@vmware.com>
#
# Decodes the USB bulk-only storage protocol, and portions of SCSI
#
# Copyright (C) 2005-2009 VMware, Inc. Licensed under the MIT
# License, please see the README.txt. All rights reserved.
#

from VUsbTools import Decode, Struct


class SCSICommand:
    """Decodes a SCSI command block"""

    _opcodes = Struct.EnumDict({
        0x00: 'TEST_UNIT_READY',
        0x01: 'REZERO_UNIT',
        0x03: 'REQUEST_SENSE',
        0x04: 'FORMAT_UNIT',
        0x05: 'READ_BLOCKLIMITS',
        0x07: 'REASSIGN_BLOCKS',
        0x07: 'INIT_ELEMENT_STATUS',
        0x08: 'READ(6)',
        0x0a: 'WRITE(6)',
        0x0a: 'PRINT',
        0x0b: 'SEEK(6)',
        0x0b: 'SLEW_AND_PRINT',
        0x0f: 'READ_REVERSE',
        0x10: 'WRITE_FILEMARKS',
        0x10: 'SYNC_BUFFER',
        0x11: 'SPACE',
        0x12: 'INQUIRY',
        0x14: 'RECOVER_BUFFERED',
        0x15: 'MODE_SELECT',
        0x16: 'RESERVE_UNIT',
        0x17: 'RELEASE_UNIT',
        0x18: 'COPY',
        0x19: 'ERASE',
        0x1a: 'MODE_SENSE',
        0x1b: 'START_UNIT',
        0x1b: 'SCAN',
        0x1b: 'STOP_PRINT',
        0x1c: 'RECV_DIAGNOSTIC',
        0x1d: 'SEND_DIAGNOSTIC',
        0x1e: 'MEDIUM_REMOVAL',
        0x24: 'SET_WINDOW',
        0x25: 'GET_WINDOW',
        0x25: 'READ_CAPACITY',
        0x28: 'READ(10)',
        0x29: 'READ_GENERATION',
        0x2a: 'WRITE(10)',
        0x2b: 'SEEK(10)',
        0x2b: 'POSITION_TO_ELEMENT',
        0x2d: 'READ_UPDATED_BLOCK',
        0x2e: 'WRITE_VERIFY',
        0x2f: 'VERIFY',
        0x30: 'SEARCH_DATA_HIGH',
        0x31: 'SEARCH_DATA_EQUAL',
        0x32: 'SEARCH_DATA_LOW',
        0x33: 'SET_LIMITS',
        0x34: 'PREFETCH',
        0x34: 'READ_POSITION',
        0x35: 'SYNC_CACHE',
        0x36: 'LOCKUNLOCK_CACHE',
        0x37: 'READ_DEFECT_DATA',
        0x38: 'MEDIUM_SCAN',
        0x39: 'COMPARE',
        0x3a: 'COPY_VERIFY',
        0x3b: 'WRITE_BUFFER',
        0x3c: 'READ_BUFFER',
        0x3d: 'UPDATE_BLOCK',
        0x3e: 'READ_LONG',
        0x3f: 'WRITE_LONG',
        0x40: 'CHANGE_DEF',
        0x41: 'WRITE_SAME',
        0x42: 'READ_SUBCHANNEL',
        0x43: 'READ_TOC',
        0x44: 'READ_HEADER',
        0x45: 'PLAY_AUDIO(10)',
        0x46: 'GET_CONFIGURATION',
        0x47: 'PLAY_AUDIO_MSF',
        0x48: 'PLAY_AUDIO_TRACK',
        0x49: 'PLAY_AUDIO_RELATIVE',
        0x4a: 'GET_EVENT_STATUS_NOTIFICATION',
        0x4b: 'PAUSE',
        0x4c: 'LOG_SELECT',
        0x4d: 'LOG_SENSE',
        0x4e: 'STOP_PLAY',
        0x51: 'READ_DISC_INFO',
        0x52: 'READ_TRACK_INFO',
        0x53: 'RESERVE_TRACK',
        0x54: 'SEND_OPC_INFORMATION',
        0x55: 'MODE_SELECT(10)',
        0x56: 'RESERVE_UNIT(10)',
        0x57: 'RELEASE_UNIT(10)',
        0x5a: 'MODE_SENSE(10)',
        0x5b: 'CLOSE_SESSION',
        0x5c: 'READ_BUFFER_CAPACITY',
        0x5d: 'SEND_CUE_SHEET',
        0x5e: 'PERSISTENT_RESERVE_IN',
        0x5f: 'PERSISTENT_RESERVE_OUT',
        0x88: 'READ(16)',
        0x8a: 'WRITE(16)',
        0x9e: 'READ_CAPACITY(16)',
        0xa0: 'REPORT_LUNS',
        0xa1: 'BLANK',
        0xa3: 'MAINTENANCE_IN',
        0xa4: 'MAINTENANCE_OUT',
        0xa3: 'SEND_KEY',
        0xa4: 'REPORT_KEY',
        0xa5: 'MOVE_MEDIUM',
        0xa5: 'PLAY_AUDIO(12)',
        0xa6: 'EXCHANGE_MEDIUM',
        0xa6: 'LOADCD',
        0xa8: 'READ(12)',
        0xa9: 'PLAY_TRACK_RELATIVE',
        0xaa: 'WRITE(12)',
        0xac: 'ERASE(12)',
        0xac: 'GET_PERFORMANCE',
        0xad: 'READ_DVD_STRUCTURE',
        0xae: 'WRITE_VERIFY(12)',
        0xaf: 'VERIFY(12)',
        0xb0: 'SEARCH_DATA_HIGH(12)',
        0xb1: 'SEARCH_DATA_EQUAL(12)',
        0xb2: 'SEARCH_DATA_LOW(12)',
        0xb3: 'SET_LIMITS(12)',
        0xb5: 'REQUEST_VOLUME_ELEMENT_ADDR',
        0xb6: 'SEND_VOLUME_TAG',
        0xb6: 'SET_STREAMING',
        0xb7: 'READ_DEFECT_DATA(12)',
        0xb8: 'READ_ELEMENT_STATUS',
        0xb8: 'SELECT_CDROM_SPEED',
        0xb9: 'READ_CD_MSF',
        0xba: 'AUDIO_SCAN',
        0xbb: 'SET_CDROM_SPEED',
        0xbc: 'SEND_CDROM_XA_DATA',
        0xbc: 'PLAY_CD',
        0xbd: 'MECH_STATUS',
        0xbe: 'READ_CD',
        0xbf: 'SEND_DVD_STRUCTURE',
        })

    # For every command we want to decode parameters for, this includes
    # a struct definition, and a format string that defines a useful
    # summary of the parameters. The entire struct is included after
    # the summary line.
    _structs = {
#        'INQUIRY': None, # FIXME: We should decode this. SCSI-2, page 104 (8.2.5)

        'READ(6)':    ("0x%(length)02x blocks at 0x%(lba)04x", lambda: (
                       Struct.UInt8('lun'),    # FIXME: lun is actually a bitfield
                       Struct.UInt16BE('lba'),
                       Struct.UInt8('length'),
                       Struct.UInt8('control'))),
        'READ(10)':   ("0x%(length)04x blocks at 0x%(lba)08x", lambda: (
                       Struct.UInt8('lun'),    # FIXME: lun is actually a bitfield
                       Struct.UInt32BE('lba'),
                       Struct.UInt8('reserved_1'),
                       Struct.UInt16BE('length'),
                       Struct.UInt8('control'))),
        }

    def __init__(self, cdb):
        self.header = Struct.Group(None, Struct.UInt8("opcode"))
        params = self.header.decode(cdb)
        self.name = self._opcodes[self.header.opcode]

        if self.name in self._structs:
            fmt, children = self._structs[self.name]
            self.params = Struct.Group(None, *children())
            self.params.decode(params)
            self.summary = fmt % self.params.__dict__
        else:
            self.params = None
            self.summary = ''

    def __str__(self):
        return "%s %s" % (self.name, self.summary)


class CommandDecoder:
    """Decodes USBC command blocks"""

    def handleEvent(self, event):
        if not event.isDataTransaction():
            return
        if not event.data.startswith("USBC"):
            return

        header = Struct.Group(None,
                              Struct.UInt32("sig"),
                              Struct.UInt32("tag"),
                              Struct.UInt32("datalen"),
                              Struct.UInt8("flag"),
                              Struct.UInt8("lun"),
                              Struct.UInt8("cdblen"))
        cdb = header.decode(event.data)
        command = SCSICommand(cdb)

        event.pushDecoded("Storage Command: %s" % command)
        event.appendDecoded("\n%s" % command.params)


class StatusDecoder:
    """Decodes USBS status blocks"""

    _statusCodes = Struct.EnumDict({
        0x00: 'ok',
        0x01: 'FAILED',
        0x02: 'PHASE ERROR',
        })

    def handleEvent(self, event):
        if not event.isDataTransaction():
            return
        if not event.data.startswith("USBS"):
            return

        header = Struct.Group(None,
                              Struct.UInt32("sig"),
                              Struct.UInt32("tag"),
                              Struct.UInt32("residue"),
                              Struct.UInt8("status"))
        header.decode(event.data)

        if header.residue:
            residue = ', residue=%d' % header.residue
        else:
            residue = ''

        event.pushDecoded("Storage Status (%s%s)" % (
            self._statusCodes[header.status], residue))
        event.appendDecoded("\n%s" % header)


def detector(context):
    if (context.interface and context.endpoint and
        context.interface.bInterfaceClass == 0x08 and
        context.interface.bInterfaceSubClass == 0x06 and
        (context.endpoint.bmAttributes & 3) == 2
        ):
        if context.endpoint.bEndpointAddress & 0x80:
            return StatusDecoder()
        else:
            return CommandDecoder()
