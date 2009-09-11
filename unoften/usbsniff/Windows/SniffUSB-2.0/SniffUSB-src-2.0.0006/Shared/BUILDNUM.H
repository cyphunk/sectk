#ifndef _BUILDNUM_H_INCLUDED_ 
#define _BUILDNUM_H_INCLUDED_ 

#define STRINGER(v, w, x, y, z) v #w "." #x "." #y "." #z "\0"
#define XSTRINGER(v, w, x, y, z) STRINGER(v, w, x, y, z)

//
// product specific information
//
#define PRODUCT_LEGALCOPYRIGHT_YEARS    "2002-2007 "
#define PRODUCT_COPYRIGHT_STR           "Copyright \251 2000-2007 SniffUsb Project"
#define PRODUCT_COMPANYNAME_STR         "SniffUsb/UsbSnoop Project"
#define PRODUCT_PRODUCTNAME_STR         "SniffUsb "

#define PRODUCT_RELEASE_LEVEL_STR       "V"
#define PRODUCT_VER_MAJOR               2
#define PRODUCT_VER_MINOR               0
#define PRODUCT_VER_BASELEVEL           0
#define PRODUCT_VER_BUILD               6

#define PRODUCT_VERSION                 PRODUCT_VER_MAJOR,PRODUCT_VER_MINOR,PRODUCT_VER_BASELEVEL,PRODUCT_VER_BUILD
#define PRODUCT_VERSION_STR             XSTRINGER(PRODUCT_RELEASE_LEVEL_STR,PRODUCT_VER_MAJOR,PRODUCT_VER_MINOR,PRODUCT_VER_BASELEVEL,PRODUCT_VER_BUILD)

#endif // !_BUILDNUM_H_INCLUDED_ 

