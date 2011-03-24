/**
 * \file btthread.c
 * \brief Fichier contenant des fonctions relatives au thread Bluetooth
 * \author Stanislas BERTRAND
 * \version 0.1
 * \date 24 mars 2011
 */

#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <stdlib.h>

#include "btthread.h"

#include "bluetooth.h"


/**
 * \fn gpointer thread_bluetooth_inquiry(AppData* data)
 * \brief Obtient les périphériques
 *
 * \param[in,out] data Donné du programme
 *
 * Thread pour la recherche des périphériques
 *
 */

gpointer thread_bluetooth_inquiry(AppData* data)
{
	get_info(data);

	gtk_combo_box_append_text(data->cb_dev_list,"Local Server");

	for(int i = 0 ;i < data->number_devices;i++ )
	{
		char device[35];
		sprintf(device,"%s - %s",data->devices[i].name,data->devices[i].address);
		gtk_combo_box_append_text(data->cb_dev_list, device);
	}

	g_cond_signal (data->cond_search_finish);

	return NULL;
}
gchar *convert_utf_string(char *t)
{
    static gchar *s = NULL;
    GError *error = NULL;
    gsize r_bytes, w_bytes;
    unsigned char *c;
    const char *fc;
    gchar *from_codeset="ISO-8859-1";

    if(!t)
        g_assert_not_reached();
    if (g_utf8_validate (t,-1,NULL))
        return t;

    /* so we got a non-UTF-8 */

        g_get_charset(&fc);
        if (fc)
        from_codeset = g_strdup(fc);
        else
        from_codeset = g_strdup("ISO-8859-1");


    if (!strcmp(from_codeset,"ISO-"))
    {
            g_free(from_codeset);
            from_codeset = g_strdup("ISO-8859-1");
    }
    if(s)
      g_free(s);

    for(c = (unsigned char *)t; *c != 0; c++)
        if(*c < 32 && *c != '\n')
            *c = ' ';
    s = g_convert (t,strlen(t),"UTF-8",from_codeset,&r_bytes,
                        &w_bytes,&error);
    g_free(from_codeset);

    if(!s)
    {
        s=g_strdup(t);
        for(c =(unsigned char *) s; *c != 0; c++) if(*c > 128) *c = '?';
    }
    if(error){
        printf("ERR: %s. Codeset for system is: %s\n",
                        error->message,from_codeset);
        g_error_free(error);
    }
    return s;
}


/**
 * \fn void thread_recv_func(AppData *data)
 * \brief Obtient les messages de la communication
 *
 * \param[in,out] data Donné du programme
 *
 * Thread pour recevoir les message de la communication
 *
 */

void thread_recv_func(AppData *data)
{
	char buffer[101];
	int status=0;
	gchar * out;
	GtkTextIter  iter;

	GTimeVal  wait;
	GMutex *wait_mut = g_mutex_new ();

	g_mutex_lock (wait_mut);
	do{
          while(status != -1)
          {
            memset(buffer,0,101);
		//read(data->devconn->bt.sock,buffer,10);

		status = recv(data->devconn->bt.sock,buffer,100,MSG_DONTWAIT);

		if(status == -1){
			if(errno!=EAGAIN)
			{
				printf("Recv Error : %d %s",errno,strerror(errno));
				char info[50];
				sprintf(info,"Recv Error : %d %s",errno,strerror(errno) );
				gdk_threads_enter();
				hildon_banner_show_information((GtkWidget *)data->window_main,NULL,info);
				gdk_threads_leave();
			}
		}
		else{

			//gtk_text_buffer_get_iter_at_offset (data->devconn->ui.text_buffer,&iter,-1);
			//gtk_text_buffer_insert(data->devconn->ui.text_buffer,&iter,convert_utf_string(buffer),-1);
			out = convert_utf_string(buffer);

			//gtk_text_buffer_set_text(data->devconn->ui.text_buffer,out, -1);

			// Make sure we can update user interface without errors
			gdk_threads_enter();

			/* get end iter */
			//gtk_text_buffer_get_end_iter (data->devconn->ui.text_buffer, &iter);

			/* add the message to the text buffer */
			gtk_text_buffer_set_text     (data->devconn->ui.text_buffer, out, status);

			/* scroll to end iter */
			//gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (data->devconn->ui.text_view),
			//					&iter, 0.0, FALSE, 0, 0);

			// Let the main loop take over for user interface
			gdk_threads_leave();

		}
          }
		g_get_current_time (&wait);
		g_time_val_add (&wait,1000);

	}while(!g_cond_timed_wait ( data->devconn->disconnect,wait_mut, &wait ));

	g_mutex_unlock (wait_mut);

	g_mutex_free(wait_mut);

	// Don't know what this is doing there
	//gtk_widget_destroy((GtkWidget*) data->banner_progress);

	g_cond_free(data->devconn->disconnect);
	data->devconn->disconnect = NULL;

	return;
}


