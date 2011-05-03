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
#include <errno.h>
#include <stdlib.h>

#include "btthread.h"
#include "../ui/interface.h"

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
	int i;
	for(i = 0 ;i < data->number_devices;i++ )
	{
		char device[35];
		sprintf(device,"%s - %s",data->devices[i].name,data->devices[i].address);
		gtk_combo_box_append_text(data->cb_dev_list, device);
	}

	g_cond_signal (data->cond_search_finish);

	return NULL;
}

// TODO ckeck up on pango for utf8 handling
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


gboolean Ack_Pending = false;

void CMD_DPL_ACK_Handler(unsigned char cmd)
{
	if(Ack_Pending)
		Ack_Pending = false;
}

gboolean CMD_ENV_Handler(unsigned char cmd,AppData* data)
{
	char *send_Buffer;
	int recv_Status;
	int send_Status;
	char cmd_Buffer[CMD_MAX_LENGTH];
	unsigned char view_Angle;
	unsigned char view_Distance;

	// Getting the CMD data
	recv_Status = recv(data->devconn->bt.sock,
					cmd_Buffer,
					CMD_ENV_LEN,
					MSG_WAITALL);

	if(recv_Status !=  CMD_ENV_LEN)
	{
		// we have an problem, we didn't get all the data
		gdk_threads_enter();
		INFO_DISP("Thread Comm, Recv CMD_ENV",(GtkWidget *)data->devconn->ui.window);
		gdk_threads_leave();
		return false;
	}

	// Update the view array

	// Getting acces to the view array
	g_mutex_lock (data->devconn->bt.view_Array_Mut);

	// the first byte contains the angle
	view_Angle = (unsigned char) cmd_Buffer[1];
	// the second byte contains the distance
	view_Distance = (unsigned char) cmd_Buffer[0];

	view_Angle = view_Angle*100/256;

	puts("\nUpdate\n");
	printf("Angle : %d \n",view_Angle);
	printf("Updating view_Array with [ %d ] = %d\n",(unsigned int)(view_Angle/ENV_ANGLE_INC),view_Distance);

	if(view_Angle<=ENV_FOV)
		data->devconn->bt.view_Array[(unsigned int)(view_Angle/ENV_ANGLE_INC)]=view_Distance;

	// releasing acces to the view array
	g_mutex_unlock(data->devconn->bt.view_Array_Mut);

	if(data->devconn->ui.drawing_area != NULL )
	{
		gdk_threads_enter();
		gdk_window_invalidate_rect ((data->devconn->ui.drawing_area->window),NULL,TRUE);
		gdk_threads_leave();
	}

	// Send ACK
	send_Buffer = (char*) malloc(sizeof(char));

	send_Buffer[0] = CMD_ENV_ACK;

	send_Status = send(data->devconn->bt.sock,
				send_Buffer,
				1,
				0);

	free(send_Buffer);

	if(send_Status == -1 )
	{
		// we have an problem
		gdk_threads_enter();
		INFO_DISP("Thread Comm, Sending CMD_ENV_ACK",(GtkWidget *)data->devconn->ui.window);
		gdk_threads_leave();
		return false;
	}

	return true;
}





/**
 * \fn void thread_Vex_Communication_Func(AppData *data)
 * \brief Gère la communication avec le Vex selon le protocole établit
 *
 * \param[in,out] data Donné du programme
 *
 * Thread pour recevoir les données de la communication avec le Vex
 *
 */

