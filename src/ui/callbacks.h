/**
 * \file callbacks.h
 * \brief Fichier contenant les définiation relatives au callbacks de l'interface
 * \author Stanislas BERTRAND
 * \version 0.1
 * \date 24 mars 2011
 */

#ifndef VEX_CALLBACKS_H
#define VEX_CALLBACKS_H

#include "../appdata.h"

// Bluetooth device selected in the combobox on the search toolbar
void callback_dev_selected(GtkComboBox *,AppData *);

// User wants to search for bluetooth device
void callback_search_dev (GtkToolButton *, AppData *);

// User wants to set options
void callback_display_options (GtkToggleToolButton *, AppData *);

// User changed UUID
void callback_uuid_changed(GtkComboBox *,AppData *);

//connect to the selected service
void callback_connect(GtkToolButton *, AppData *);

// Callback to the send button
void callback_send (GtkButton *button,AppData *);

//Callback to get hardware button events
gboolean callback_hardware_button(GtkWidget *, GdkEventKey *,AppData *);

// The communication window has been closed
gboolean callback_com_close(GtkWidget *, GdkEvent *,AppData *);

// The callback to pain the comm user interface.
gboolean callback_expose (GtkWidget *widget, GdkEventExpose *event, AppData * data);

// Get Mouse events
gboolean callback_mouse (GtkWidget *widget,GdkEvent *event,AppData *data);

//GDK_Return
gboolean callback_entry_keys(GtkWidget *entry,GdkEventKey *event,AppData *data);

// Com STOP ( Fullscrean )
gboolean callback_com_stop(GtkWidget * widget, GdkEvent *event,AppData *data);

#endif // VEX_CALLBACKS_H
