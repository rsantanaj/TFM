/*
 * datalogger.c
 *
 *  Created on: 3/11/2019
 *      Author: José Raúl Santana Jiménez
 *
 *  License Creative Commons 3.0 (by-nc-nd)
 */
#include "wiced.h"
#include "datalogger_MetOcean_TCP_protocol.h"
#include "datalogger_MetOcean_Commands.h"
#include "datalogger_Acquire.h"
#include "datalogger_general.h"
#include "datalogger_Sensors.h"
#include "datalogger_Serial_Port.h"
#include "datalogger_memory.h"
/******************************************************
 *                      Macros
 ******************************************************/

#define TCP_SERVER_LISTEN_PORT              (50007)
#define TCP_PACKET_MAX_DATA_LENGTH          (30)


#define THREAD_PRIORITY_ACQUISITION_MODE     (10)
#define THREAD_STACK_SIZE_ACQUISITION_MODE   (1024)



/******************************************************
 *                   Enumerations
 ******************************************************/
typedef enum
{
    SBE37_SM,
    OPTODE_3835,
    WETLABS_FLNTU
}datalogger_instruments_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static wiced_result_t client_connected_callback   ( wiced_tcp_socket_t* socket, void* arg );
static wiced_result_t client_disconnected_callback( wiced_tcp_socket_t* socket, void* arg );
static wiced_result_t received_data_callback      ( wiced_tcp_socket_t* socket, void* arg );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static wiced_tcp_socket_t tcp_server_socket;
static datalogger_protocol_command_t struct_command;
datalogger_instrument_t datalogger_available_instruments[MAX_NUMBER_OF_INSTRUMENTS];

wiced_utc_time_t datalogger_datetime;
wiced_iso8601_time_t datalogger_datetime_iso;

static wiced_thread_t ThreadHandle_Acquisition_Mode;

static wiced_semaphore_t semaphoreHandle_Acquisition_Mode;

static const wiced_ip_setting_t device_init_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS(192, 168, 10,  1) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS(255, 255, 255, 0) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS(192, 168, 10,  1) ),
};

/******************************************************
 *               Function Definitions
 ******************************************************/
void datalogger_enable_tcp_server(void) {
    wiced_result_t result;

    /* Bring up the network interface */
    wiced_network_up( WICED_AP_INTERFACE, WICED_USE_INTERNAL_DHCP_SERVER, &device_init_ip_settings );

    /* Create a TCP server socket */
    if ( wiced_tcp_create_socket( &tcp_server_socket, WICED_AP_INTERFACE ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("TCP socket creation failed\r\n") );
    }

    /* Register callbacks to handle various TCP events */
    result = wiced_tcp_register_callbacks( &tcp_server_socket, client_connected_callback, received_data_callback, client_disconnected_callback, NULL );

    if ( result != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("TCP server socket initialization failed\r\n") );
    }

    /* Start TCP server to listen for connections */
    if ( wiced_tcp_listen( &tcp_server_socket, TCP_SERVER_LISTEN_PORT ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("TCP server socket initialization failed\r\n") );
        wiced_tcp_delete_socket( &tcp_server_socket );
        return;
    }

    WPRINT_APP_INFO( ("Async tcp server started. Listening on port %d \r\n", TCP_SERVER_LISTEN_PORT) );

}


/* Define the thread function that will set the aquisition mode of the datalogger */
void acquisition_mode_thread(wiced_thread_arg_t arg){
    datalogger_mode_satates_t datalogger_prev_mode;
    wiced_rtos_delay_milliseconds(500);
    datalogger_prev_mode = datalogger_actual_mode;
    while(1){
        if (datalogger_actual_mode == CONFIGURING){
            //Here stops the acquisition threads
            //instruments_acquiring_stop = 1;
            set_stop_instrument_acquiring(1);
            datalogger_prev_mode = CONFIGURING;
            wiced_rtos_get_semaphore(&semaphoreHandle_Acquisition_Mode, WICED_WAIT_FOREVER);
            wiced_rtos_delay_milliseconds(1000);
        }
        if ((datalogger_prev_mode == CONFIGURING)&& (datalogger_actual_mode == ACQUIRING)) {
            //instruments_acquiring_stop=0;
            set_stop_instrument_acquiring(0);
            acquisition_mode_start(&datalogger_available_instruments);
        }
        datalogger_prev_mode = datalogger_actual_mode;
        wiced_rtos_delay_milliseconds(100);
    }
}


void datalogger_init(void){
    //Initialize the datalogger_available_instruments array
    for (int i = 0; i<MAX_NUMBER_OF_INSTRUMENTS; i++){
        datalogger_available_instruments[i].enabled = 0;
        datalogger_available_instruments[i].instrument_type = NONE_It;
    }

    //Set up the datalogger mode semaphore handle
    wiced_rtos_init_semaphore(&semaphoreHandle_Acquisition_Mode);
    /* Initialize and start Acquisition MODE thread */
    wiced_rtos_create_thread(&ThreadHandle_Acquisition_Mode, THREAD_PRIORITY_ACQUISITION_MODE, "Thread_Acquisition_Mode", acquisition_mode_thread, THREAD_STACK_SIZE_ACQUISITION_MODE, NULL);
    /* Initialize the datalogger serial ports */
    datalogger_serial_ports_init();
    /* Set up the instruments semaphore handlers and Initialize and start instruments threads */
    acquisition_mode_init();
    datalogger_get_memory_size();
    if (!datalogger_memory_ready()){
        datalogger_memory_init();
    }
    wiced_rtos_delay_milliseconds(5000);
    wiced_time_get_utc_time(&datalogger_datetime);
    wiced_time_get_iso8601_time(&datalogger_datetime_iso);
}

