/**
 * \file btthread.h
 * \brief Fichier contenant les déclaration des fonctions relatives au thread Bluetooth
 * \author Stanislas BERTRAND
 * \version 0.1
 * \date 24 mars 2011
 */

#ifndef VEX_BT_THREAD_H
#define VEX_BT_THREAD_H

#include "../appdata.h"

gpointer thread_bluetooth_inquiry(AppData*);

void thread_Communication_Func(AppData *);
void thread_local_server(AppData *);

#endif // VEX_BT_THREAD_H
