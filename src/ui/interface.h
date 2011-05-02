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

#define SCREEN_ORIGIN_X 350
#define SCREEN_ORIGIN_Y 400
#define DRAWING_AREA_WIDTH 700
#define DRAWING_AREA_HEIGTH 480

AppData * create_data();
void create_user_interface(AppData *);
void create_search_toolbar(AppData *);
void create_services_toolbar(AppData *);
void create_options_toolbar(AppData *);
void create_comm_interface( AppData *);
void display_comm_interface( AppData * );
void create_local_server_interface( AppData *);
void display_local_server_interface( AppData * );
void info_display(char * info,GtkWidget * window,const char * file,int line);
#define INFO_DISP(info,window) info_display(info,window,__FUNCTION__,__LINE__)

#endif // VEX_INTERFACE_H