void application_start( void )
{
    datalogger_mode_satates_t datalogger_prev_mode;
    /* Initialise the device and WICED framework */
    wiced_init( );
    datalogger_init();
    datalogger_enable_tcp_server();
    datalogger_prev_mode = datalogger_actual_mode;
    while (1) {
        if((datalogger_actual_mode == ACQUIRING)&&(datalogger_prev_mode == CONFIGURING)){
            // Set the Acquiring state semaphore
             wiced_rtos_delay_milliseconds(datalogger_programmed_acquire_mode_delay);
            wiced_rtos_set_semaphore(&semaphoreHandle_Acquisition_Mode);
        }
        datalogger_prev_mode = datalogger_actual_mode;
        wiced_rtos_delay_milliseconds(200);
    }
}

static wiced_result_t client_connected_callback( wiced_tcp_socket_t* socket, void* arg )
{
    wiced_result_t      result;
    wiced_ip_address_t  ipaddr;
    uint16_t            port;

    UNUSED_PARAMETER( arg );

    /* Accept connection request */
    result = wiced_tcp_accept( socket );
    if( result == WICED_SUCCESS )
    {
        /* Extract IP address and the Port of the connected client */
        wiced_tcp_server_peer( socket, &ipaddr, &port );

        WPRINT_APP_INFO(("Accepted connection from :: "));

        WPRINT_APP_INFO ( ("IP %u.%u.%u.%u : %d\r\n", (unsigned char) ( ( GET_IPV4_ADDRESS(ipaddr) >> 24 ) & 0xff ),
                                                      (unsigned char) ( ( GET_IPV4_ADDRESS(ipaddr) >> 16 ) & 0xff ),
                                                      (unsigned char) ( ( GET_IPV4_ADDRESS(ipaddr) >>  8 ) & 0xff ),
                                                      (unsigned char) ( ( GET_IPV4_ADDRESS(ipaddr) >>  0 ) & 0xff ),
                                                      port ) );

        return WICED_SUCCESS;
    }
    return WICED_ERROR;
}

static wiced_result_t client_disconnected_callback( wiced_tcp_socket_t* socket, void* arg )
{
    UNUSED_PARAMETER( arg );

    WPRINT_APP_INFO(("Client disconnected\r\n\r\n"));

    wiced_tcp_disconnect(socket);
    /* Start listening on the socket again */
    if ( wiced_tcp_listen( socket, TCP_SERVER_LISTEN_PORT ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("TCP server socket re-initialization failed\r\n") );
        wiced_tcp_delete_socket( socket );
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

static wiced_result_t received_data_callback( wiced_tcp_socket_t* socket, void* arg )
{
    wiced_result_t      result;
    wiced_packet_t*     tx_packet;
    char*               tx_data;
    wiced_packet_t*     rx_packet = NULL;
    char*               request;
    uint16_t            request_length;
    uint16_t            available_data_length;
    char*               command_received;

    result = wiced_tcp_receive( socket, &rx_packet, WICED_WAIT_FOREVER );
    if ( result != WICED_SUCCESS )
    {
        return result;
    }

    wiced_packet_get_data( rx_packet, 0, (uint8_t**) &request, &request_length, &available_data_length );

    if (request_length != available_data_length)
    {
        WPRINT_APP_INFO(("Fragmented packets not supported\r\n"));
        return WICED_ERROR;
    }
    printf("AVAILABLE DATA LENGTH: %d\r\n", available_data_length);

    /* Null terminate the received string */
    request[request_length] = '\x0';
    WPRINT_APP_INFO(("Received data: %s \r\n", request));

    /* Send echo back */
    if ( wiced_packet_create_tcp( socket, TCP_PACKET_MAX_DATA_LENGTH, &tx_packet, (uint8_t**)&tx_data, &available_data_length ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO(("TCP packet creation failed\r\n"));
        return WICED_ERROR;
    }

    // Parse the received MetOcean command
    struct_command = datalogger_metocean_parser(request);
    // Execute the received MetOcean Command
    metocean_command_execute(&struct_command, datalogger_available_instruments);
    request_length = command_out_length;
    tx_data[request_length] = '\x0';
    memcpy( tx_data, &command_out[0], request_length );
    /* Set the end of the data portion */
    wiced_packet_set_data_end( tx_packet, (uint8_t*)tx_data + request_length );

    /* Send the TCP packet */
    if ( wiced_tcp_send_packet( socket, tx_packet ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("TCP packet send failed\r\n") );

        /* Delete packet, since the send failed */
        wiced_packet_delete( tx_packet );
    }
    WPRINT_APP_INFO(("Echo data: %s\r\n", tx_data));

    /* Release a packet */
    wiced_packet_delete( rx_packet );
    return WICED_SUCCESS;
}
