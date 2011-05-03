/**
 * \file interface.c
 * \brief Fichier contenant les fonctions relative à l'interface
 * \author Stanislas BERTRAND
 * \version 0.1
 * \date 24 mars 2011
 */


#include <gtk/gtk.h>

#include "interface.h"
#include "callbacks.h"
#include "../appdata.h"
#include "../bt/bluetooth.h"

#ifndef PACKAGE
#define PACKAGE "Vex"
#endif

#ifndef VERSION
#define VERSION "1.0.42"
#endif


#define OSSO_SERVICE "org.maemo."PACKAGE

AppData * create_data()
{
	AppData * data = g_new0(AppData,1);
	data->setting_uuid = 0x1002;

	//data->osso = osso_initialize ( OSSO_SERVICE, VERSION, TRUE, NULL );
	//g_assert ( data->osso );

	return data;
}

void create_user_interface(AppData * data)
{
	/* Create HildonWindow and set it to HildonProgram */
	data->window_main = HILDON_WINDOW(hildon_window_new());
	hildon_program_add_window(data->program, data->window_main);

	g_signal_connect(G_OBJECT(data->window_main),
		"key_press_event", G_CALLBACK(callback_hardware_button),data);

#ifndef FAST_CONNECT
	create_search_toolbar(data);
	create_options_toolbar(data);
	gtk_widget_hide_all((GtkWidget *)data->tb_options);
#else
	create_services_toolbar(data);
#endif
}

void info_display(char * info,GtkWidget * window,const char * file,int line)
{
	static char msg[100];
	printf("%s @ %s %d \nErrno : %d ( %s )\n",info,file,line,errno,strerror(errno) );
	sprintf(msg,"%s Errno : %d (%s)",info,errno,strerror(errno));
	hildon_banner_show_information(window,NULL,msg);
}

/*
* search toolbar for bluetooth device search
* Content : search, options, progress, device list, close
*/

void create_search_toolbar(AppData * data)
{

	GtkToolbar *tb_search;

	GtkToolItem *it_dev_list;
	GtkComboBox *cb_dev_list;

	GtkToolItem *it_search;
	GtkToolItem *it_options;
	GtkToolItem *it_close;



	/* Create toolbar */
	tb_search = (GtkToolbar*) gtk_toolbar_new();

	/* Create toolbar button items */
	it_search = gtk_tool_button_new_from_stock(GTK_STOCK_FIND);
	it_options = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_PREFERENCES);
		//gtk_tool_button_new_from_stock(GTK_STOCK_PREFERENCES);
	it_close = gtk_tool_button_new_from_stock(GTK_STOCK_CLOSE);

	/* Create the Combo Box */

	it_dev_list = gtk_tool_item_new();
	cb_dev_list = GTK_COMBO_BOX(gtk_combo_box_new_text());
	data->cb_dev_list = cb_dev_list;
	/* Make the ComboBox use all available toolbar space */
	gtk_tool_item_set_expand(it_dev_list,TRUE);
	/* Add Progress Bar inside toolitem */
	gtk_container_add(GTK_CONTAINER(it_dev_list), GTK_WIDGET(cb_dev_list));

	gtk_combo_box_append_text(cb_dev_list,"Local Server");

	gtk_toolbar_insert(tb_search, it_search, -1);
	gtk_toolbar_insert(tb_search, it_options, -1);
	gtk_toolbar_insert(tb_search, it_dev_list, -1);
	gtk_toolbar_insert(tb_search, it_close, -1);

	/* When user change selected device */
	g_signal_connect(G_OBJECT(cb_dev_list), "changed",
			G_CALLBACK(callback_dev_selected), data);

	/* Connect close item to quit */
	g_signal_connect(G_OBJECT(it_close), "clicked",
			G_CALLBACK(gtk_main_quit), NULL);

#ifdef FAST_CONNECT

	/* Connect search item to search bluetooth device */
	g_signal_connect(G_OBJECT(it_search), "clicked",
			G_CALLBACK( callback_connect/*  callback_search_dev*/),data);


#else

	/* Connect search item to search bluetooth device */
	g_signal_connect(G_OBJECT(it_search), "clicked",
			G_CALLBACK(callback_search_dev),data);

#endif


	/* Connect option item to displaying options toolbar */
	g_signal_connect(G_OBJECT(it_options), "toggled",
			G_CALLBACK(callback_display_options),data);

	/* Add toolbar HildonWindow */
	hildon_window_add_toolbar(data->window_main, tb_search);


	gtk_widget_show_all((GtkWidget*) tb_search);

}