void thread_Vex_Communication_Func(AppData *data)
{
	char cmd_Buffer[CMD_MAX_LENGTH];
	char *send_Buffer;
	int recv_Status;
	int send_Status;
	int i;
	fd_set fd_Read,fd_Write;
	gboolean running;
	struct timeval tv;

	FD_ZERO(&fd_Read);
	FD_ZERO(&fd_Write);

	FD_SET(data->devconn->bt.sock,&fd_Write);
	FD_SET(data->devconn->bt.sock,&fd_Read);

	running = true;

	tv.tv_sec = 0;
	tv.tv_usec = 100;

	/* Waiting to have display fonctiontial */

	g_mutex_lock (data->devconn->bt.thread_Communication_Mut);


	while(select(data->devconn->bt.sock+1,&fd_Read,NULL,NULL,&tv)!=-1 && running)
	{

		if(FD_ISSET(data->devconn->bt.sock,&fd_Read))
		{
			// Data to be retrieved
			recv_Status = recv(data->devconn->bt.sock,cmd_Buffer,1,MSG_WAITALL);
			if(recv_Status == -1 )
			{
				gdk_threads_enter();
				INFO_DISP("Thread Comm, Recv",(GtkWidget *)data->devconn->ui.window);
				gdk_threads_leave();
				running = false;
			}
			else
			{
				switch(cmd_Buffer[0]&0xF0)
				{

					case CMD_DPL_ACK :
					case CMD_ENV_ACK :

						CMD_DPL_ACK_Handler(cmd_Buffer[0]);
						break;

					case CMD_DPL :
						printf("CMD_DPL unhandle!\n");
						break;

					case CMD_ENV :
						running = CMD_ENV_Handler(cmd_Buffer[0],data);
						break;
					case CMD_ERROR :
						printf("CMD_ERROR unhandle!\n");
						break;
					default :

					printf("Unknown CMD Recived : %X\n",cmd_Buffer[0]);
				}
			}
		}

		if(FD_ISSET(data->devconn->bt.sock,&fd_Write) && !Ack_Pending)
		{
			// Able to write
			// Getting acces to the send queue
			g_mutex_lock (data->devconn->bt.send_Queue_Mut);

			if(g_slist_length(data->devconn->bt.send_Queue)>0 )
			{
				switch(*((char*)data->devconn->bt.send_Queue->data))
				{
					case CMD_DPL :
					case CMD_ENV :


						send_Status = send(data->devconn->bt.sock,
									data->devconn->bt.send_Queue->data,
									3,
									0);


						if(send_Status==-1)
						{
							// we have an problem, we didn't send the data correctly
							gdk_threads_enter();
							INFO_DISP("Thread Comm, Sending CMD",(GtkWidget *)data->devconn->ui.window);
							gdk_threads_leave();
							running = false;
						}
						else
						{


							Ack_Pending = TRUE;

							// free the data link to the element of the queue
							free(data->devconn->bt.send_Queue->data);
							// remove the top element of the queue
							data->devconn->bt.send_Queue=g_slist_delete_link (data->devconn->bt.send_Queue,
												data->devconn->bt.send_Queue);

						}
						break;

					default :

					printf("commande unknown : %d\n",*((char*)data->devconn->bt.send_Queue->data) );

					printf("Command & Data : %s\n",(char*)data->devconn->bt.send_Queue->data);
					free(data->devconn->bt.send_Queue->data);
					//remove
					data->devconn->bt.send_Queue=g_slist_delete_link (data->devconn->bt.send_Queue,
												data->devconn->bt.send_Queue);

				}
			}

			// releasing acces to the send queue
			g_mutex_unlock(data->devconn->bt.send_Queue_Mut);
		}
		tv.tv_sec = 0;
		tv.tv_usec = 100;
		FD_SET(data->devconn->bt.sock,&fd_Write);
		FD_SET(data->devconn->bt.sock,&fd_Read);
	}

	// Select returned -1, Error occured
	//printf("Select returned -1 : %d %s\n",errno,strerror(errno));
	g_mutex_unlock (data->devconn->bt.thread_Communication_Mut);


	close(data->devconn->bt.sock);

	printf("Thread Ends\n");

	return;
}

/**
 * \fn void thread_Communication_Func(AppData *data)
 * \brief Obtient les messages de la communication
 *
 * \param[in,out] data Donné du programme
 *
 * Thread pour recevoir les message de la communication
 *
 */

