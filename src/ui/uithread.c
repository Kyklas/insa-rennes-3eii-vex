/*
*
*	File name : uithread.c
*	Author : Stanislas BERTRAND
*	Purpuse : Bluetooth thread methodes for Vex Bluetooth Remote Control
*
*/

#include "uithread.h"

gpointer thread_banner_progress(AppData* data)
{

	GTimeVal  wait;
	GMutex *wait_mut = g_mutex_new ();

	g_mutex_lock (wait_mut);
	do
	{
		g_get_current_time (&wait);
		g_time_val_add (&wait,200000);
		gtk_progress_bar_pulse (data->banner_progress_bar);
	}
	while(!g_cond_timed_wait ( data->cond_search_finish,wait_mut, &wait ));

	g_mutex_unlock (wait_mut);

	g_mutex_free(wait_mut);

	gtk_widget_destroy((GtkWidget*) data->banner_progress);

	g_cond_free(data->cond_search_finish);
	data->cond_search_finish = NULL;

	return NULL;
}
