/**
 * \file appdata.h
 * \brief Fichier de la structure des données de l'application
 * \author Stanislas BERTRAND
 * \version 0.1
 * \date 14 mars 2011
 */

#ifndef VEX_APPDATA_H
#define VEX_APPDATA_H

#include "communication.h"


/**
 * \def FAST_CONNECT
 * \brief Quand définit, la connection rapide est compilé
 *
 */
//#define FAST_CONNECT

/**
 * \def FAST_CONNECT_ADD
 * \brief Définition de l'adresse pour une connection rapide
 *
 */
//#define FAST_CONNECT_ADD "00:0B:CE:02:3C:D5" // Vex
//#define FAST_CONNECT_CHAN 1
#define FAST_CONNECT_ADD "00:1D:6E:D4:C8:71" // N810
#define FAST_CONNECT_CHAN 3
//	#define FAST_CONNECT_ADD "00:11:67:B0:4B:6D"



#include <hildon/hildon-program.h>
#include <hildon/hildon-window.h>
#include <hildon/hildon-banner.h>

#include <libintl.h>
#include <locale.h>
#include <libosso.h>

#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>


/**
 * \struct _AppData appdata.h
 * \brief Structure contenant les données de l'application
 */
typedef struct _AppData AppData;
/**
 * \struct _BluetoothService appdata.h
 * \brief Structure contenant un service Bluetooth
 */
typedef struct _BluetoothService BluetoothService;
/**
 * \struct _BluetoothDevice appdata.h
 * \brief Structure contenant les informations sur un périphérique
 */
typedef struct _BluetoothDevice BluetoothDevice;
/**
 * \struct _DeviceConnection appdata.h
 * \brief Structure contenant les informations de connection a un périphérique
 */
typedef struct _DeviceConnection DeviceConnection;

struct _BluetoothService
{
	int channel;
	char name[20];
};

struct _BluetoothDevice
{
	char name[20];
	char address[18];
	int number_services;
	BluetoothService *services;
};

struct _DeviceConnection
{
	GCond* disconnect;

	/**
	 * \struct _ui
	 * \brief Structure contenant les informations relative à l'interface pour la connection au Périphérique
	 */
	struct _ui
	{
		GtkTextBuffer *text_buffer;
		GtkTextView *text_view;
		GtkVBox *vbox;
		GtkEntry *entry;
		GtkButton *button;
		HildonWindow *window;
            GtkWidget *drawing_area;
		bool fullscreen;				/**< Indicateur d'utilisation du mode plein écran*/
	}ui;


	/**
	 * \struct _bt
	 * \brief Structure contenant les informations relative au Bluetooth pour la connection au Périphérique
	 */
	struct _bt
	{
		BluetoothDevice *device;		/**< Périphérique connecté*/
		BluetoothService *service;		/**< Service du périphérique*/
		int sock;					/**< Socket utilisé pour la communication*/
		GThread *thread_Communication;	/**< Identificateur pour le thread de communication*/
		GMutex *view_Array_Mut;
		GMutex *send_Queue_Mut;
		GMutex *thread_Communication_Mut;
		char view_Array[ENV_FOV/ENV_ANGLE_INC];
		GSList *send_Queue;
	}bt;
};

struct _AppData {

	/* View items */


	HildonProgram *program;				/**< Référence du programme ( Librairie Hildon ) */
	HildonWindow *window_main;			/**< Fenêtre principale du programme ( Librairie Hildon ) */
	HildonWindow *window_conn;			/**< Fenêtre de connection a un périphérique ( Librairie Hildon ) */
	osso_context_t* osso;				/**< ( Librairie osso ) */

	HildonBanner *banner_progress;		/**< Bannières d'information avec une barre d'avancement ( Librairie Hildon ) */
	GtkProgressBar *banner_progress_bar;	/**< La barre d'avancement de la bannière ( Librairie Gtk ) */

	GCond* cond_search_finish;			/**< Condition : La recherche bluetooth a fini */

	GtkComboBox *cb_serv_list;			/**< Liste des services du périphérique selectionné*/
	GtkComboBox *cb_dev_list;			/**< liste des périphériques découvert*/

	GtkToolbar *tb_services;			/**< Barre de service*/
	GtkToolbar *tb_options;				/**< Barre d'options*/

	int setting_uuid;					/**< UUID du service Bluetooth recherché*/

	int number_devices;				/**< Nombre de périphérique dévouvert*/
	BluetoothDevice *devices;			/**< Tableau contenant les périphériques découvert*/

	DeviceConnection *devconn;			/**< Liste des périphérique connecté*/
	DeviceConnection *locserv;			/**< Liste des services Locaux ???*/
	sdp_record_t *locserv_record;			/**< Liste des informations des services locaux ??? */

};

#endif // VEX_APPDATA_H
