/**
 * \file callbacks.c
 * \brief Fichier contenant les fonctions relatives au callbacks de l'interface
 * \author Stanislas BERTRAND
 * \version 0.1
 * \date 24 mars 2011
 */

#include <glib.h>

#include "callbacks.h"
#include "uithread.h"
#include "../bt/btthread.h"
#include "interface.h"
#include "../bt/bluetooth.h"
#include <math.h>
#include "../communication.h"


/**
 * \fn void callback_dev_selected(GtkComboBox * cb_dev_list,AppData * data)
 * \brief Selection d'un périphérique
 *
 * \param[in] cb_dev Liste déroulante de la selection
 * \param[in,out] data Donné du programme
 *
 * Un périphérique a été selectionné dans la liste déroulante
 *
 */

// Bluetooth device selected in the combobox on the search toolbar
void callback_dev_selected(GtkComboBox * cb_dev_list,AppData * data)
{

	int device = gtk_combo_box_get_active(data->cb_dev_list);

	if( device == 0 )
	{
		// local device selected.
		//terminate any server connection
		//Like we were closing the window
		if(data->locserv != NULL )
		{
		//	GdkEventAny* event = new GdkEventAny;
		//	event->type=GDK_NOTHING;
		//	event->window = (GdkWindow* )data->locserv->ui.window;
		//	//event->sent_event = false;
		//	callback_com_close((GtkWidget *)data->locserv->ui.window,(GdkEvent *)event,data);
			gtk_widget_destroy( (GtkWidget *) data->locserv->ui.window); // to test
		}
		// Create the local server
		// 1 - Create the bluetooth record  and register on the SDP
		// 	if the registering fails, no connection will be able to come thru.
		if ( !create_loc_serv_record(data) )
		{
			// Do other stuff
			//return; // fails on sratchbox
		}
		// 2 - the user interface
		// 	Creates the data->locserv
		create_local_server_interface(data);
		// 3 - Create the server socket and listen
		// 	lunch the thread ..
		data->locserv->bt.thread_Communication = g_thread_create((GThreadFunc) thread_local_server,data,true,NULL);

		hildon_banner_show_information((GtkWidget *)data->window_main,NULL,"Local Server !");
	}
	else
	{
		// a remote device is selected
		create_services_toolbar(data);
	}
}
/**
 * \fn void callback_search_dev (GtkToolButton * it_search , AppData * data)
 * \brief Demande de recherche d'un périphérique
 *
 * \param[in] it_search Bouton de la demande
 * \param[in,out] data Donné du programme
 *
 * L'utilisateur veux rechercher les périphériques bluetooth
 *
 */

void callback_search_dev (GtkToolButton * it_search , AppData * data)
{

	/* Remove services toolbar */
	if (data->tb_services != NULL)
	{
		hildon_window_remove_toolbar(data->window_main, data->tb_services);
	}

	while(data->number_devices > 0)
	{
		gtk_combo_box_remove_text(data->cb_dev_list,0);
		data->number_devices--;
	}
	// and the local server
	gtk_combo_box_remove_text(data->cb_dev_list,0);

	if(data->cond_search_finish != NULL)
	{
		g_cond_signal(data->cond_search_finish);
		while(data->cond_search_finish!=NULL);
	}


	data->cond_search_finish = g_cond_new();

	data->banner_progress_bar = (GtkProgressBar *) gtk_progress_bar_new();
	data->banner_progress =(HildonBanner*) hildon_banner_show_progress((GtkWidget*) data->window_main,
							data->banner_progress_bar,"Searching Devices ...");
	gtk_widget_show_all((GtkWidget*) data->banner_progress);

	g_thread_create((GThreadFunc) thread_banner_progress,data,FALSE,NULL);

	g_thread_create((GThreadFunc) thread_bluetooth_inquiry,data,FALSE,NULL);



}

/**
 * \fn void callback_display_options (GtkToggleToolButton *item, AppData *data)
 * \brief Affiche la barre d'options
 *
 * \param[in] item Bouton d'option
 * \param[in,out] data Donné du programme
 *
 *  Affiche la barre d'options pour selectionner et changer celles-ci.
 *
 */

