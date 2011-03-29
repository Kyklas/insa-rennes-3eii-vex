/**
 * \file bluetooth.c
 * \brief Fichier contenant les fonctions relatives au Bluetooth
 * \author Stanislas BERTRAND
 * \version 0.1
 * \date 14 mars 2011
 */

#include "btthread.h"
#include "bluetooth.h"



/**
 * \brief Nom des services Bluetooth
 *
 */

char uuid_strings[BT_NUMBER_UUID_VALUES][40] ={
				"SDP",
				"RFCOMM",
				"OBEX",
				"HTTP",
				"L2CAP",
				"BNEP",
				"Serial Port",
				"ServiceDiscoveryServerServiceClassID",
				"BrowseGroupDescriptorServiceClassID",
				"PublicBrowseGroup",
				"OBEX Object Push Profile",
				"OBEX File Transfer Profile",
				"Personal Area Networking User",
				"Network Access Point",
				"Group Network",
				};

/**
 * \brief Valeur des services Bluetooth
 *
 */

int uuid_values[BT_NUMBER_UUID_VALUES] = {
				0x0001,
				0x0003,
				0x0008,
				0x000C,
				0x0100,
				0x000F,
				0x1101,
				0x1000,
				0x1001,
				0x1002,
				0x1105,
				0x1106,
				0x1115,
				0x1116,
				0x1117
				};

/**
 * \fn void get_info(AppData * data)
 * \brief Obtient les périphériques et leurs informations
 *
 * \param[in,out] data Donné du programme
 *
 * Recherche les périphérique puis les informations qui leur sont relatives
 *
 */

void get_info(AppData * data){

	printf("Get_Info bluetooth.c\n");
	inquiry_info *ii = NULL;
	int number_devices_max,number_devices_found;
	int local_device_id, sock, inquiry_lenght;

	// SDP
	uuid_t uuid_searched;
	sdp_list_t *list_services = NULL, *list_search_services, *list_attrid;
	sdp_session_t *session = 0;



	local_device_id = hci_get_route(NULL);
	sock = hci_open_dev( local_device_id );
	if (local_device_id < 0 || sock < 0)
	{
		perror("opening socket");
		char info[50];
		sprintf(info,"Bluetooth Error : %d %s",errno,strerror(errno) );
		hildon_banner_show_information((GtkWidget *)data->window_main,NULL,info);
		goto ERROR;
	}

	inquiry_lenght = 8;
	number_devices_max = 255;

	ii = new inquiry_info[number_devices_max];

	number_devices_found = hci_inquiry(	local_device_id,
					inquiry_lenght,
					number_devices_max,
					NULL,
					&ii,
					IREQ_CACHE_FLUSH);
	if( number_devices_found < 0 )
	{
		perror("hci_inquiry");
		char info[50];
		sprintf(info,"Inquiry Error : %d %s",errno,strerror(errno) );
		hildon_banner_show_information((GtkWidget *)data->window_main,NULL,info);
		goto ERROR;
	}

	// Allocate the Devices Array
	data->number_devices = number_devices_found;
	data->devices = (BluetoothDevice*) malloc(sizeof(BluetoothDevice)*number_devices_found);

	for (int i = 0; i < number_devices_found; i++) {

		ba2str(&(ii+i)->bdaddr, data->devices[i].address);

		printf("Address %s\n",data->devices[i].address);

		// connect to the SDP server running on the remote machine
		session = sdp_connect( BDADDR_ANY, &(ii+i)->bdaddr, SDP_RETRY_IF_BUSY );

            if(session==NULL)
            {
                perror("sdp_connect");
                char info[50];
                sprintf(info,"sdp_connect Error : %d %s",errno,strerror(errno) );
                hildon_banner_show_information((GtkWidget *)data->window_main,NULL,info);
                continue;
            }

		// Creating the UUID for the search
		sdp_uuid16_create( &uuid_searched, data->setting_uuid );
		list_search_services = sdp_list_append( NULL, &uuid_searched );

		// specify that we want a list of all the matching applications' attributes
		uint32_t range = 0x0000ffff;
		list_attrid = sdp_list_append( NULL, &range );

		if(sdp_service_search_attr_req( session, list_search_services,
									SDP_ATTR_REQ_RANGE,
									list_attrid,
									&list_services) == -1 )
		// TODO : handle logs
		printf("SDP ended by a timeout\n");

		// Counting how many services there is
		int number_service_found;
		sdp_list_t *list = list_services;
		for (number_service_found = 0; list; list = list->next,number_service_found++ );

		//allocation services array
		data->devices[i].number_services = number_service_found;
		data->devices[i].services =(BluetoothService*) malloc(sizeof(BluetoothService)*number_service_found);

		// Going thru the services retrived
		for (int service_index=0; list_services; list_services = list_services->next,service_index++ ) {

			sdp_record_t *service_record = (sdp_record_t*) list_services->data;

			// Getting and storing the service name
			sdp_get_service_name(service_record, data->devices[i].services[service_index].name, 20);

			printf("\t Service Name %s\n",data->devices[i].services[service_index].name);
			sdp_list_t * service_protocols=NULL;

			if( sdp_get_access_protos(service_record,&service_protocols) == -1 )
			{
				goto ERROR;
			}

			data->devices[i].services[service_index].channel = sdp_get_proto_port(service_protocols,RFCOMM_UUID);

			printf("\t\t RFCOM Porte %d\n",data->devices[i].services[service_index].channel);
		}

		sdp_close(session);

		memset(data->devices[i].name, 0, sizeof(data->devices[i].name));
		if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(data->devices[i].name),data->devices[i].name, 0) < 0)
		{
			strcpy(data->devices[i].name, "[unknown]");
		}
		printf("Addres %s Name %s\n",data->devices[i].address,data->devices[i].name);
	}