void create_services_toolbar(AppData * data)
{
	int i;
	GtkToolbar *tb_services;

	GtkToolItem *it_label;
	GtkLabel *lb_label;

	GtkToolItem *it_serv_list;
	GtkComboBox *cb_serv_list;

	GtkToolItem * it_connect;

	/* Remove old toolbar before adding a new one */
	if (data->tb_services != NULL)
		hildon_window_remove_toolbar(data->window_main, data->tb_services);

	/* Create toolbar */
	tb_services = (GtkToolbar*) gtk_toolbar_new();
	data->tb_services = tb_services;
	it_connect = gtk_tool_button_new_from_stock(GTK_STOCK_CONNECT);

	/* Create the label */
	it_label = gtk_tool_item_new();
	lb_label = (GtkLabel*) gtk_label_new("Service ");
	gtk_container_add(GTK_CONTAINER(it_label), GTK_WIDGET(lb_label));

	/* Create the Combo Box */

	it_serv_list = gtk_tool_item_new();
	cb_serv_list = GTK_COMBO_BOX(gtk_combo_box_new_text());
	data->cb_serv_list = cb_serv_list;

	#ifndef FAST_CONNECT

	// Adding services
	int device = gtk_combo_box_get_active(data->cb_dev_list);

	if( device == 0 )
	{
		// local device selected.
		//hildon_banner_show_information((GtkWidget *)data->window_main,NULL,"Local Server !");

		//create_app_bt_service(data);

		// MOVED TO THE CALLBACK AS A FUNCTION

	}
	else
	{
		device--; // for array matching
		for(i=0 ; i < data->devices[device].number_services ; i++)
		{
			gtk_combo_box_append_text(cb_serv_list,data->devices[device].services[i].name );

		}
	}

	#endif

	/* Make the ComboBox use all available toolbar space */
	gtk_tool_item_set_expand(it_serv_list,TRUE);
	/* Add Progress Bar inside toolitem */
	gtk_container_add(GTK_CONTAINER(it_serv_list), GTK_WIDGET(cb_serv_list));

	gtk_toolbar_insert(tb_services,it_label,-1);
	gtk_toolbar_insert(tb_services,it_serv_list,-1);
	gtk_toolbar_insert(tb_services,it_connect,-1);

	/* Connect search item to search bluetooth device */
	g_signal_connect(G_OBJECT(it_connect), "clicked",
			G_CALLBACK(callback_connect),data);

	/* Add toolbar HildonWindow */
	hildon_window_add_toolbar(data->window_main, tb_services);

	gtk_widget_show_all((GtkWidget*) tb_services);

}

void create_options_toolbar(AppData * data)
{
	GtkToolbar *tb_options;

	GtkToolItem *it_label;
	GtkLabel *lb_label;

	GtkToolItem *it_uuid_list;
	GtkComboBoxEntry *cb_uuid_list;

	/* Create toolbar */
	tb_options = (GtkToolbar*) gtk_toolbar_new();
	data->tb_options = tb_options;


	/* Create the label */
	it_label = gtk_tool_item_new();
	lb_label = (GtkLabel*) gtk_label_new("UUID ");
	gtk_container_add(GTK_CONTAINER(it_label), GTK_WIDGET(lb_label));

	/* Create the Combo Box */

	it_uuid_list = gtk_tool_item_new();
	cb_uuid_list = (GtkComboBoxEntry *)gtk_combo_box_entry_new_text();

	// Adding services

	int i;
	char uuid_name[50];

	for(i = 0 ; i< BT_NUMBER_UUID_VALUES ; i++ )
	{
		sprintf(uuid_name,"%s - 0x%04X",uuid_strings[i],uuid_values[i]);
		gtk_combo_box_append_text((GtkComboBox*)cb_uuid_list,uuid_name );
	}


	/* Make the ComboBox use all available toolbar space */
	gtk_tool_item_set_expand(it_uuid_list,TRUE);
	/* Add Progress Bar inside toolitem */
	gtk_container_add(GTK_CONTAINER(it_uuid_list), GTK_WIDGET(cb_uuid_list));

	/* When user change selected uuid */
	g_signal_connect(G_OBJECT(cb_uuid_list), "changed",
			G_CALLBACK(callback_uuid_changed), data);


	gtk_toolbar_insert(tb_options,it_label,-1);
	gtk_toolbar_insert(tb_options,it_uuid_list,-1);

	//gtk_widget_hide_all((GtkWidget *)tb_options);

	/* Add toolbar HildonWindow */
	hildon_window_add_toolbar(data->window_main, tb_options);

	//gtk_widget_hide_all((GtkWidget *)tb_options);


}