void callback_display_options (GtkToggleToolButton *item, AppData *data){

	if(gtk_toggle_tool_button_get_active(item)){
		gtk_widget_show_all((GtkWidget *)data->tb_options);
	}
	else{
		gtk_widget_hide_all((GtkWidget *)data->tb_options);
	}

}

/**
 * \fn callback_uuid_changed(GtkComboBox * cb_uuid_list,AppData * data)
 * \brief Change le profile Bluetooth recherché
 *
 * \param[in] cb_uuid_list Liste déroulante des profiles
 * \param[in,out] data Donné du programme
 *
 *  Selectionne le nouveau profile Bluetooth recherché
 *
 */

void callback_uuid_changed(GtkComboBox * cb_uuid_list,AppData * data){
	printf("Selected %d - text %s\n",(int) gtk_combo_box_get_active (cb_uuid_list),gtk_combo_box_get_active_text(cb_uuid_list));
	puts("callback_uuid_changed");

	// if gtk_combo_box_get_active return -1
	// the user has specified a UUID

	int selected_uuid;

	if(gtk_combo_box_get_active (cb_uuid_list) == -1)
	{
		sscanf(gtk_combo_box_get_active_text(cb_uuid_list),"%X",&selected_uuid);
	}
	else
	{
		selected_uuid=uuid_values[gtk_combo_box_get_active (cb_uuid_list)];
	}

	printf("0x%04X",selected_uuid);
	puts(".");
	data->setting_uuid = selected_uuid;

}

/**
 * \fn void callback_connect(GtkToolButton * widget, AppData * data)
 * \brief Connection a un périphérique
 *
 * \param[in] widget Bouton connect
 * \param[in,out] data Donné du programme
 *
 *  lance la connection a un périphérique
 *
 */

void callback_connect(GtkToolButton * widget, AppData * data){


	if(data->devconn != NULL )
	{
		//terminate any other connection
		// Like we were closing the window
		GdkEventAny* event =(GdkEventAny*) malloc(sizeof(GdkEventAny));
		event->type=GDK_NOTHING;
		event->window = (GdkWindow* )data->devconn->ui.window;

		callback_com_close((GtkWidget *)data->devconn->ui.window,(GdkEvent *)event,data);
	}

	//Create device connection data
	data->devconn = (DeviceConnection*) malloc(sizeof(DeviceConnection));

#ifdef FAST_CONNECT

	hildon_banner_show_information((GtkWidget *)data->window_main,NULL,"Fast Connect !");

	data->devconn->bt.device =(BluetoothDevice*) malloc(sizeof(BluetoothDevice));
	data->devconn->bt.service =(BluetoothService*) malloc(sizeof(BluetoothService));

	sprintf(data->devconn->bt.device->address,FAST_CONNECT_ADD);
	sprintf(data->devconn->bt.device->name , "VEX");
	data->devconn->bt.service->channel = FAST_CONNECT_CHAN;

#else
	// updating devcomm with selected device and service
	int serivce_selected = gtk_combo_box_get_active (data->cb_serv_list);

	// - 1 for the local server offset
	int device_selected = gtk_combo_box_get_active (data->cb_dev_list) - 1;

	data->devconn->bt.device = &data->devices[device_selected];
	data->devconn->bt.service = &data->devices[device_selected].services[serivce_selected];

#endif

	// Create bluetooth Connection

	if(create_comm_connection(data))
	{
		// Create Communication Interface
		printf("Comm interface \n");
		create_comm_interface(data);
		printf("un lock \n");
		g_mutex_unlock (data->devconn->bt.thread_Communication_Mut);
	}

}

/**
 * \fn void callback_send (GtkButton *button,AppData * data)
 * \brief Envoye l'information au périphérique connecté
 *
 * \param[in] widget Bouton send
 * \param[in,out] data Donné du programme
 *
 *	Envoye l'information au périphérique connecté
 *
 */

