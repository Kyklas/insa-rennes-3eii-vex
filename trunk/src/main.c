/**
 * \file main.c
 * \brief Fichier principale du programme Vex
 * \author Stanislas BERTRAND
 * \version 0.1
 * \date 14 mars 2011
 */


#include <stdio.h>

#include <glib.h>

#include <hildon/hildon-program.h>
#include <hildon/hildon-window.h>
#include <hildon/hildon-banner.h>

#include <gtk/gtkmain.h>


#include "appdata.h"
#include "ui/interface.h"

/**
 * \fn int main(int argc, char *argv[])
 * \brief Fonction main du programme Vex
 *
 * \param[in] argc Nombre d'argument passé a main
 * \param[in] argv Liste des argument
 * \return 0
 *
 * Fonction main pour le programme Vex.
 * Initialisation des variables du programme
 * Inistalisation de l'environement graphique
 *
 */


int main(int argc, char *argv[])
{
	/* Create needed variables */
	HildonProgram *program;

	/* Initilize thread support */
	if (!g_thread_supported())
	{
		g_thread_init(NULL);
		gdk_threads_init();
	}

	/* Initialize the GTK. */
	gtk_init(&argc, &argv);

	/* Create the Hildon program and setup the title */
	program = HILDON_PROGRAM(hildon_program_get_instance());
	g_set_application_name("Vex");


	/* Initilize thread support */
	if (!g_thread_supported())
		g_thread_init(NULL);


	AppData * data = create_data();
	data->program = program;
	create_user_interface(data);

	/* Connect signal to X in the upper corner */
	g_signal_connect(G_OBJECT(data->window_main), "delete_event",
			G_CALLBACK(gtk_main_quit), NULL);


	/* Show winodw */
	gtk_widget_show(GTK_WIDGET(data->window_main));
	/* Begin the main application */
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	/* Clean Up */
	// TODO : Clean Up

	/* Exit */
	return 0;

}
