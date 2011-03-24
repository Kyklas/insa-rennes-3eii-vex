/**
 * \file bluetooth.h
 * \brief Fichier contenant les déclaration des fonctions relatives au Bluetooth
 * \author Stanislas BERTRAND
 * \version 0.1
 * \date 14 mars 2011
 */

#ifndef VEX_BLUETOOTH_H
#define VEX_BLUETOOTH_H

#include "../appdata.h"

#include <cstdio>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>


#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>



/**
 * \brief Nombre de valeur UUID (Bluetooth)
 */
#define BT_NUMBER_UUID_VALUES 15

/**
 * \brief Channel du service bluetooth local
 *
 */
#define LOCAL_SERVER_CHANNEL 3

extern char uuid_strings[BT_NUMBER_UUID_VALUES][40];

extern int uuid_values[BT_NUMBER_UUID_VALUES];

void get_info(AppData *);

bool create_comm_connection(AppData *);

bool create_loc_serv_record(AppData *);

void unregister_loc_serv_record(AppData* );

#endif // VEX_BLUETOOTH_H