void callback_send (GtkButton *button,AppData * data){

	if(data->devconn != NULL && button == data->devconn->ui.button)
	{
		char * buffer =(char *) gtk_entry_get_text(data->devconn->ui.entry);
		char * send_buffer = (char*)malloc(sizeof(char)*strlen(buffer));
		strcpy(send_buffer,buffer);
		printf(" : %s\n",send_buffer);
		puts("\n");
		// Getting acces to the send queue
		g_mutex_lock (data->devconn->bt.send_Queue_Mut);

		data->devconn->bt.send_Queue = g_slist_append(data->devconn->bt.send_Queue,
										(char*)send_buffer);

		// releasing acces to the send queue
		g_mutex_unlock (data->devconn->bt.send_Queue_Mut);

		gtk_entry_set_text(data->devconn->ui.entry,"");
	}
	else if(data->locserv != NULL && button == data->locserv->ui.button)
	{
		char * buffer =(char *) gtk_entry_get_text(data->locserv->ui.entry);

		write(data->locserv->bt.sock,buffer,strlen(buffer));

		gtk_entry_set_text(data->locserv->ui.entry,"");
	}

}

/**
 * \fn gboolean callback_hardware_button(GtkWidget * widget, GdkEventKey * event,AppData * data)
 * \brief Traite les évènement liee au bouton physique du PDA
 *
 * \param[in] widget Fenêtre de l'évenement
 * \param[in] event Evenement occouru.
 * \param[in,out] data Donné du programme
 *
 * Traite le passage en pein ecran pour la fenêtre de connection
 *
 */

gboolean callback_hardware_button(GtkWidget * widget, GdkEventKey * event,AppData * data)
{
	if(data->devconn !=NULL && widget == (GtkWidget*)data->devconn->ui.window)
	{
		switch (event->keyval)
		{
			case HILDON_HARDKEY_FULLSCREEN:
				if(!data->devconn->ui.fullscreen)
				{
					gtk_window_fullscreen (GTK_WINDOW (data->devconn->ui.window));
					data->devconn->ui.fullscreen = true;
					display_comm_interface( data );
				}
				else
				{
					gtk_window_unfullscreen (GTK_WINDOW (data->devconn->ui.window));
					data->devconn->ui.fullscreen = false;
					display_comm_interface( data );
				}
			  return true;
		}
	}
	else if(data->locserv !=NULL && widget == (GtkWidget*)data->locserv->ui.window)
	{
		switch (event->keyval)
		{
			case HILDON_HARDKEY_FULLSCREEN:
				if(!data->locserv->ui.fullscreen)
				{
					gtk_window_fullscreen (GTK_WINDOW (data->locserv->ui.window));
					data->locserv->ui.fullscreen = true;
					display_local_server_interface( data );
				}
				else
				{
					gtk_window_unfullscreen (GTK_WINDOW (data->locserv->ui.window));
					data->locserv->ui.fullscreen = false;
					display_local_server_interface( data );
				}
			  return true;
		}
	}
	return false;
}

/**
 * \fn gboolean callback_com_stop(GtkWidget * widget, GdkEvent *event,AppData *data)
 * \brief Boutton STOP du mode plein ecran, clicked, communication terminer
 *
 * \param[in] widget Fenêtre de l'évenement
 * \param[in] event Evenement occouru.
 * \param[in,out] data Donné du programme
 *
 * Renvoie une demande a callback_com_close
 *
 */

// The communication need to be terminated
gboolean callback_com_stop(GtkWidget * widget, GdkEvent *event,AppData *data)
{

	/*GdkEventAny* event_send =(GdkEventAny*) malloc(sizeof(GdkEventAny));
	event_send->type=GDK_NOTHING;
	event_send->window = (GdkWindow* )data->devconn->ui.window;

	callback_com_close((GtkWidget *)data->devconn->ui.window,(GdkEvent *)event_send,data);
*/
	// shutdown communication
	//shutdown(data->devconn->bt.sock,SHUT_RDWR );

	// close the socket

	printf("comm Stop close\n");

	close(data->devconn->bt.sock);


	return TRUE;
}


/**
 * \fn gboolean callback_com_close(GtkWidget * widget, GdkEvent *event,AppData *data)
 * \brief Traite la fermeture de la fenêtre liée à la connection avec le périphérique
 *
 * \param[in] widget Fenêtre de l'évenement
 * \param[in] event Evenement occouru.
 * \param[in,out] data Donné du programme
 *
 * Ferme la connection avec le périphérique et tous ce qui en découle
 *
 */

