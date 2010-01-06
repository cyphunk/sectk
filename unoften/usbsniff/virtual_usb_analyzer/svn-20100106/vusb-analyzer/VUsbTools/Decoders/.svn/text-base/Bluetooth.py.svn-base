#
# VUsbTools.Decoders.Bluetooth
# Micah Dowty <micah@vmware.com>
#
# A decoder module for portions of the Bluetooth HCI protocol
#
# Copyright (C) 2005-2009 VMware, Inc. Licensed under the MIT
# License, please see the README.txt. All rights reserved.
#

from VUsbTools import Decode, Struct

class ControlDecoder(Decode.ControlDecoder):
    """This extends the default ControlDecoder with support for
       HCI commands encapsulated within class-specific control requests.
       """
    classRequests = Struct.EnumDict({
        0x00: 'HCICommand',
        })

    hciOpcodes = {
        0x01: ("LC", { 0x0001: "HCI_Inquiry",
                       0x0002: "HCI_Inquiry_Cancel",
                       0x0003: "HCI_Periodic_Inquiry_Mode",
                       0x0004: "HCI_Exit_Periodic_Inquiry_Mode",
                       0x0005: "HCI_Create_Connection",
                       0x0006: "HCI_Disconnect",
                       0x0008: "HCI_Create_Connection_Cancel",
                       0x0009: "HCI_Accept_Connection_Request",
                       0x000A: "HCI_Reject_Connection_Request",
                       0x000B: "HCI_Link_Key_Request_Reply",
                       0x000C: "HCI_Link_Key_Request_Negative_Reply",
                       0x000D: "HCI_PIN_Code_Request_Reply",
                       0x000E: "HCI_PIN_Code_Request_Negative_Reply",
                       0x000F: "HCI_Change_Connection_Packet_Type",
                       0x0011: "HCI_Authentication_Requested",
                       0x0013: "HCI_Set_Connection_Encryption",
                       0x0015: "HCI_Change_Connection_Link_Key",
                       0x0017: "HCI_Master_Link_Key",
                       0x0019: "HCI_Remote_Name_Request",
                       0x001A: "HCI_Remote_Name_Request_Cancel",
                       0x001B: "HCI_Read_Remote_Supported_Features",
                       0x001C: "HCI_Read_Remote_Extended_Features",
                       0x001D: "HCI_Read_Remote_Version_Information",
                       0x001F: "HCI_Read_Clock_Offset",
                       0x0020: "HCI_Read_LMP_Handle",
                       0x0028: "HCI_Setup_Synchronous_Connection",
                       0x0029: "HCI_Accept_Synchronous_Connection_Request",
                       0x002A: "HCI_Reject_Synchronous_Connection_Request",
                       }),
        0x02: ("LP", { 0x0001: "HCI_Hold_Mode",
                       0x0003: "HCI_Sniff_Mode",
                       0x0004: "HCI_Exit_Sniff_Mode",
                       0x0005: "HCI_Park_State",
                       0x0006: "HCI_Exit_Park_State",
                       0x0007: "HCI_QoS_Setup",
                       0x0009: "HCI_Role_Discovery",
                       0x000B: "HCI_Switch_Role",
                       0x000C: "HCI_Read_Link_Policy_Settings",
                       0x000D: "HCI_Write_Link_Policy_Settings",
                       0x000E: "HCI_Read_Default_Link_Policy_Settings",
                       0x000F: "HCI_Write_Default_Link_Policy_Settings",
                       0x0010: "HCI_Flow_Specification",
                       }),
        0x03: ("C&B", {0x0001: "HCI_Set_Event_Mask",
                       0x0003: "HCI_Reset",
                       0x0005: "HCI_Set_Event_Filter",
                       0x0008: "HCI_Flush",
                       0x0009: "HCI_Read_PIN_Type",
                       0x000A: "HCI_Write_PIN_Type",
                       0x000B: "HCI_Create_New_Unit_Key",
                       0x000D: "HCI_Read_Stored_Link_Key",
                       0x0011: "HCI_Write_Stored_Link_Key",
                       0x0012: "HCI_Delete_Stored_Link_Key",
                       0x0013: "HCI_Write_Local_Name",
                       0x0014: "HCI_Read_Local_Name",
                       0x0015: "HCI_Read_Connection_Accept_Timeout",
                       0x0016: "HCI_Write_Connection_Accept_Timeout",
                       0x0017: "HCI_Read_Page_Timeout",
                       0x0018: "HCI_Write_Page_Timeout",
                       0x0019: "HCI_Read_Scan_Enable",
                       0x001A: "HCI_Write_Scan_Enable",
                       0x001B: "HCI_Read_Page_Scan_Activity",
                       0x001C: "HCI_Write_Page_Scan_Activity",
                       0x001D: "HCI_Read_Inquiry_Scan_Activity",
                       0x001E: "HCI_Write_Inquiry_Scan_Activity",
                       0x001F: "HCI_Read_Authentication_Enable",
                       0x0020: "HCI_Write_Authentication_Enable",
                       0x0021: "HCI_Read_Encryption_Mode",
                       0x0022: "HCI_Write_Encryption_Mode",
                       0x0023: "HCI_Read_Class_of_Device",
                       0x0024: "HCI_Write_Class_of_Device",
                       0x0025: "HCI_Read_Voice_Setting",
                       0x0026: "HCI_Write_Voice_Setting",
                       0x0027: "HCI_Read_Automatic_Flush_Timeout",
                       0x0028: "HCI_Write_Automatic_Flush_Timeout",
                       0x0029: "HCI_Read_Num_Broadcast_Transmissions",
                       0x002A: "HCI_Write_Num_Broadcast_Transmissions",
                       0x002B: "HCI_Read_Hold_Mode_Activity",
                       0x002C: "HCI_Write_Hold_Mode_Activity",
                       0x002D: "HCI_Read_Transmit_Power_Level",
                       0x002E: "HCI_Read_Synchronous_Flow_Control_Enable",
                       0x002F: "HCI_Write_Synchronous_Flow_Control_Enable",
                       0x0031: "HCI_Set_Controller_To_Host_Flow_Control",
                       0x0033: "HCI_Host_Buffer_Size",
                       0x0035: "HCI_Host_Number_Of_Completed_Packets",
                       0x0036: "HCI_Read_Link_Supervision_Timeout",
                       0x0037: "HCI_Write_Link_Supervision_Timeout",
                       0x0038: "HCI_Read_Number_Of_Supported_IAC",
                       0x0039: "HCI_Read_Current_IAC_LAP",
                       0x003A: "HCI_Write_Current_IAC_LAP",
                       0x003B: "HCI_Read_Page_Scan_Period_Mode",
                       0x003C: "HCI_Write_Page_Scan_Period_Mode",
                       0x003F: "Set_AFH_Host_Channel_Classification",
                       0x0042: "HCI_Read_Inquiry_Scan_Type",
                       0x0043: "HCI_Write_Inquiry_Scan_Type",
                       0x0044: "HCI_Read_Inquiry_Mode",
                       0x0045: "HCI_Write_Inquiry_Mode",
                       0x0046: "HCI_Read_Page_Scan_Type",
                       0x0047: "HCI_Write_Page_Scan_Type",
                       0x0048: "Read_AFH_Channel_Assignment_Mode",
                       0x0049: "Write_AFH_Channel_Assignment_Mode",
                       }),
        0x04: ("IP", { 0x0001: "HCI_Read_Local_Version_Information",
                       0x0002: "HCI_Read_Local_Supported_Commands",
                       0x0003: "HCI_Read_Local_Supported_Features",
                       0x0004: "HCI_Read_Local_Extended_Features",
                       0x0005: "HCI_Read_Buffer_Size",
                       0x0009: "HCI_Read_BD_Addr",
                       }),
        0x05: ("SP", { 0x0001: "HCI_Read_Failed_Contact_Counter",
                       0x0002: "HCI_Reset_Failed_Contact_Counter",
                       0x0003: "HCI_Read_Link_Quality",
                       0x0005: "HCI_Read_RSSI",
                       0x0006: "HCI_Read_AFH_Channel_Map",
                       0x0007: "HCI_Read_Clock",
                       }),
        0x06: ("Test",{0x0001: "HCI_Read_Loopback_Mode",
                       0x0002: "HCI_Write_Loopback_Mode",
                       0x0003: "HCI_Enable_Device_Under_Test_Mode",
                       }),
        }

    def decode_HCICommand(self, setup):
        header = Struct.Group(None,
            Struct.UInt16("opcode"),
            Struct.UInt8("numParameters"),
            )
        params = header.decode(setup.event.data[8:])

        if header.opcode is None:
            ocf = ogf = None
        else:
            ocf = header.opcode & 0x03FF
            ogf = header.opcode >> 10

        ogfName = self.hciOpcodes.get(ogf, (ogf, {}))[0]
        ocfName = self.hciOpcodes.get(ogf, (ogf, {}))[1].get(ocf, ocf)

        setup.event.pushDecoded("BT Command: %s [%s]" % (ocfName, ogfName))