void create_comm_interface( AppData * data)
{
	GtkScrolledWindow * scroll;
	GtkWidget *view;
	GtkTextBuffer *buffer;
	GtkEntry * entry;
	GtkVBox * vbox;
	GtkHBox * hbox;
	GtkButton * button;


	button = (GtkButton *) gtk_button_new_with_label ("Send");
	data->devconn->ui.button = button;
	/* Connect to the clicked signal*/
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK( callback_send ),data);


	vbox = (GtkVBox *) gtk_vbox_new(FALSE,0);
	hbox = (GtkHBox *) gtk_hbox_new(FALSE,0);

	scroll = (GtkScrolledWindow *) gtk_scrolled_window_new (NULL,NULL);

	view = gtk_text_view_new ();

	gtk_text_view_set_editable (GTK_TEXT_VIEW(view),false);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	data->devconn->ui.text_buffer = buffer;

	//gtk_text_buffer_set_text (buffer, "Hello, this is some text", -1);

	gtk_scrolled_window_add_with_viewport (scroll,view);

	entry = (GtkEntry *) gtk_entry_new();
	data->devconn->ui.entry = entry;

	g_signal_connect(G_OBJECT(entry),
		"key_press_event", G_CALLBACK(callback_entry_keys), data);

	gtk_box_pack_start((GtkBox *)hbox,GTK_WIDGET(entry),TRUE,TRUE,5);
	gtk_box_pack_start((GtkBox *)hbox,GTK_WIDGET(button),FALSE,FALSE,5);
	gtk_box_pack_start_defaults((GtkBox *)vbox,GTK_WIDGET(scroll));
	gtk_box_pack_start((GtkBox *)vbox,GTK_WIDGET(hbox),FALSE,FALSE,5);

	// Create Comm Window

	HildonWindow* window_conn = (HildonWindow*) hildon_window_new();
	data->devconn->ui.window = window_conn;
	hildon_program_add_window (data->program,data->devconn->ui.window);
	g_signal_connect(G_OBJECT(data->devconn->ui.window),
		"key_press_event", G_CALLBACK(callback_hardware_button), data);
	data->devconn->ui.fullscreen = false;
	g_signal_connect(G_OBJECT(data->devconn->ui.window), "delete_event",
			G_CALLBACK(callback_com_close), data);


	data->devconn->ui.vbox = vbox;
	data->devconn->ui.text_view =(GtkTextView *) view;
	data->devconn->ui.drawing_area = NULL;
	display_comm_interface(data);
}

void display_comm_interface( AppData * data)
{
	static GtkWidget *drawing_area;
	GtkWidget *VBox = NULL;
	static GtkWidget *HBox = NULL;
	GtkWidget *Button[4];
	GtkWidget *Separator = NULL;
	GdkColor NOIR;
      NOIR.pixel = 32;
      NOIR.red = 0;
      NOIR.green = 0;
      NOIR.blue = 0;


	// Base on the state of the window the ui is different.
	if (data->devconn->ui.fullscreen)
	{
		g_object_ref (data->devconn->ui.vbox);
		gtk_container_remove(GTK_CONTAINER(data->devconn->ui.window),GTK_WIDGET(data->devconn->ui.vbox));

		/* Création d'une HBOX dans laquelle on insère une drawing_area et une VBOX contenant des boutons */
		HBox = gtk_hbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(data->devconn->ui.window), HBox);
		VBox = gtk_vbox_new(TRUE, 0);
		//boutons
		Button[0] = gtk_button_new_with_label("Bt 1");
		Button[1] = gtk_button_new_with_label("STOP");

		g_signal_connect(G_OBJECT(Button[1]), "clicked",
			G_CALLBACK( callback_com_stop ),data);

		gtk_box_pack_start(GTK_BOX(VBox), Button[0], TRUE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(VBox), Button[1], TRUE, FALSE, 0);

		drawing_area = gtk_drawing_area_new ();
		data->devconn->ui.drawing_area = drawing_area;
		gtk_widget_set_size_request (drawing_area, DRAWING_AREA_WIDTH, DRAWING_AREA_HEIGTH);

		g_signal_connect (G_OBJECT (drawing_area),
					"expose_event",
					G_CALLBACK (callback_expose), data); //TODO CHECK if accurate
		g_signal_connect(G_OBJECT (drawing_area),
					"motion_notify_event",
					G_CALLBACK(callback_mouse), data);
		g_signal_connect(G_OBJECT (drawing_area),
					"button_press_event",
					G_CALLBACK(callback_mouse), data);
		g_signal_connect(G_OBJECT (drawing_area),
					"button_release_event",
					G_CALLBACK(callback_mouse), data);

		gtk_widget_set_events(GTK_WIDGET(drawing_area),
						GDK_EXPOSURE_MASK |
						GDK_BUTTON_PRESS_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_POINTER_MOTION_MASK);


		gtk_box_pack_start(GTK_BOX(HBox), drawing_area, TRUE, FALSE, 0);
		//séparateur
		Separator = gtk_vseparator_new();
		gtk_box_pack_start(GTK_BOX(HBox), Separator, TRUE, FALSE, 1);
		//VBOX
		gtk_box_pack_start(GTK_BOX(HBox), VBox, TRUE, TRUE, 0);

	}
	else
	{
		gtk_container_remove(GTK_CONTAINER(data->devconn->ui.window),GTK_WIDGET(HBox));
		gtk_container_add(GTK_CONTAINER(data->devconn->ui.window),GTK_WIDGET(data->devconn->ui.vbox));

	}
	gtk_widget_show_all(GTK_WIDGET(data->devconn->ui.window));
}