void thread_Communication_Func(AppData *data)
{
	char cmd_Buffer[CMD_MAX_LENGTH];
	char *send_Buffer;
	int recv_Status;
	int send_Status;
	gboolean ack_Pending=0;
	GTimeVal  wait;

	signed char view_Angle;
	unsigned char view_Distance;


	g_mutex_lock (data->devconn->bt.thread_Communication_Mut);
	do{

		// First we handle incoming data
		// Getting the CMD Code
		recv_Status = recv(data->devconn->bt.sock,cmd_Buffer,1,MSG_DONTWAIT);

		//printf("Recv : status : %d error : %d strerror %s\n",recv_Status,errno,strerror(errno));

		if(recv_Status == -1 && errno!=EAGAIN )
		{
			// we have an problem
			printf("Recv Error : %d %s\n",errno,strerror(errno));
			if( errno == ECONNRESET || errno == ECONNABORTED || errno == ENETRESET || errno == EBADF )
			{
				gdk_threads_enter();
				gtk_widget_destroy( (GtkWidget *) data->devconn->ui.window) ;
				gdk_threads_leave();
			}

		}
		else if(recv_Status == 1 )
		{
			switch(cmd_Buffer[0])
			{
				case CMD_DPL :
					// Getting the CMD data
					recv_Status = recv(data->devconn->bt.sock,
									cmd_Buffer,
									CMD_DPL_LEN,
									MSG_WAITALL);

					if(recv_Status != CMD_DPL_LEN )
					{
						// we have an problem, we didn't get all the data
						printf("Recv Problem while getting CMD_DPL : %d %s\n",errno,strerror(errno));
					}
					else
					{
						// I don' know what to do with a deplacment commande for the PDA !!
						// We keep going
					}
					break;
				case CMD_ENV :

					// Send ACK

					send_Buffer = (char*) malloc(sizeof(char));

					send_Buffer[0] = CMD_ENV_ACK;

					send_Status = send(data->devconn->bt.sock,
								send_Buffer,
								1,
								0);

					if(send_Status == -1 )
					{
						// we have an problem
						printf("Sending Problem while sending CMD_ENV_ACK : %d %s\n",errno,strerror(errno));
					}


					// Getting the CMD data
					recv_Status = recv(data->devconn->bt.sock,
									cmd_Buffer,
									CMD_DPL_LEN,
									MSG_WAITALL);

					if(recv_Status !=  CMD_DPL_LEN)
					{
						// we have an problem, we didn't get all the data
						printf("Recv Problem while getting CMD_DPL : %d %s\n",errno,strerror(errno));
					}
					else
					{
						// Update the view array

						// Getting acces to the view array
						g_mutex_lock (data->devconn->bt.view_Array_Mut);

						// the first byte contains the angle
						view_Angle = (signed char) cmd_Buffer[0];
						// the second byte contains the distance
						view_Distance = (unsigned char) cmd_Buffer[1];

						printf("Updating view_Array with [ %d ] = %d\n",(view_Angle+ENV_FOV_HALF),view_Distance);

						if(view_Angle>= -ENV_FOV_HALF && view_Angle<= ENV_FOV_HALF)
							data->devconn->bt.view_Array[view_Angle+ENV_FOV_HALF]=view_Distance;

						// releasing acces to the view array
						g_mutex_unlock(data->devconn->bt.view_Array_Mut);

						// TODO check if we invalidate the screen ?

					}
					break;
				case CMD_DPL_ACK :

					/*if(ack_Pending)
						ack_Pending=0;

					break;*/
				case CMD_ENV_ACK :

					if(ack_Pending)
						ack_Pending=0;

					printf("Ack recived for %d\n",cmd_Buffer[0]);

					// I don' know what to do with a environement command ACK to the PDA !!
					// We keep going
					break;

				default :

				printf("Unknown Commande Recived : %d\n",cmd_Buffer[0]);
			}

		}

		// Second we handle outgoing data

		// Getting acces to the send queue
		g_mutex_lock (data->devconn->bt.send_Queue_Mut);

		if(g_slist_length(data->devconn->bt.send_Queue)>0 )
		{
			switch(*((char*)data->devconn->bt.send_Queue->data))
			{
				case CMD_DPL_ACK :
				case CMD_ENV_ACK :

					send_Status = send(data->devconn->bt.sock,
								data->devconn->bt.send_Queue->data,
								1,
								0);

					free(data->devconn->bt.send_Queue->data);

					// remove the top element of the queue
					data->devconn->bt.send_Queue=g_slist_delete_link (data->devconn->bt.send_Queue,
											data->devconn->bt.send_Queue);

					printf("Ack Sent\n");

					break;

				case CMD_DPL :
				case CMD_ENV :

					if(ack_Pending)
						break;

					// TODO Better Handling
					send_Status = send(data->devconn->bt.sock,
									data->devconn->bt.send_Queue->data,
									3,
									0);
					printf("Command & Data : %d %d %d\n",(int)((char*)data->devconn->bt.send_Queue->data)[0],(int)((char*)data->devconn->bt.send_Queue->data)[1],(int)((char*)data->devconn->bt.send_Queue->data)[2] );
					ack_Pending = 1;
					printf("Ack Pending\n");
					free(data->devconn->bt.send_Queue->data);
					// remove the top element of the queue
					data->devconn->bt.send_Queue=g_slist_delete_link (data->devconn->bt.send_Queue,
											data->devconn->bt.send_Queue);
					break;

				default :

				printf("commande unknown : %d\n",*((char*)data->devconn->bt.send_Queue->data) );

				printf("Command & Data : %s\n",(char*)data->devconn->bt.send_Queue->data);
				free(data->devconn->bt.send_Queue->data);
				//remove
				data->devconn->bt.send_Queue=g_slist_delete_link (data->devconn->bt.send_Queue,
											data->devconn->bt.send_Queue);

			}
		}

		// releasing acces to the send queue
		g_mutex_unlock(data->devconn->bt.send_Queue_Mut);


		g_get_current_time (&wait);
		g_time_val_add (&wait,100);

	}while(!g_cond_timed_wait ( data->devconn->disconnect,
					data->devconn->bt.thread_Communication_Mut,
					&wait ));

	g_mutex_unlock (data->devconn->bt.thread_Communication_Mut);

	g_cond_free(data->devconn->disconnect);
	data->devconn->disconnect = NULL;

	return;
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
	int s;
	//int bytes_read;
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
	/*int status;
	gchar * out;
	GtkTextIter  iter;*/

	ba2str( &rem_addr.rc_bdaddr, buffer );
	fprintf(stderr, "accepted connection from %s\n", buffer);
	printf("accepted connection from %s\n", buffer);
	memset(buffer, 0, sizeof(buffer));


	//GTimeVal  wait;
	GMutex *wait_mut = g_mutex_new ();

	data->locserv->disconnect = g_cond_new ();

	g_mutex_lock (wait_mut);
/*
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
*/
	g_mutex_unlock (wait_mut);

	g_mutex_free(wait_mut);

	g_cond_free(data->locserv->disconnect);
	data->locserv->disconnect = NULL;

	// Closing the server socket
	close(s);

	return;

}
