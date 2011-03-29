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

	create_comm_interface(data);  //TODO check if accurate
	return;


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
		GdkEventAny* event = new GdkEventAny;
		event->type=GDK_NOTHING;
		event->window = (GdkWindow* )data->devconn->ui.window;

		callback_com_close((GtkWidget *)data->devconn->ui.window,(GdkEvent *)event,data);
	}

	//Create device connection data
	data->devconn = (DeviceConnection*) malloc(sizeof(DeviceConnection));

#ifdef FAST_CONNECT

	hildon_banner_show_information((GtkWidget *)data->window_main,NULL,"Fast Connect !");

	data->devconn->bt.device = new BluetoothDevice;
	data->devconn->bt.service = new BluetoothService;

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
		create_comm_interface(data);
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
		// close the recving thread
		g_cond_signal(data->devconn->disconnect);
		if(data->devconn->bt.thread_Communication != NULL)
		{
			//Waiting for thread
			g_thread_join(data->devconn->bt.thread_Communication);
		}

		g_mutex_free (data->devconn->bt.view_Array_Mut);
		g_mutex_free (data->devconn->bt.send_Queue_Mut);
		g_mutex_free (data->devconn->bt.thread_Communication_Mut);

		// close the socket
		close(data->devconn->bt.sock);

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
		delete data->locserv;
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

	gdk_draw_arc (widget->window,
                widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                TRUE,
                0, 0, widget->allocation.width, widget->allocation.height,
                0, 64 * 360);

	gdk_draw_line (widget->window,
                widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			210,
                200,400,600);

	gdk_draw_point(widget->window,
                widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			210,
                200);
	gdk_draw_point(widget->window,
                widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			200,
                210);
	gdk_draw_point(widget->window,
                widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			210,
                210);
	gdk_draw_point(widget->window,
                widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			205,
                205);



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
    static unsigned char joyInfo[9];
    static bool sending =  false;
	switch (event->type)
	{
		case GDK_MOTION_NOTIFY :

		//	gdk_draw_point(widget->window,
		//			    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		//				(int)((GdkEventMotion*)event)->x,
		//			    (int)((GdkEventMotion*)event)->y);

		gdk_gc_set_function(widget->style->fg_gc[GTK_WIDGET_STATE (widget)], GDK_INVERT);
		gdk_draw_line (widget->window,widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                                       widget->allocation.width/2,widget->allocation.height/2,
                                      (int)((GdkEventMotion*)event)->x,
                                        (int)((GdkEventMotion*)event)->y);

                  double x,y;
                  x = ((GdkEventMotion*)event)->x - widget->allocation.width/2 + 255 ;
                  y = ((GdkEventMotion*)event)->y - widget->allocation.height/2 + 255 ;
                  x/=2;
                  y/=2;

                  // limit to 0 - 255
                  if(x<0)
                      x = 0;
                  else if(x>255)
                      x = 255;
                  if(y<0)
                      y = 0;
                  else if(y>255)
                      y = 255;



              if(data->devconn != NULL && sending)
              {
                    write(data->devconn->bt.sock,(char *)joyInfo,9);
              }

			break;
		case GDK_BUTTON_PRESS :
                  gtk_widget_queue_draw_area      (widget,
                                              0,
                                             0,
                                              800,
                                              480);
			break;
		case GDK_2BUTTON_PRESS :
			gdk_draw_point(widget->window,
					    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
						(int)((GdkEventMotion*)event)->x,
					    (int)((GdkEventMotion*)event)->y);
			break;
		case GDK_3BUTTON_PRESS :
			break;
		case GDK_BUTTON_RELEASE :




              if(data->devconn != NULL && sending)
              {
                    write(data->devconn->bt.sock,(char *)joyInfo,9);
                    write(data->devconn->bt.sock,(char *)joyInfo,9);
                    write(data->devconn->bt.sock,(char *)joyInfo,9);
              }

                  sending = false;

			break;
		case GDK_EXPOSE :
                        GdkGC * gc =  gdk_gc_new (widget->window);
                        gdk_gc_set_function (gc,GDK_AND);
                        gdk_draw_line (widget->window,gc,0,widget->allocation.height/2,
                                        widget->allocation.width,widget->allocation.height/2);

                        gdk_draw_line (widget->window,gc,widget->allocation.width/2,0,
                                        widget->allocation.width/2,widget->allocation.height);
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
