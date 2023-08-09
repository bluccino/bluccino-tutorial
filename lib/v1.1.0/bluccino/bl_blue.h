//==============================================================================
//  bl_blue.h
//  auxillary header to include Bluetooth/Mesh Zephyr-stuff
//
//  Created by Hugo Pristauz on 19.02.2022
//  Copyright © 2022 Bluenetics GmbH. All rights reserved.
//==============================================================================

#ifndef __BL_BLUE_H__
#define __BL_BLUE_H__

#include "bl_type.h"

    #if defined(__ZEPHYR__)
        #include <zephyr/settings/settings.h>
        #include <zephyr/bluetooth/bluetooth.h>
        #include <zephyr/bluetooth/conn.h>
        #include <zephyr/bluetooth/l2cap.h>
        #include <zephyr/bluetooth/hci.h>
        #include <zephyr/bluetooth/mesh.h>
    #endif // defined(__ZEPHYR__)


//==============================================================================
// get own Bluetooth address
//==============================================================================

    void bl_get_bdaddr(uint8_t *paddr);

#endif // __BL_BLUE_H__
