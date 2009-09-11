; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CSniffUSBDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "sniffusb.h"
LastPage=0

ClassCount=3
Class1=CSniffUSBApp
Class2=CAboutDlg
Class3=CSniffUSBDlg

ResourceCount=3
Resource1=IDD_SNIFFUSB_DIALOG (English (U.S.))
Resource2=IDD_ABOUTBOX (English (U.S.))
Resource3=IDR_SNOOPUSB (English (U.S.))

[CLS:CSniffUSBApp]
Type=0
BaseClass=CWinApp
HeaderFile=SniffUSB.h
ImplementationFile=SniffUSB.cpp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=SniffUSBDlg.cpp
ImplementationFile=SniffUSBDlg.cpp
LastObject=CAboutDlg

[CLS:CSniffUSBDlg]
Type=0
BaseClass=CDialog
HeaderFile=SniffUSBDlg.h
ImplementationFile=SniffUSBDlg.cpp
Filter=D
VirtualFilter=dWC
LastObject=IDC_DELETE

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg

[DLG:IDD_SNIFFUSB_DIALOG]
Type=1
Class=CSniffUSBDlg

[DLG:IDD_ABOUTBOX (English (U.S.))]
Type=1
Class=?
ControlCount=5
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_STATIC,static,1342308352

[DLG:IDD_SNIFFUSB_DIALOG (English (U.S.))]
Type=1
Class=CSniffUSBDlg
ControlCount=14
Control1=IDC_USBDEVS,SysListView32,1350631708
Control2=IDC_REPLUG,button,1342242816
Control3=IDC_REFRESH,button,1342242816
Control4=IDC_INSTALL,button,1342242816
Control5=IDC_UNINSTALL,button,1342242816
Control6=IDCANCEL,button,1342242816
Control7=IDC_FILTERINSTALL,button,1073807360
Control8=IDC_TEST,button,1342242816
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC2,static,1342308352
Control11=IDC_LOG_SIZE,edit,1350633600
Control12=IDC_LOG_FILENAME,edit,1350633600
Control13=IDC_DELETE,button,1342242816
Control14=IDC_VIEW,button,1342242816

[MNU:IDR_SNOOPUSB (English (U.S.))]
Type=1
Class=?
Command1=ID_SNOOPUSB_INSTALL
Command2=ID_SNOOPUSB_UNINSTALL
Command3=ID_SNOOPUSB_REPLUG
Command4=ID_SNOOPUSB_CREATESERVICE
Command5=ID_SNOOPUSB_DELETESERVICE
Command6=ID_SNOOPUSB_STARTSERVICE
Command7=ID_SNOOPUSB_STOPSERVICE
CommandCount=7

