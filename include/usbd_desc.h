#ifndef __USBD_DESC_H
#define __USBD_DESC_H

#include "usbd_def.h"

#define USBD_VID     1155
#define USBD_LANGID_STRING     1033
#define USBD_MANUFACTURER_STRING     "JulesIndustries"
#define USBD_PID_FS     22336
#define USBD_PRODUCT_STRING_FS     "AERO-CORE Thermal Controller"
#define USBD_CONFIGURATION_STRING_FS     "CDC Config"
#define USBD_INTERFACE_STRING_FS     "CDC Interface"

extern USBD_DescriptorsTypeDef FS_Desc;

#endif /* __USBD_DESC_H */