// The communication window has been closed
gboolean callback_com_close(GtkWidget * widget, GdkEvent *event,AppData *data)
{
	//terminate any other connection
	puts("callback_com_close\n");

	if( data->devconn !=NULL && widget == (GtkWidget *)data->devconn->ui.window )
	{
		puts("Close dev conn\n");
		// close the recving thread
		if(data->devconn->disconnect != NULL)
			g_cond_signal(data->devconn->disconnect);
		puts("Shutdown\n");
		// shutdown communication
		shutdown(data->devconn->bt.sock,SHUT_RDWR );

		// close the socket
		close(data->devconn->bt.sock);
		puts("Close dev conn thread\n");
		if(data->devconn->bt.thread_Communication != NULL)
		{
			//Waiting for thread
			g_thread_join(data->devconn->bt.thread_Communication);
		}
		puts("Close dev conn thread closed\n");
		g_mutex_free (data->devconn->bt.view_Array_Mut);
		g_mutex_free (data->devconn->bt.send_Queue_Mut);
		g_mutex_free (data->devconn->bt.thread_Communication_Mut);

		// close the socket
		//close(data->devconn->bt.sock);

		//remove the old user interface

		//window_main doesn't have the vbox any more it's the data->devconn->ui.window
		//gtk_container_remove(GTK_CONTAINER(data->window_main),GTK_WIDGET(data->devconn->ui.vbox));

		hildon_program_remove_window(data->program,data->devconn->ui.window);
		gtk_widget_hide_all((GtkWidget *) data->devconn->ui.window);
		gtk_widget_destroy ((GtkWidget *) data->devconn->ui.window);
		//free object
		// TODO : free better
		free(data->devconn);
		data->devconn = NULL;

	}

	else if( data->locserv !=NULL && widget == (GtkWidget *)data->locserv->ui.window )
	{
		// close the recving thread
		g_cond_signal(data->locserv->disconnect);
		if(data->locserv->bt.thread_Communication != NULL)
		{
			//Waiting for thread Local server
			g_thread_join(data->locserv->bt.thread_Communication);
		}

		g_mutex_free (data->locserv->bt.view_Array_Mut);
		g_mutex_free (data->locserv->bt.send_Queue_Mut);
		g_mutex_free (data->locserv->bt.thread_Communication_Mut);


		// close the socket
		close(data->locserv->bt.sock);

		//remove the old user interface

		//window_main doesn't have the vbox any more it's the data->devconn->ui.window
		//gtk_container_remove(GTK_CONTAINER(data->window_main),GTK_WIDGET(data->devconn->ui.vbox));

		hildon_program_remove_window(data->program,data->locserv->ui.window);
		gtk_widget_hide_all((GtkWidget *) data->locserv->ui.window);
		gtk_widget_destroy ((GtkWidget *) data->locserv->ui.window);
		//free object
		// TODO : free better
		free(data->locserv);
		data->locserv = NULL;
		//Local Server Closed
		// Unregister the record
		unregister_loc_serv_record(data);
	}
	return true;
}

/**
 * \fn gboolean callback_expose (GtkWidget *widget, GdkEventExpose *event, AppData * data)
 * \brief Traite l'affichage de la fenêtre de communication
 *
 * \param[in] widget Fenêtre de l'évenement
 * \param[in] event Evenement occouru.
 * \param[in,out] data Donné du programme
 *
 *  Dessine l'affichage de la fenêtre de communication avec le périphérique
 *
 */