unsigned char Limit_Mix (int intermediate_value)
{
  static int limited_value;

  if (intermediate_value < 2000)
  {
    limited_value = 2000;
  }
  else if (intermediate_value > 2254)
  {
    limited_value = 2254;
  }
  else
  {
    limited_value = intermediate_value;
  }
  return (unsigned char) (limited_value - 2000);
}

/**
 * \fn void thread_local_server(AppData * data)
 * \brief Serveur pour le profil local
 *
 * \param[in,out] data Donné du programme
 *
 * Thread pour la gestion du profile local
 *
 */

void thread_local_server(AppData * data)
{
	struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
	int s, bytes_read;
	socklen_t opt = sizeof(rem_addr);

	// allocate socket
	s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	// bind socket to port 1 of the first available
	// local bluetooth adapter
	loc_addr.rc_family = AF_BLUETOOTH;
	loc_addr.rc_bdaddr = *BDADDR_ANY;
	loc_addr.rc_channel = (uint8_t) LOCAL_SERVER_CHANNEL;
	bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

	// put socket into listening mode
	listen(s, 1);

	// accept one connection
	data->locserv->bt.sock = accept(s, (struct sockaddr *)&rem_addr, &opt);

	// Get information from the device

	 #define BUFFER_LEN 20
      char buffer[BUFFER_LEN];
	int status;
	gchar * out;
	GtkTextIter  iter;

	ba2str( &rem_addr.rc_bdaddr, buffer );
	fprintf(stderr, "accepted connection from %s\n", buffer);
	printf("accepted connection from %s\n", buffer);
	memset(buffer, 0, sizeof(buffer));


	GTimeVal  wait;
	GMutex *wait_mut = g_mutex_new ();

	data->locserv->disconnect = g_cond_new ();

	g_mutex_lock (wait_mut);

      char read[1];
      unsigned char joy[10];
      char CurrentMode=0;
      int control,MOTOR_D,MOTOR_G;
      int addIndex=0;
      int readIndex=0;
      char processMode;

	do{

            while(addIndex<BUFFER_LEN)
		{

                  status = recv(data->locserv->bt.sock,read,1,MSG_DONTWAIT);
                    if(status == -1){
                          if(errno!=EAGAIN)
                          {

                                printf("Local Recv Error : %d %s",errno,strerror(errno) );
                                char info[50];
                                sprintf(info,"Local Recv Error : %d %s",errno,strerror(errno) );
                                gdk_threads_enter();
                                hildon_banner_show_information((GtkWidget *)data->locserv->ui.window,NULL,info);
                                gdk_threads_leave();

                          }
                          break;
                    }
                    else{

                          buffer[addIndex++]=read[0];
                    }

		}


            processMode = 1;
		while(processMode)
		{
			switch(CurrentMode)
			{
				case MODE_JOY :
					printf("%c MODE_JOY Buffer Indexes : %d %d %c\n",MODE_TEXT,readIndex,addIndex,0x00);
					if ( addIndex-readIndex < 8)
					{
						processMode = 0;
						break; // wait for more data
					}

					// recopie du buffer pour le joy

					joy[JOY_AXIS_X_DI] = buffer[readIndex];
					joy[JOY_AXIS_Y_DI] = buffer[readIndex+1];
					joy[JOY_AXIS_R_DI] = buffer[readIndex+2];
					joy[JOY_AXIS_Z_DI] = buffer[readIndex+3];
					joy[JOY_BUTTON_EX_DI] = buffer[readIndex+4];
					joy[JOY_BUTTON_DI] = buffer[readIndex+5];

					control = joy[JOY_AXIS_X_DI] ^ joy[JOY_AXIS_Y_DI] ^ joy[JOY_AXIS_R_DI] ^ joy[JOY_AXIS_Z_DI] ^ joy[JOY_BUTTON_EX_DI] ^ joy[JOY_BUTTON_DI];

					if ( ! ( (unsigned char)buffer[readIndex+6] == 255 && (unsigned char)buffer[readIndex+7] == control ) )
					{

						printf("%ctransmission error control %d %c\n",MODE_TEXT,control,0x00);
						addIndex = 0;
					    readIndex = 0;
					    CurrentMode=0;
					    printf("%c Reset %c\n",MODE_TEXT,0x00);
                                  break;
                              }
                              readIndex+=7;

                              // process display;
                              MOTOR_D = Limit_Mix(2000 + joy[JOY_AXIS_X_DI] + joy[JOY_AXIS_Y_DI] - 127);       //right motor = right stick y-axis
					MOTOR_G = Limit_Mix(2000 + joy[JOY_AXIS_X_DI] - joy[JOY_AXIS_Y_DI] + 127);        //left motor = left stick y-axis

                              gdk_threads_enter();

                              GtkWidget* widget = data->locserv->ui.drawing_area;

                              gdk_draw_point(widget->window,
                                widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                                  widget->allocation.width/2,widget->allocation.height/2);

                              GdkGC * gc =  gdk_gc_new (widget->window);
                              gdk_gc_set_function (gc,GDK_AND);
                              gdk_draw_line (widget->window,gc,widget->allocation.width/2-255,widget->allocation.height/2,
                                            widget->allocation.width/2-255,widget->allocation.height/2 + MOTOR_G -127 );

                              gdk_draw_line (widget->window,gc,widget->allocation.width/2 + 255,widget->allocation.height/2,
                                            widget->allocation.width/2 + 255,widget->allocation.height/2 + MOTOR_D -127 );

                              gdk_threads_leave();

                              CurrentMode = 0;
					processMode = 0;
					break;
                         case 0 :
					if( addIndex >0)
						CurrentMode = buffer[readIndex++];
					else
						processMode = 0;
					break;
				default :
					printf("%c Mode Erro r%c \n",MODE_TEXT,0x00);
					if( addIndex >0)
						CurrentMode = buffer[readIndex++];
					break;
                  }
                  if( addIndex<=readIndex )
			{
				addIndex = 0;
				readIndex = 0;
				processMode = 0;
			}



		}
                    if( readIndex>0 )
                  {
                  //printf("%cProcess_Data_From_Local_IO Buffer Indexes : %d %d %c",MODE_TEXT,readIndex,addIndex,0x00);
                        while(readIndex>0)
                        {
                              for(int i=readIndex;i<addIndex;i++)
                                    buffer[i-1]=buffer[i];
                              readIndex--;
                              addIndex--;
                        }
                  //printf("%cProcess_Data_From_Local_IO Buffer Indexes : %d %d %c",MODE_TEXT,readIndex,addIndex,0x00);
                  }
		g_get_current_time (&wait);
		g_time_val_add (&wait,10000);

	}while(!g_cond_timed_wait ( data->locserv->disconnect,wait_mut, &wait ));

	g_mutex_unlock (wait_mut);

	g_mutex_free(wait_mut);

	g_cond_free(data->locserv->disconnect);
	data->locserv->disconnect = NULL;

	// Closing the server socket
	close(s);

	return;

}
