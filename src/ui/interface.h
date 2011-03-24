/*
*
*	File name : interface.h
*	Author : Stanislas BERTRAND
*	Purpuse : User Interface definitions for Vex Bluetooth Remote Control
*
*/

#ifndef VEX_INTERFACE_H
#define VEX_INTERFACE_H

#include "../appdata.h"

AppData * create_data();
void create_user_interface(AppData *);
void create_search_toolbar(AppData *);
void create_services_toolbar(AppData *);
void create_options_toolbar(AppData *);
void create_comm_interface( AppData *);
void display_comm_interface( AppData * );
void create_local_server_interface( AppData *);
void display_local_server_interface( AppData * );

#endif // VEX_INTERFACE_H