gboolean callback_expose (GtkWidget *widget, GdkEventExpose *event, AppData * data)
{

	unsigned char * view_Array;
	GdkPoint tab_points[ENV_FOV/ENV_ANGLE_INC+2];

	if(!g_mutex_trylock (data->devconn->bt.view_Array_Mut))
	{
		printf("Mutex Locked\n\n");
		return FALSE;
	}


	view_Array = data->devconn->bt.view_Array;

	// création d'un tableau de points à partir de view_Array

	int j,i;
	for (j=0; j<= ENV_FOV/ENV_ANGLE_INC; j++)
	{
		tab_points[j].x=SCREEN_ORIGIN_X+(int)(view_Array[j]*300/200)*cos((3.14/180)*(j*ENV_ANGLE_INC+40));  // x = r.cos(a)
		tab_points[j].y=SCREEN_ORIGIN_Y-(int)(view_Array[j]*300/200)*sin((3.14/180)*(j*ENV_ANGLE_INC+40));  // y = r.sin(a)
	}
	tab_points[j].x=SCREEN_ORIGIN_X;
	tab_points[j].y=SCREEN_ORIGIN_Y;

	// Graphic Context
	GdkColor BLANC;
	BLANC.pixel = 0;
	BLANC.red = 0xFFFF;
	BLANC.green = 0xFFFF;
	BLANC.blue = 0xFFFF;
	GdkColor GRIS;
	GRIS.pixel = 0;
	GRIS.red = 0xAAAA;
	GRIS.green = 0xAAAA;
	GRIS.blue = 0xAAAA;
	GdkColor NOIR;
	NOIR.pixel = 32;
	NOIR.red = 0;
	NOIR.green = 0;
	NOIR.blue = 0;

	gdk_gc_set_function(widget->style->fg_gc[GTK_WIDGET_STATE (widget)],GDK_COPY);
	gdk_gc_set_rgb_fg_color (widget->style->fg_gc[GTK_WIDGET_STATE (widget)], &NOIR);
	gdk_gc_set_line_attributes (widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
				  1, GDK_LINE_SOLID, GDK_CAP_PROJECTING, GDK_JOIN_MITER);

	// dessiner les arcs, lignes, points...
	gdk_draw_arc (widget->window,
		widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		TRUE,
		0, 50, widget->allocation.width, widget->allocation.width,
		64 * (180-ENV_FOV)/2, 64 * ENV_FOV);
	gdk_gc_set_rgb_fg_color (widget->style->fg_gc[GTK_WIDGET_STATE (widget)], &BLANC);


	gdk_draw_polygon (widget->window, widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
				  TRUE,
				  tab_points,
				  ENV_FOV/ENV_ANGLE_INC+2);


	gdk_gc_set_rgb_fg_color (widget->style->fg_gc[GTK_WIDGET_STATE (widget)], &GRIS);
	gdk_draw_line(widget->window, widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
				  0,SCREEN_ORIGIN_Y,
				  widget->allocation.width,SCREEN_ORIGIN_Y);
	gdk_draw_line(widget->window, widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
				  SCREEN_ORIGIN_X, 0,
				  SCREEN_ORIGIN_X, widget->allocation.height);
	gdk_draw_line(widget->window, widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
				  SCREEN_ORIGIN_X,SCREEN_ORIGIN_Y,
				  SCREEN_ORIGIN_X+350*cos((2*3.14/360)*(180-ENV_FOV)/2),SCREEN_ORIGIN_Y-350*sin((2*3.14/360)*(180-ENV_FOV)/2));
	gdk_draw_line(widget->window, widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
				  SCREEN_ORIGIN_X,SCREEN_ORIGIN_Y,
				  SCREEN_ORIGIN_X+350*cos((2*3.14/360)*(180-(180-ENV_FOV)/2)),SCREEN_ORIGIN_Y-350*sin((2*3.14/360)*(180-(180-ENV_FOV)/2)));
	gdk_gc_set_rgb_fg_color (widget->style->fg_gc[GTK_WIDGET_STATE (widget)], &NOIR);

	g_mutex_unlock (data->devconn->bt.view_Array_Mut);

	return TRUE;
}

/**
 * \fn gboolean callback_mouse (GtkWidget *widget,GdkEvent *event,AppData *data)
 * \brief Traite les évenements de la souris
 *
 * \param[in] widget Fenêtre de l'évenement
 * \param[in] event Evenement occouru.
 * \param[in,out] data Donné du programme
 *
 *  Traite les évenements de la souris liée à la fenêtre de communication avec le périphérique
 *
 */