ERROR:

	free(ii);
	close( sock );

	return;
}

/*
void get_info_BU(AppData * data){
	cout<<"Get INFO"<<endl;

	inquiry_info *ii = NULL;
	int max_rsp, result;
	int dev_id, sock, len, flags;
	int i;
	char addr[19] = { 0 };
	char name[248] = { 0 };

	// SDP
	uuid_t svc_uuid;
	sdp_list_t *response_list = NULL, *search_list, *attrid_list;
	sdp_session_t *session = 0;
	uint16_t uuid_looking = data->setting_uuid;


	dev_id = hci_get_route(NULL);
	sock = hci_open_dev( dev_id );
	if (dev_id < 0 || sock < 0)
	{
		perror("opening socket");
		return;
	}

	len  = 8;
	max_rsp = 255;
	flags = IREQ_CACHE_FLUSH;
	ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

	result = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
	if( result < 0 )
		perror("hci_inquiry");

	int dev_found = result;
	for (i = 0; i < dev_found; i++) {
		ba2str(&(ii+i)->bdaddr, addr);
		cout<<"Address "<<addr<<endl;
		// connect to the SDP server running on the remote machine
		session = sdp_connect( BDADDR_ANY, &(ii+i)->bdaddr, SDP_RETRY_IF_BUSY );

		sdp_uuid16_create( &svc_uuid, uuid_looking );
		search_list = sdp_list_append( NULL, &svc_uuid );

		// specify that we want a list of all the matching applications' attributes
		uint32_t range = 0x0000ffff;
		attrid_list = sdp_list_append( NULL, &range );

		if( sdp_service_search_attr_req( session, search_list,
									SDP_ATTR_REQ_RANGE,
									attrid_list,
									&response_list) == -1 )
		cout<<" SDP eended by a timeout"<<endl;
		for (; response_list; response_list = response_list->next ) {
			sdp_record_t *rec = (sdp_record_t*) response_list->data;
			char serv_name[100];
			sdp_get_service_name(rec,  serv_name, 100);
			cout<<"\tService Name "<<serv_name<<endl;
			sdp_get_clnt_exec_url(rec,  serv_name, 100);
			cout<<"\tExec Url "<<serv_name<<endl;
			sdp_list_t * proto=NULL;

			if( sdp_get_access_protos(rec,&proto) == 0 )
			{
				cout<<"\t\tsdp_get_access_protos == 0"<<endl;
			}

			for(;proto;proto=proto->next)
			{

				sdp_list_t *pds = (sdp_list_t*)proto->data;

				 // go through each protocol list of the protocol sequence
				for( ; pds ; pds = pds->next ) {

					// check the protocol attributes
					sdp_data_t *d = (sdp_data_t*)pds->data;
					int protonum=0;

					for( ; d; d = d->next ) {
						switch( d->dtd ) {
							case SDP_UUID16:
							case SDP_UUID32:
							case SDP_UUID128:
								char protoname[20];
								sdp_proto_uuid2strn(&d->val.uuid,protoname, 20);
								cout<<"\t\tProto name "<<protoname<<endl;
								protonum = sdp_uuid_to_proto( &d->val.uuid );
								cout<<"\t\tProto num "<<protonum<<endl;
								break;
						}
					}
				}
				cout<<"\t\tRFCOM Port "<<sdp_get_proto_port(proto,RFCOMM_UUID)<<endl;
				cout<<"\t\tL2CAP Port "<<sdp_get_proto_port(proto,L2CAP_UUID)<<endl;
			}
		}


		sdp_close(session);
		memset(name, 0, sizeof(name));
		if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name),name, 0) < 0)
		{
			strcpy(name, "[unknown]");
		}
		cout<<"Address "<<addr<<" Name "<<name<<endl;
	}
	free( ii );
	close( sock );
	return;
}
*/

/**
 * \fn bool create_comm_connection(AppData* data)
 * \brief Créer la connection à un périphérique pour la communication
 *
 * \param[in,out] data Donnée du programme
 * \return Retourne l'état de la connection, vrai si établi, faux sinon
 */