class EventDecoder:
    """This decodes incoming Interrupt traffic from a bluetooth HCI
       device, representing HCI events.
       """
    eventNames = Struct.EnumDict({
        0x01: 'Inquiry Complete',
        0x02: 'Inquiry Result',
        0x03: 'Connection Complete',
        0x04: 'Connection Request',
        0x05: 'Disconnection Complete',
        0x06: 'Authentication Complete',
        0x07: 'Remote Name Request Complete',
        0x08: 'Encryption Change',
        0x09: 'Change Connection Link Key Complete',
        0x0A: 'Master Link Key Complete',
        0x0B: 'Read Remote Supported Features Complete',
        0x0C: 'Read Remote Version Information Complete',
        0x0D: 'QoS Setup Complete',
        0x0E: 'Command Complete',
        0x0F: 'Command Status',
        0x10: 'Hardware Error',
        0x11: 'Flush Occurred',
        0x12: 'Role Change',
        0x13: 'Number of Completed Packets',
        0x14: 'Mode Change',
        0x15: 'Return Link Keys',
        0x16: 'PIN Code Request',
        0x17: 'Link Key Request',
        0x18: 'Link Key Notification',
        0x19: 'Loopback Command',
        0x1A: 'Data Buffer Overflow',
        0x1B: 'Max Slots Change',
        0x1C: 'Read Clock Offset Complete',
        0x1D: 'Connection Packet Type Changed',
        0x1E: 'QoS Violation',
        0x20: 'Page Scan Repetition Mode Change',
        0x21: 'HCI Flow Specification Complete',
        0x22: 'Inquiry Result With RSSI',
        0x23: 'Read Remote Extended Features Complete',
        0x2C: 'Synchronous Connection Complete',
        0x2D: 'Synchronous Connection Changed',
        })

    def handleEvent(self, event):
        if not event.isDataTransaction():
            return

        header = Struct.Group(None,
            Struct.UInt8("eventCode"),
            Struct.UInt8("numParameters"),
            )
        params = header.decode(event.data)

        event.pushDecoded("BT Event: %s" % self.eventNames[header.eventCode])