gboolean callback_mouse (GtkWidget *widget,GdkEvent *event,AppData *data)
{
	static gboolean pressed = false;
	static unsigned char* send_buffer = NULL;
	static float x;
	static float y;

	switch (event->type)
	{

		case GDK_BUTTON_PRESS :
			pressed = true;
			// Keep going, sending the first data
		case GDK_MOTION_NOTIFY :
			if (pressed)
			{
				// Getting acces to the send queue
				if(g_mutex_trylock(data->devconn->bt.send_Queue_Mut))
				{
					//printf("PRESSED : Delpacement Data Adding\n");

					gdk_gc_set_line_attributes (widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
					  1, GDK_LINE_ON_OFF_DASH, GDK_CAP_PROJECTING, GDK_JOIN_MITER);

					gdk_gc_set_function(widget->style->fg_gc[GTK_WIDGET_STATE (widget)],GDK_INVERT);

					gdk_draw_line (widget->window,
								widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
								SCREEN_ORIGIN_X,
								SCREEN_ORIGIN_Y,
								(int)((GdkEventMotion*)event)->x,
								(int)((GdkEventMotion*)event)->y);
					x = ((GdkEventMotion*)event)->x - SCREEN_ORIGIN_X;
					y = -((GdkEventMotion*)event)->y + SCREEN_ORIGIN_Y;

					//printf("x : %f , y : %f\n",x,y);

					x = x/(widget->allocation.width-SCREEN_ORIGIN_X) * 128 + 127;

					if( y > 0 )
					{
						y = y/(SCREEN_ORIGIN_Y) * 128 + 127;
					}
					else
					{
						y = y/(widget->allocation.height-SCREEN_ORIGIN_Y) * 128 + 127;
					}
					//printf("Compute x : %f , y : %f\n",x,y);

					if(x>255)
					{
						x=255;
					}

					send_buffer =(unsigned char*) malloc(sizeof(char)*(1+CMD_DPL_LEN));
					send_buffer[0] = CMD_DPL;

					send_buffer[1]=(unsigned char)x;
					send_buffer[2]=(unsigned char)y;

					//printf("Buffer : %d %d %d\n",send_buffer[0],send_buffer[1],send_buffer[2]);

					// Emptiing the queue
					while(data->devconn->bt.send_Queue!=NULL)
					{
						// free the data link to the element of the queue
						free(data->devconn->bt.send_Queue->data);
						// remove the top element of the queue
						data->devconn->bt.send_Queue=g_slist_delete_link (data->devconn->bt.send_Queue,
												data->devconn->bt.send_Queue);
					}

					data->devconn->bt.send_Queue = g_slist_append(data->devconn->bt.send_Queue,
												send_buffer);

					// releasing acces to the send queue
					g_mutex_unlock(data->devconn->bt.send_Queue_Mut);
					// Buffer queued, reset pointer
					send_buffer = NULL;
				}
			}
			break;
		case GDK_BUTTON_RELEASE :
			pressed = false;
			send_buffer =(unsigned char*) malloc(sizeof(char)*(1+CMD_DPL_LEN));
			send_buffer[0] = CMD_DPL;
			send_buffer[1]=(unsigned char)127; // idle
			send_buffer[2]=(unsigned char)127; // idle

			g_mutex_lock(data->devconn->bt.send_Queue_Mut);

			data->devconn->bt.send_Queue = g_slist_append(data->devconn->bt.send_Queue,
												send_buffer);

					// releasing acces to the send queue
			g_mutex_unlock(data->devconn->bt.send_Queue_Mut);
					// Buffer queued, reset pointer
			send_buffer = NULL;

			break;
		default :
			return false;
	}

	return true;
}


/**
 * \fn gboolean callback_entry_keys(GtkWidget *entry,GdkEventKey *event,AppData *data)
 * \brief Gére l'envoie d'information avec la touche return ( entrée )
 *
 * \param[in] widget Fenêtre de l'évenement
 * \param[in] event Evenement occouru.
 * \param[in,out] data Donné du programme
 *
 *  Gére l'envoie d'informations avec la touche return ( entrée )
 *
 */

gboolean callback_entry_keys(GtkWidget *entry,GdkEventKey *event,AppData *data)
{
    if(event->keyval == GDK_Return)
    {
        if(data->devconn != NULL && (GtkWidget*)data->devconn->ui.entry == entry)
        {
                callback_send (data->devconn->ui.button,data);
        }
        else  if(data->locserv != NULL && (GtkWidget*)data->locserv->ui.entry == entry)
        {
                callback_send (data->locserv->ui.button,data);
        }
        return true;
    }

    return false;
}