bool create_comm_connection(AppData* data)
{
	struct sockaddr_rc addr = { 0 };
	int status;
	data->devconn->bt.thread_Communication = NULL;
	// allocate a socket
	data->devconn->bt.sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	// set the connection parameters (who to connect to)
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) data->devconn->bt.service->channel;

	str2ba( data->devconn->bt.device->address , &addr.rc_bdaddr );



	// connect to server
	status = connect(data->devconn->bt.sock, (struct sockaddr *)&addr, sizeof(addr));

	if( status == -1 )
	{
		// Erreur

		printf("Erreur %d %s",errno,strerror(errno));
		char info[50];
		sprintf(info,"Recv Error : %d %s",errno,strerror(errno) );
		hildon_banner_show_information((GtkWidget *)data->window_main,NULL,info);
		close(data->devconn->bt.sock);
		return false;
	}

	data->devconn->disconnect = g_cond_new ();

	data->devconn->bt.view_Array_Mut = g_mutex_new ();
	data->devconn->bt.send_Queue_Mut = g_mutex_new ();
	data->devconn->bt.thread_Communication_Mut = g_mutex_new ();
	data->devconn->bt.send_Queue = NULL;


	data->devconn->bt.thread_Communication = g_thread_create((GThreadFunc) thread_Communication_Func,data,true,NULL);

	return true;

}

/**
 * \fn bool create_loc_serv_record(AppData * data)
 * \brief Créer le service local
 *
 * \param[in,out] data Donnée du programme
 * \return Retourne l'état du service, vrai si établi, faux sinon
 */

bool create_loc_serv_record(AppData * data)
{


	uint16_t uuid = 0x1101;
	uint8_t rfcomm_channel = LOCAL_SERVER_CHANNEL;
	const char *service_name = "Vex SP";
	const char *service_dsc = "Vex Remote Serial Port";
	const char *service_prov = "stanislasbertrand@gmail.com";

	uuid_t root_uuid, l2cap_uuid, rfcomm_uuid, svc_uuid;
	sdp_list_t *l2cap_list = 0,
		   *rfcomm_list = 0,
		   *root_list = 0,
		   *proto_list = 0,
		   *access_proto_list = 0;
	sdp_data_t *channel = 0;//*psm = 0;

	sdp_record_t *record = sdp_record_alloc();

	// set the general service ID
	printf("set the general service ID\n");
	sdp_uuid16_create( &svc_uuid, uuid );
	sdp_set_service_id( record, svc_uuid );

	// make the service record publicly browsable
	printf("make the service record publicly browsable\n");
	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root_list = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups( record, root_list );

	// set l2cap information
	printf(" set l2cap information\n");
	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	l2cap_list = sdp_list_append( 0, &l2cap_uuid );
	proto_list = sdp_list_append( 0, l2cap_list );

	// set rfcomm information
	printf(" set rfcomm information\n");
	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	channel = sdp_data_alloc(SDP_UINT8, &rfcomm_channel);
	rfcomm_list = sdp_list_append( 0, &rfcomm_uuid );
	sdp_list_append( rfcomm_list, channel );
	sdp_list_append( proto_list, rfcomm_list );

	// attach protocol information to service record
	printf("attach protocol information to service record\n");
	access_proto_list = sdp_list_append( 0, proto_list );
	sdp_set_access_protos( record, access_proto_list );

	// set the name, provider, and description
	sdp_set_info_attr(record, service_name, service_prov, service_dsc);


	//int err = 0;
	sdp_session_t *session = 0;

	// connect to the local SDP server, register the service record, and
	// disconnect
	session = sdp_connect(BDADDR_ANY, BDADDR_LOCAL , 0 );
	if(session != NULL )
	{
		if(sdp_record_register(session, record, 0) == -1 )
		{
			printf("sdp_record_register Error : %d %s\n",errno,strerror(errno));
			goto ERROR;
		}
	}
	else
	{
		printf("sdp_connect Error : %d %s\n",errno,strerror(errno));
		goto ERROR;
	}
	//sdp_record_unregisters
	data->locserv_record = record ;
	// cleanup

	sdp_data_free( channel );
	sdp_list_free( l2cap_list, 0 );
	sdp_list_free( rfcomm_list, 0 );
	sdp_list_free( root_list, 0 );
	sdp_list_free( access_proto_list, 0 );
	sdp_close(session);

	return true;

ERROR :
	data->locserv_record = NULL;

	sdp_data_free( channel );
	sdp_list_free( l2cap_list, 0 );
	sdp_list_free( rfcomm_list, 0 );
	sdp_list_free( root_list, 0 );
	sdp_list_free( access_proto_list, 0 );

	return false;

}

/**
 * \fn void unregister_loc_serv_record(AppData* data)
 * \brief Supprime le service local
 *
 * \param[in,out] data Donnée du programme
 */

void unregister_loc_serv_record(AppData* data)
{
	if( data->locserv_record == NULL)
		return;

	//int err = 0;
	sdp_session_t *session = 0;
	session = sdp_connect(BDADDR_ANY, BDADDR_LOCAL , 0 );
	if(session != NULL )
	{
		if(sdp_record_unregister(session, data->locserv_record) == -1 )
		{
			printf("sdp_record_unregister Erreur %d %s",errno,strerror(errno));
		}
	}
	else
	{
	// sdp_connect Erreur
		printf("sdp_connect Erreur %d %s",errno,strerror(errno));
	}
	sdp_close(session);
}