class ACLDecoder:
    """This decodes incoming or outgoing Bulk data, which represents
       ACL transfers via the Bluetooth HCI protocol.
       """
    packetBoundaryNames = [
        "reserved_boundary_flag_00",
        "continuation",
        "first_packet",
        "reserved_boundary_flag_11",
        ]
    broadcastFlagNames = [
        "",
        " active_slave_broadcast",
        " parked_slave_broadcast",
        " reserved_broadcast_flag_11",
        ]

    def handleEvent(self, event):
        if not event.isDataTransaction():
            return

        header = Struct.Group(None,
            Struct.UInt16("handle"),
            Struct.UInt16("dataTotalLength"),
            )
        params = header.decode(event.data)
        if header.handle is None:
            return

        boundaryFlag = (header.handle >> 12) & 3
        broadcastFlag = (header.handle >> 14) & 3
        handle = header.handle & 0x0FFF

        event.pushDecoded("BT ACL: handle=0x%04x len=0x%04x (%s%s)" %
                          (handle, header.dataTotalLength,
                           self.packetBoundaryNames[boundaryFlag],
                           self.broadcastFlagNames[broadcastFlag]))


def detector(context):
    if (context.device and
        context.device.bDeviceClass == 0xE0 and
        context.device.bDeviceSubClass == 0x01
        ):

        if not context.endpoint:
            return ControlDecoder(context.devInstance)

        if context.endpoint.bmAttributes is not None:
            if context.endpoint.bmAttributes & 3 == 3:
                return EventDecoder()

            if context.endpoint.bmAttributes & 3 == 2:
                return ACLDecoder()