void create_local_server_interface( AppData * data)
{
	GtkScrolledWindow * scroll;
	GtkWidget *view;
	GtkTextBuffer *buffer;
	GtkEntry * entry;
	GtkVBox * vbox;
	GtkHBox * hbox;
	GtkButton * button;


	data->locserv = g_new0(DeviceConnection,1);

	button = (GtkButton *) gtk_button_new_with_label ("Send");
	data->locserv->ui.button = button;
	/* Connect to the clicked signal*/
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(callback_send ),data);


	vbox = (GtkVBox *) gtk_vbox_new(FALSE,0);
	hbox = (GtkHBox *) gtk_hbox_new(FALSE,0);

	scroll = (GtkScrolledWindow *) gtk_scrolled_window_new (NULL,NULL);

	view = gtk_text_view_new ();

	gtk_text_view_set_editable (GTK_TEXT_VIEW(view),false);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	data->locserv->ui.text_buffer = buffer;

	//gtk_text_buffer_set_text (buffer, "Hello, this is some text", -1);

	gtk_scrolled_window_add_with_viewport (scroll,view);

	entry = (GtkEntry *) gtk_entry_new();
	data->locserv->ui.entry = entry;

      g_signal_connect(G_OBJECT(entry),
		"key_press_event", G_CALLBACK(callback_entry_keys), data);


	gtk_box_pack_start((GtkBox *)hbox,GTK_WIDGET(entry),TRUE,TRUE,5);
	gtk_box_pack_start((GtkBox *)hbox,GTK_WIDGET(button),FALSE,FALSE,5);
	gtk_box_pack_start_defaults((GtkBox *)vbox,GTK_WIDGET(scroll));
	gtk_box_pack_start((GtkBox *)vbox,GTK_WIDGET(hbox),FALSE,FALSE,5);

	// Create Comm Window

	HildonWindow* window_conn = (HildonWindow*) hildon_window_new();
	data->locserv->ui.window = window_conn;
	hildon_program_add_window (data->program,data->locserv->ui.window);
	g_signal_connect(G_OBJECT(data->locserv->ui.window),
		"key_press_event", G_CALLBACK(callback_hardware_button), data);
	data->locserv->ui.fullscreen = false;
	g_signal_connect(G_OBJECT(data->locserv->ui.window), "delete_event",
			G_CALLBACK(callback_com_close), data);


	data->locserv->ui.vbox = vbox;
	data->locserv->ui.text_view =(GtkTextView *) view;
	display_local_server_interface(data);
}

void display_local_server_interface(AppData * data)
{

	static GtkWidget *drawing_area;
	// Base on the state of the window the ui is different.
	if (data->locserv->ui.fullscreen)
	{
		g_object_ref (data->locserv->ui.vbox);
		gtk_container_remove(GTK_CONTAINER(data->locserv->ui.window),GTK_WIDGET(data->locserv->ui.vbox));

		// TODO : Think about local server interface.

		drawing_area = gtk_drawing_area_new ();
            data->locserv->ui.drawing_area = drawing_area;
		gtk_widget_set_size_request (drawing_area, 100, 100);

		g_signal_connect (G_OBJECT (drawing_area),
					"expose_event",
					G_CALLBACK (callback_mouse), data);
		g_signal_connect(G_OBJECT (drawing_area),
					"motion_notify_event",
					G_CALLBACK(callback_mouse), data);
		g_signal_connect(G_OBJECT (drawing_area),
					"button_press_event",
					G_CALLBACK(callback_mouse), data);
		g_signal_connect(G_OBJECT (drawing_area),
					"button_release_event",
					G_CALLBACK(callback_mouse), data);

		gtk_widget_set_events(drawing_area,
						GDK_EXPOSURE_MASK |
						GDK_BUTTON_PRESS_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_POINTER_MOTION_MASK);



		gtk_container_add(GTK_CONTAINER(data->locserv->ui.window),GTK_WIDGET(drawing_area));
	}
	else
	{
		gtk_container_remove(GTK_CONTAINER(data->locserv->ui.window),GTK_WIDGET(drawing_area));
		gtk_container_add(GTK_CONTAINER(data->locserv->ui.window),GTK_WIDGET(data->locserv->ui.vbox));

	}
	gtk_widget_show_all(GTK_WIDGET(data->locserv->ui.window));
}
