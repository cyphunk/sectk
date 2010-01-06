#
# VUsbTools.Decoders.Cypress
# Micah Dowty <micah@vmware.com>
#
# Decodes the bootloader interface used by Cypress EZ-USB microcontrollers
#
# Copyright (C) 2005-2009 VMware, Inc. Licensed under the MIT
# License, please see the README.txt. All rights reserved.
#

from VUsbTools import Decode, Struct


class FX2Decoder(Decode.ControlDecoder):
    """Decodes the control endpoint on Cypress FX2 chips. This endpoint
       has vendor-specific commands, implemented in hardware, to read and write RAM.
       """
    vendorRequests = Struct.EnumDict({
        0xA0: 'FirmwareCommand',
        })

    registerInfo = {
        0xE600: ('CPUCS',             'Control & Status'),
        0xE601: ('IFCONFIG',          'Interface Configuration'),
        0xE602: ('PINFLAGSAB',        'FIFO FLAGA and FLAGB Assignments'),
        0xE603: ('PINFLAGSCD',        'FIFO FLAGC and FLAGD Assignments'),
        0xE604: ('FIFORESET',         'Restore FIFOS to default state'),
        0xE605: ('BREAKPT',           'Breakpoint'),
        0xE606: ('BPADDRH',           'Breakpoint Address H'),
        0xE607: ('BPADDRL',           'Breakpoint Address L'),
        0xE608: ('UART230',           '230 Kbaud clock for T0,T1,T2'),
        0xE609: ('FIFOPINPOLAR',      'FIFO polarities'),
        0xE60A: ('REVID',             'Chip Revision'),
        0xE60B: ('REVCTL',            'Chip Revision Control'),
        0xE610: ('EP1OUTCFG',         'Endpoint 1-OUT Configuration'),
        0xE611: ('EP1INCFG',          'Endpoint 1-IN Configuration'),
        0xE612: ('EP2CFG',            'Endpoint 2 Configuration'),
        0xE613: ('EP4CFG',            'Endpoint 4 Configuration'),
        0xE614: ('EP6CFG',            'Endpoint 6 Configuration'),
        0xE615: ('EP8CFG',            'Endpoint 8 Configuration'),
        0xE618: ('EP2FIFOCFG',        'Endpoint 2 FIFO configuration'),
        0xE619: ('EP4FIFOCFG',        'Endpoint 4 FIFO configuration'),
        0xE61A: ('EP6FIFOCFG',        'Endpoint 6 FIFO configuration'),
        0xE61B: ('EP8FIFOCFG',        'Endpoint 8 FIFO configuration'),
        0xE620: ('EP2AUTOINLENH',     'Endpoint 2 Packet Length H (IN only)'),
        0xE621: ('EP2AUTOINLENL',     'Endpoint 2 Packet Length L (IN only)'),
        0xE622: ('EP4AUTOINLENH',     'Endpoint 4 Packet Length H (IN only)'),
        0xE623: ('EP4AUTOINLENL',     'Endpoint 4 Packet Length L (IN only)'),
        0xE624: ('EP6AUTOINLENH',     'Endpoint 6 Packet Length H (IN only)'),
        0xE625: ('EP6AUTOINLENL',     'Endpoint 6 Packet Length L (IN only)'),
        0xE626: ('EP8AUTOINLENH',     'Endpoint 8 Packet Length H (IN only)'),
        0xE627: ('EP8AUTOINLENL',     'Endpoint 8 Packet Length L (IN only)'),
        0xE630: ('EP2FIFOPFH',        'EP2 Programmable Flag trigger H'),
        0xE631: ('EP2FIFOPFL',        'EP2 Programmable Flag trigger L'),
        0xE632: ('EP4FIFOPFH',        'EP4 Programmable Flag trigger H'),
        0xE633: ('EP4FIFOPFL',        'EP4 Programmable Flag trigger L'),
        0xE634: ('EP6FIFOPFH',        'EP6 Programmable Flag trigger H'),
        0xE635: ('EP6FIFOPFL',        'EP6 Programmable Flag trigger L'),
        0xE636: ('EP8FIFOPFH',        'EP8 Programmable Flag trigger H'),
        0xE637: ('EP8FIFOPFL',        'EP8 Programmable Flag trigger L'),
        0xE640: ('EP2ISOINPKTS',      'EP2 (if ISO) IN Packets per frame (1-3)'),
        0xE641: ('EP4ISOINPKTS',      'EP4 (if ISO) IN Packets per frame (1-3)'),
        0xE642: ('EP6ISOINPKTS',      'EP6 (if ISO) IN Packets per frame (1-3)'),
        0xE643: ('EP8ISOINPKTS',      'EP8 (if ISO) IN Packets per frame (1-3)'),
        0xE648: ('INPKTEND',          'Force IN Packet End'),
        0xE649: ('OUTPKTEND',         'Force OUT Packet End'),
        0xE650: ('EP2FIFOIE',         'Endpoint 2 Flag Interrupt Enable'),
        0xE651: ('EP2FIFOIRQ',        'Endpoint 2 Flag Interrupt Request'),
        0xE652: ('EP4FIFOIE',         'Endpoint 4 Flag Interrupt Enable'),
        0xE653: ('EP4FIFOIRQ',        'Endpoint 4 Flag Interrupt Request'),
        0xE654: ('EP6FIFOIE',         'Endpoint 6 Flag Interrupt Enable'),
        0xE655: ('EP6FIFOIRQ',        'Endpoint 6 Flag Interrupt Request'),
        0xE656: ('EP8FIFOIE',         'Endpoint 8 Flag Interrupt Enable'),
        0xE657: ('EP8FIFOIRQ',        'Endpoint 8 Flag Interrupt Request'),
        0xE658: ('IBNIE',             'IN-BULK-NAK Interrupt Enable'),
        0xE659: ('IBNIRQ',            'IN-BULK-NAK interrupt Request'),
        0xE65A: ('NAKIE',             'Endpoint Ping NAK interrupt Enable'),
        0xE65B: ('NAKIRQ',            'Endpoint Ping NAK interrupt Request'),
        0xE65C: ('USBIE',             'USB Int Enables'),
        0xE65D: ('USBIRQ',            'USB Interrupt Requests'),
        0xE65E: ('EPIE',              'Endpoint Interrupt Enables'),
        0xE65F: ('EPIRQ',             'Endpoint Interrupt Requests'),
        0xE660: ('GPIFIE',            'GPIF Interrupt Enable'),
        0xE661: ('GPIFIRQ',           'GPIF Interrupt Request'),
        0xE662: ('USBERRIE',          'USB Error Interrupt Enables'),
        0xE663: ('USBERRIRQ',         'USB Error Interrupt Requests'),
        0xE664: ('ERRCNTLIM',         'USB Error counter and limit'),
        0xE665: ('CLRERRCNT',         'Clear Error Counter EC[3..0]'),
        0xE666: ('INT2IVEC',          'Interupt 2 (USB) Autovector'),
        0xE667: ('INT4IVEC',          'Interupt 4 (FIFOS & GPIF) Autovector'),
        0xE668: ('INTSETUP',          'Interrupt 2&4 Setup'),
        0xE670: ('PORTACFG',          'I/O PORTA Alternate Configuration'),
        0xE671: ('PORTCCFG',          'I/O PORTC Alternate Configuration'),
        0xE672: ('PORTECFG',          'I/O PORTE Alternate Configuration'),
        0xE678: ('I2CS',              'Control & Status'),
        0xE679: ('I2DAT',             'Data'),
        0xE67A: ('I2CTL',             'I2C Control'),
        0xE67B: ('EXTAUTODAT1',       'Autoptr1 MOVX access'),
        0xE67C: ('EXTAUTODAT2',       'Autoptr2 MOVX access'),
        0xE680: ('USBCS',             'USB Control & Status'),
        0xE681: ('SUSPEND',           'Put chip into suspend'),
        0xE682: ('WAKEUPCS',          'Wakeup source and polarity'),
        0xE683: ('TOGCTL',            'Toggle Control'),
        0xE684: ('USBFRAMEH',         'USB Frame count H'),
        0xE685: ('USBFRAMEL',         'USB Frame count L'),
        0xE686: ('MICROFRAME',        'Microframe count, 0-7'),
        0xE687: ('FNADDR',            'USB Function address'),
        0xE68A: ('EP0BCH',            'Endpoint 0 Byte Count H'),
        0xE68B: ('EP0BCL',            'Endpoint 0 Byte Count L'),
        0xE68D: ('EP1OUTBC',          'Endpoint 1 OUT Byte Count'),
        0xE68F: ('EP1INBC',           'Endpoint 1 IN Byte Count'),
        0xE690: ('EP2BCH',            'Endpoint 2 Byte Count H'),
        0xE691: ('EP2BCL',            'Endpoint 2 Byte Count L'),
        0xE694: ('EP4BCH',            'Endpoint 4 Byte Count H'),
        0xE695: ('EP4BCL',            'Endpoint 4 Byte Count L'),
        0xE698: ('EP6BCH',            'Endpoint 6 Byte Count H'),
        0xE699: ('EP6BCL',            'Endpoint 6 Byte Count L'),
        0xE69C: ('EP8BCH',            'Endpoint 8 Byte Count H'),
        0xE69D: ('EP8BCL',            'Endpoint 8 Byte Count L'),
        0xE6A0: ('EP0CS',             'Endpoint  Control and Status'),
        0xE6A1: ('EP1OUTCS',          'Endpoint 1 OUT Control and Status'),
        0xE6A2: ('EP1INCS',           'Endpoint 1 IN Control and Status'),
        0xE6A3: ('EP2CS',             'Endpoint 2 Control and Status'),
        0xE6A4: ('EP4CS',             'Endpoint 4 Control and Status'),
        0xE6A5: ('EP6CS',             'Endpoint 6 Control and Status'),
        0xE6A6: ('EP8CS',             'Endpoint 8 Control and Status'),
        0xE6A7: ('EP2FIFOFLGS',       'Endpoint 2 Flags'),
        0xE6A8: ('EP4FIFOFLGS',       'Endpoint 4 Flags'),
        0xE6A9: ('EP6FIFOFLGS',       'Endpoint 6 Flags'),
        0xE6AA: ('EP8FIFOFLGS',       'Endpoint 8 Flags'),
        0xE6AB: ('EP2FIFOBCH',        'EP2 FIFO total byte count H'),
        0xE6AC: ('EP2FIFOBCL',        'EP2 FIFO total byte count L'),
        0xE6AD: ('EP4FIFOBCH',        'EP4 FIFO total byte count H'),
        0xE6AE: ('EP4FIFOBCL',        'EP4 FIFO total byte count L'),
        0xE6AF: ('EP6FIFOBCH',        'EP6 FIFO total byte count H'),
        0xE6B0: ('EP6FIFOBCL',        'EP6 FIFO total byte count L'),
        0xE6B1: ('EP8FIFOBCH',        'EP8 FIFO total byte count H'),
        0xE6B2: ('EP8FIFOBCL',        'EP8 FIFO total byte count L'),
        0xE6B3: ('SUDPTRH',           'Setup Data Pointer high address byte'),
        0xE6B4: ('SUDPTRL',           'Setup Data Pointer low address byte'),
        0xE6B5: ('SUDPTRCTL',         'Setup Data Pointer Auto Mode'),
        0xE6B8: ('SETUPDAT[8]',       '8 bytes of SETUP data'),
        0xE6C0: ('GPIFWFSELECT',      'Waveform Selector'),
        0xE6C1: ('GPIFIDLECS',        'GPIF Done, GPIF IDLE drive mode'),
        0xE6C2: ('GPIFIDLECTL',       'Inactive Bus, CTL states'),
        0xE6C3: ('GPIFCTLCFG',        'CTL OUT pin drive'),
        0xE6C4: ('GPIFADRH',          'GPIF Address H'),
        0xE6C5: ('GPIFADRL',          'GPIF Address L'),
        0xE6CE: ('GPIFTCB3',          'GPIF Transaction Count Byte 3'),
        0xE6CF: ('GPIFTCB2',          'GPIF Transaction Count Byte 2'),
        0xE6D0: ('GPIFTCB1',          'GPIF Transaction Count Byte 1'),
        0xE6D1: ('GPIFTCB0',          'GPIF Transaction Count Byte 0'),
        0xE6D0: ('EP2GPIFTCH',        'EP2 GPIF Transaction Count High'),
        0xE6D1: ('EP2GPIFTCL',        'EP2 GPIF Transaction Count Low'),
        0xE6D2: ('EP2GPIFFLGSEL',     'EP2 GPIF Flag select'),
        0xE6D3: ('EP2GPIFPFSTOP',     'Stop GPIF EP2 transaction on prog. flag'),
        0xE6D4: ('EP2GPIFTRIG',       'EP2 FIFO Trigger'),
        0xE6D8: ('EP4GPIFTCH',        'EP4 GPIF Transaction Count High'),
        0xE6D9: ('EP4GPIFTCL',        'EP4 GPIF Transactionr Count Low'),
        0xE6DA: ('EP4GPIFFLGSEL',     'EP4 GPIF Flag select'),
        0xE6DB: ('EP4GPIFPFSTOP',     'Stop GPIF EP4 transaction on prog. flag'),
        0xE6DC: ('EP4GPIFTRIG',       'EP4 FIFO Trigger'),
        0xE6E0: ('EP6GPIFTCH',        'EP6 GPIF Transaction Count High'),
        0xE6E1: ('EP6GPIFTCL',        'EP6 GPIF Transaction Count Low'),
        0xE6E2: ('EP6GPIFFLGSEL',     'EP6 GPIF Flag select'),
        0xE6E3: ('EP6GPIFPFSTOP',     'Stop GPIF EP6 transaction on prog. flag'),
        0xE6E4: ('EP6GPIFTRIG',       'EP6 FIFO Trigger'),
        0xE6E8: ('EP8GPIFTCH',        'EP8 GPIF Transaction Count High'),
        0xE6E9: ('EP8GPIFTCL',        'EP8GPIF Transaction Count Low'),
        0xE6EA: ('EP8GPIFFLGSEL',     'EP8 GPIF Flag select'),
        0xE6EB: ('EP8GPIFPFSTOP',     'Stop GPIF EP8 transaction on prog. flag'),
        0xE6EC: ('EP8GPIFTRIG',       'EP8 FIFO Trigger'),
        0xE6F0: ('XGPIFSGLDATH',      'GPIF Data H (16-bit mode only)'),
        0xE6F1: ('XGPIFSGLDATLX',     'Read/Write GPIF Data L & trigger transac'),
        0xE6F2: ('XGPIFSGLDATLNOX',   'Read GPIF Data L, no transac trigger'),
        0xE6F3: ('GPIFREADYCFG',      'Internal RDY,Sync/Async, RDY5CFG'),
        0xE6F4: ('GPIFREADYSTAT',     'RDY pin states'),
        0xE6F5: ('GPIFABORT',         'Abort GPIF cycles'),
        0xE6C6: ('FLOWSTATE',         'Defines GPIF flow state'),
        0xE6C7: ('FLOWLOGIC',         'Defines flow/hold decision criteria'),
        0xE6C8: ('FLOWEQ0CTL',        'CTL states during active flow state'),
        0xE6C9: ('FLOWEQ1CTL',        'CTL states during hold flow state'),
        0xE6CA: ('FLOWHOLDOFF',       ''),
        0xE6CB: ('FLOWSTB',           'CTL/RDY Signal to use as master data strobe'),
        0xE6CC: ('FLOWSTBEDGE',       'Defines active master strobe edge'),
        0xE6CD: ('FLOWSTBHPERIOD',    'Half Period of output master strobe'),
        0xE60C: ('GPIFHOLDAMOUNT',    'Data delay shift'),
        0xE67D: ('UDMACRCH',          'CRC Upper byte'),
        0xE67E: ('UDMACRCL',          'CRC Lower byte'),
        0xE67F: ('UDMACRCQUAL',       'UDMA In only, host terminated use only'),
        0xE6F8: ('DBUG',              'Debug'),
        0xE6F9: ('TESTCFG',           'Test configuration'),
        0xE6FA: ('USBTEST',           'USB Test Modes'),
        0xE6FB: ('CT1',               'Chirp Test--Override'),
        0xE6FC: ('CT2',               'Chirp Test--FSM'),
        0xE6FD: ('CT3',               'Chirp Test--Control Signals'),
        0xE6FE: ('CT4',               'Chirp Test--Inputs'),
        }

    def decode_FirmwareCommand(self, setup):
        direction = setup.bitmap & 0x80 != 0
        address = setup.wValue
        length = setup.wLength

        setup.event.pushDecoded("FX2 %s at 0x%04x (%s)" % (
            ('Write', 'Read')[direction], address,
            self.getAddressDescription(address)))

    def getAddressDescription(self, address):
        if address < 0x2000:
            return "Program Memory"

        if address < 0xE000:
            return "INVALID"

        if address < 0xE200:
            return "Scratch RAM"

        if address < 0xE400:
            return "RESERVED"

        if address < 0xE480:
            return "GPIF Waveforms"

        if address < 0xE600:
            return "RESERVED"

        if address < 0xE700:
            try:
                return "Register [%s] %s" % self.registerInfo[address]
            except KeyError:
                return "Unknown Register"

        if address < 0xE740:
            return "UNAVAILABLE"

        if address < 0xE780:
            return "EP0 Buffer"

        if address < 0xE800:
            return "EP1 Buffer"

        if address < 0xF000:
            return "RESERVED"

        return "EP2/4/6/8 Buffer"


def detector(context):
    #
    # There's no way to 100% reliably detect an EZ-USB device,
    # but we can look for its default collection of 9 endpoints.
    #

    if context.device and context.descriptors and not context.endpoint:
        # Do our detection on the device descriptor, but peek around
        # the list of other descriptors that were returned at the
        # same time.

        endpoints = []
        for desc in context.descriptors:
            if desc.type == 'endpoint':
                endpoints.append(desc.bEndpointAddress)

        if endpoints == [0x81, 0x82, 0x02, 0x84, 0x04, 0x86, 0x06, 0x88, 0x08]:
            return FX2Decoder(context.devInstance)
