
# Created on: 23/11/2019
#     Author: José Raúl Santana Jiménez
# This file includes the code for the creation and behaviour of the GUI
# License Creative Commons 3.0 (by-nc-nd)


#!/usr/bin/env python
import socket
import optparse
import datetime
import time
import sys
import threading
import PIL.Image
import PIL.ImageTk
import tkFont
import MetOceanParser
import datalogger_acquire

from tkinter import * 
from tkinter import ttk
import tkMessageBox, tkFileDialog
from MetOceanParser import *

MetOceanCommand = MetOcean_object_command(-1,-1,{})

#datalogger_date_time= datetime.datetime.now()

# Max number of instruments that can be configured in the GUI
NUMBER_OF_INSTRUMENTS = 8;
# Instruments that are available per row 
NUMBER_OF_INSTRUMENTS_PER_ROW = 4

BUFFER_SIZE = 1024
COMMAND = "0-0(0,2;1,1)"
# IP details for the WICED TCP server
DEFAULT_IP   = '192.168.10.1'    # IP address of the WICED TCP async server
DEFAULT_PORT = 50007             # Port of the WICED TCP server


# Labels for error handling while connecting to the datalogger##
SUCCESS = 0
NETWORK_MISTMATCH = 1
UNABLE_TO_CONNECT_TO_SERVER = 2
UNABLE_TO_GET_HOST_IP = 3
################################################################
# Labels for MetOcean principal commands definition
#SET = '0'
#GET = '1'
## Labels for MetOcean secondary commands definition
#INSTRUMENT = '0'
#MODE = '1'
#
#
##Labels for MetOcean commands arguments definition
#INSTRUMENT_ID = '0'
#TYPE_INSTRUMENT = '1'
#
## Labels for TYPE_INSTRUMENT argument possible values
#NONE = '0'
#SBE_37SM = '1'


DEFAULT_KEEP_ALIVE = 0           # Keep the connection alive (=1), or close the connection (=0)

received_MetOcean_data = "-1" # Global variable for keeping the las received MetOcean data from the datalogger
realtime_file_path = ""

# Arrays for managing the GUIs widgets##########################
# list containing all the "Enabled" Checkbuttos of the GUI
instruments_checkbuttons_list = []
# list of lists containing all widget names inside the instruments frames
widget_instruments_array=[]
widget_instruments_tuple_array=[]

memory_used = 0 

realtime_data_flag = 0

def get_memory_state():
    send_command = GET + "-" + MEMORY_STATE + "(0,0)"
    TCP_Tx_Rx(send_command)
    MetOceanCommand.parseMetOceanCommand(received_MetOcean_data)
    execute_command()
    

def realtime_data(time_to_start_realtime):
    global DEFAULT_KEEP_ALIVE
    global realtime_data_flag
    global realtime_file_path
    global MetOceanCommand
    global realtime_file
    if (time_to_start_realtime != 0):
        now = datetime.datetime.utcnow()
        uct_seconds = (now - datetime.datetime(1970, 1, 1)).total_seconds()
        time_to_start_realtime = float(time_to_start_realtime) - float(uct_seconds)
    print("DENTRO DEL THREAD: ")
    print(time_to_start_realtime)
    
    time.sleep(float(time_to_start_realtime))
    realtime_file = open(realtime_file_path, "w")
    realtime_data_flag = 1
    while (realtime_data_flag == 1):
        send_command = GET + "-" + REALTIME_DATA + "(9,0)"
        print("DATO EN TIEMPO REAL")
        
        TCP_Tx_Rx(send_command)
        MetOceanCommand.parseMetOceanCommand(received_MetOcean_data)
        execute_command()
        
        time.sleep(2)
    realtime_file.close()
        

def erase_memory():
    if tkMessageBox.askyesno("Erase Memory", "Do you want to delete all stored data?"):
        send_command = SET + "-" + MEMORY_ERASE + "(0,0)"
        TCP_Tx_Rx(send_command)
        get_memory_state()
        

def save_data_file():
    datafile = tkFileDialog.asksaveasfilename(title = "Select file to save data")
    if (datafile != ""):
        global delayed_data_file
        global memory_used
        get_memory_state()
        if (memory_used > 0):
            delayed_data_file = open(datafile, "w")
            first_address_to_read = 0
            print("MEMORIA USADA")
            print(memory_used)
            if(int(memory_used) > 1023):
                
                last_address_to_read = 1023
                while (1):
                    print(memory_used)
                    print("DENTRO DEL IF")
                    send_command = GET + "-" + MEMORY_DOWNLOAD + "(" + MEMORY_READ_ADDRESS + "," + str(first_address_to_read) + ";" + MEMORY_READ_BUFFER_LENGTH + "," + str(last_address_to_read - first_address_to_read) + ")"
                    print(send_command)
                    TCP_Tx_Rx(send_command)
                    MetOceanCommand.parseMetOceanCommand(received_MetOcean_data)
                    execute_command()
                    first_address_to_read = last_address_to_read + 1
                    last_address_to_read = first_address_to_read + 1023
                    if (last_address_to_read > int(memory_used)):
                        break
            send_command = GET + "-" + MEMORY_DOWNLOAD + "(" + MEMORY_READ_ADDRESS + "," + str(first_address_to_read) + ";" + MEMORY_READ_BUFFER_LENGTH + "," + str(int(memory_used) - first_address_to_read) + ")"
            print(send_command)
            TCP_Tx_Rx(send_command)
            MetOceanCommand.parseMetOceanCommand(received_MetOcean_data)
            execute_command()
            delayed_data_file.close()



def execute_command():
    global realtime_file
    global delayed_data_file
    if (MetOceanCommand.primary_command == GET):
        if(MetOceanCommand.secondary_command == INSTRUMENT):
            #Actualize all selected instrument widgets as stated in the argument list
            for argument in MetOceanCommand.arguments:
                argument_pair = argument.split(",")
                if (argument_pair[0] == INSTRUMENT_ID):
                    instrument_id = argument_pair[1]
                   
                    print("EL INSTRUMENTO ES: " +instrument_id)
                elif (argument_pair[0] == TYPE_INSTRUMENT):
                    if (argument_pair[1] == SBE_37SM):
                        instrument_type = "SBE 37SMP"
                        instrument_value = SBE_37SM
                    elif (argument_pair[1] == NONE):
                        instrument_type = "NONE"
                        instrument_value = NONE
                elif (argument_pair[0] == INTERVAL):
                    interval = argument_pair[1]
                elif (argument_pair[0] == SAMPLES_PER_INTERVAL):
                    samples_interval = argument_pair[1]
                elif (argument_pair[0] == PROCESSING):
                    if(argument_pair[1] == NONE):
                        processing_type  = "NONE"
                        processing_value = NONE_p
                    elif (argument_pair[1] == AVERAGE):
                        processing_type = "Average"
                        processing_value = AVERAGE_p
                    elif (argument_pair[1] == MEDIAN):
                        processing_type = "Median"
                        processing_value = MEDIAN_p
 
            for actual_widget in widget_instruments_array[int(instrument_id)]:
                if actual_widget[0] == 'Instrument':
                    
                    get_command ="type =" + actual_widget[1] + ".set( \"" + instrument_type + "\")"
                    exec(get_command)
                if actual_widget[0] == 'Acquisition_Interval':
                    get_command = "type =" + actual_widget[1] + ".set( \"" + interval + "\")"
                    exec(get_command)
                if actual_widget[0] == 'Samples_Acquisition':
                    get_command = "type =" + actual_widget[1] + ".set( \"" + samples_interval + "\")"
                    exec(get_command)
                if actual_widget[0] == 'Processing':
                    get_command = "type =" + actual_widget[1] + ".set( \"" + processing_type + "\")"
                    exec(get_command)
                
        elif(MetOceanCommand.secondary_command == MODE):
            argument_pair = MetOceanCommand.arguments[0].split(",")
            if (argument_pair[1] == CONFIGURING):
                for instrument_chb in instruments_checkbuttons_list:
                    command = instrument_chb +".config(state=NORMAL)"
                    exec(command)
                print("Configurando boton de parada de adquisicion")
                stop_bt.config(state = DISABLED)
                printLineState("Datalogger in ", "black")
                printLineState("ADMINISTRATION ", "blue")
                printLineState("mode \n","black")
            else:
                printLineState("Datalogger in ", "black")
                printLineState("ACQUISITION ", "blue")
                printLineState("mode \n","black")
        elif(MetOceanCommand.secondary_command == DATE_TIME):
            for argument in MetOceanCommand.arguments:
                argument_pair = argument.split(",")
                print(argument_pair)
                if (argument_pair[0] == YEAR):
                    year = argument_pair[1]
                elif (argument_pair[0] == MONTH):
                    month = argument_pair[1]
                elif(argument_pair[0] == DAY):
                    day = argument_pair[1]
                elif(argument_pair[0] == HOUR):
                    hours = argument_pair[1]
                elif(argument_pair[0] == MINUTE):
                    minutes = argument_pair[1]
                else:
                    seconds = argument_pair[1]
            print (datetime.datetime.now().strftime("%c"))
            global datalogger_date_time
            datalogger_date_time = datetime.datetime(int(year),int( month), int(day), int(hours), int(minutes), int(seconds))
            #datalogger_date_time.date(year, month, day)
            datetime_variable.set("Client's date/time: \n\t  " + datetime.datetime.now().strftime("%c") + "\nDatalogger's date/time: \n\t" + month + "/" + day + "/"+ year+ " " + hours + ":" + minutes + ":" + seconds)
        elif(MetOceanCommand.secondary_command == REALTIME_DATA):
            if (MetOceanCommand.arguments[0] != "0,0"):
                text_data.insert(END, MetOceanCommand.arguments[0])
                print("AHORA A IMPRIMIR EL DATO")
                realtime_file.write(MetOceanCommand.arguments[0])
        elif(MetOceanCommand.secondary_command == MEMORY_STATE):
            memory_arguments = MetOceanCommand.arguments[0].split(",")
            global memory_used
            memory_used = memory_arguments[1]
            memory_used = 127
            memory_state_text = "\nMemory used: " + str(memory_used) + " Bytes"
            memory_state_variable.set(memory_state_text)
        elif(MetOceanCommand.secondary_command == MEMORY_DOWNLOAD):
            delayed_data_file.write(MetOceanCommand.arguments[0])
            

def command_send(command):
    #sck.connect((server_ip, server_port))
    sck.send(command)


# Function for creating one string containing the command for a new widget creation in the configuration tab of the GUI
def create_New_Widget_Command(aux_widget_name, function, arguments):
    aux_arguments = "("
    number_of_arguments =len(arguments)
    i = 1;
    for arg in arguments:
        if i != number_of_arguments:
            aux_arguments = aux_arguments + arg  + ","
        else:
            aux_arguments = aux_arguments + arg
        i = i+1
    aux_arguments = aux_arguments + ")"
    return(aux_widget_name + "=" + function + aux_arguments)

   
def checkFunc(inst):
    command = "cbstate = cb_bool_{}.get()".format(inst)
    exec(command)
    
    widget_name = "cb_enable_{}".format(str(inst))
    for actual_widget in widget_instruments_array[inst]:
        print(actual_widget)
        print(actual_widget[1])
        if cbstate == 1:
            try:
                command_text = actual_widget[1] + ".config(state = NORMAL)"          
                exec(command_text)
            except:
                continue
        else:
            try:
                command_text = actual_widget[1] + ".config(state = DISABLED)"          
                exec(command_text)
            except:
                continue
        for instrument_chb in instruments_checkbuttons_list:
           command = instrument_chb +".config(state=NORMAL)"
           exec(command)
    
    
def printLineState(line, tag):
    text_state.config(state = NORMAL)
    text_state.insert(END, line, (tag))
    text_state.yview_moveto(1)

        

def tcp_client( server_ip, server_port, test_keepalive, command ):
    message_count=0;
    global realtime_data_flag
    
    
    print "Starting tcp client"
    print(COMMAND)
    try:
        host_name = socket.gethostname()
        host_ip = socket.gethostbyname(host_name)
        return_value = SUCCESS
    except:
        return_value = UNABLE_TO_GET_HOST_IP
    finally:
        if return_value == UNABLE_TO_GET_HOST_IP:
            return return_value
    server_ip_split = server_ip.split(".")
    host_ip_split = host_ip.split(".")
    if (server_ip_split[0] != host_ip_split[0]) or (server_ip_split[1] != host_ip_split[1])or (server_ip_split[2] != host_ip_split[2]):
        return NETWORK_MISTMATCH
    #try:
    sck = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sck.connect((server_ip, server_port))
        #return_value = SUCCESS
    #except:
    #    return_value = UNABLE_TO_CONNECT_TO_SERVER
    #finally:
        
    #if connect == True:
            #print "Send message: %s\r\n" % MESSAGE
    sck.send(command)
    data = sck.recv(BUFFER_SIZE)
    print("Recibido: " + data)
    global received_MetOcean_data
    received_MetOcean_data = data
    #if ( test_keepalive == 1 ):
    #    while (realtime_data_flag == 1):
    #       text_data.insert(END, "HOLA MI PINGUINITA \n")
    #      time.sleep(5)
    #     data = sck.recv(BUFFER_SIZE)
    #    print(data)
    sck.close()
    return return_value

            
def connectFunction():
    #connectFunction_1(COMMAND)
    printLineState("Connecting... \n", "black")
    #TCP_Tx_Rx("0-0(0,2;1,1)")
    send_command = GET + "-" + MODE + "(" + TYPE_MODE + ",0)"
    if (TCP_Tx_Rx(send_command) == SUCCESS):
        printLineState("Connected to the datalogger \n", "green")
        MetOceanCommand.parseMetOceanCommand(received_MetOcean_data)
        execute_command()       
##        for instrument_chb in instruments_checkbuttons_list:
#           command = instrument_chb +".config(state=NORMAL)"
#           exec(command)
        button_upload.config(state=NORMAL)
        button_refresh.config(state=NORMAL)
        button_connect.config(state=DISABLED)
        principal1.tab(1, state="normal")
        get_datetime()
        get_memory_state()
    else:
        printLineState("Unable to connect to the datalogger \n", "red")
    
    
def TCP_Tx_Rx(command):
    #printLineState("Connecting... \n", "black")
    parser = optparse.OptionParser()
    parser.add_option("--hostip", dest="hostip", default=DEFAULT_IP, help="Hostip to listen on.")
    parser.add_option("-p", "--port", dest="port", type="int", default=DEFAULT_PORT, help="Port to listen on [default: %default].")
    parser.add_option("--test_keepalive", dest="test_keepalive", type="int", default=DEFAULT_KEEP_ALIVE, help="Test keepalive capability")
    (options, args) = parser.parse_args()
    connect = tcp_client(options.hostip, options.port, options.test_keepalive, command)
    return connect

def single_instrument_upload(inst):
    print(inst)
    send_command = SET + "-"+ INSTRUMENT + "("
    for actual_widget in widget_instruments_array[inst]:
        try:
            if actual_widget[0] == 'Instrument':
                get_command ="type =" + actual_widget[1] + ".get()"
                exec(get_command)
                if type == "SBE 37SMP":
                    typevalue = SBE_37SM
                if type == "NONE":
                    typevalue = NONE
                if type == "":
                    value_processing = "Select one value for Type Instrument"
                    raise ValueError
                send_command = send_command + INSTRUMENT_ID + ","+ str(inst) + ";" + TYPE_INSTRUMENT + "," + typevalue
            if actual_widget[0] == 'Acquisition_Interval':
                get_command = "interval =" + actual_widget[1] + ".get()"
                exec(get_command)
                send_command = send_command + ';'
                send_command = send_command  + INTERVAL + ',' + str(interval)
                if interval == "":
                    value_processing = "Add one value for Acquisition Interval"
                    raise ValueError
                if (interval.isdigit() == 0):
                    value_processing = "Acquisition Interval must be positive integer"
                    raise ValueError
            if actual_widget[0] == 'Samples_Acquisition':
                get_command = "samples_per_acquisition =" + actual_widget[1] + ".get()"
                exec(get_command)
                send_command = send_command + ';'
                send_command = send_command + SAMPLES_PER_INTERVAL + ',' + str(samples_per_acquisition)
                if samples_per_acquisition == "":
                    value_processing = "Add one value for Samples per Acquisition"
                    raise ValueError
                if (samples_per_acquisition.isdigit() == 0):
                    value_processing = "Samples per Acquisition must be  positive integer"
                    raise ValueError
            if actual_widget[0] == 'Processing':
                get_command = "processing =" + actual_widget[1] + ".get()"
                exec(get_command)
                send_command = send_command + ';'
                if processing == "NONE":
                    type_processing = NONE_p
                if processing == "Median":
                    type_processing = MEDIAN_p
                if processing == "Average":
                    type_processing = AVERAGE_p
                    send_command = send_command + PROCESSING + ',' + type_processing
                if processing == "":
                    value_processing = "Select one value for Acquisition processing"
                    raise ValueError
        except ValueError:
            tkMessageBox.showinfo("Instrument configuration", value_processing)
            return
            
    send_command = send_command + ")"
    #COMMAND = send_command
    #print(COMMAND)
    TCP_Tx_Rx(send_command)

def upload_all():
    for i in range(8):
        cb_boolean_variable = "boolean_value = cb_bool_{}".format(str(i)) + ".get()"
        exec(cb_boolean_variable) 
        if boolean_value == 1:
            single_instrument_upload(i)
    
def single_instrument_refresh(inst):
    send_command = GET + "-" + INSTRUMENT + "(" + INSTRUMENT_ID + "," +  str(inst) + ")"
    TCP_Tx_Rx(send_command)
    MetOceanCommand.parseMetOceanCommand(received_MetOcean_data)
    execute_command()
    
def refresh_all():
    for i in range(8):
        single_instrument_refresh(i)

# Send the command for setting the datalogger in acquisition mode
def setAquisitionMode():
    print(start_rd.get())

    if (start_rd.get() == 1):
        try:
            delayed_acquire_date_time = start_en.get()
            value_processing = "Please, insert a valid date & time value:\n MM/DD/YY hh:mm:ss"
            aux_fields_delayed_acquire_date = delayed_acquire_date_time.split("/")
            print(aux_fields_delayed_acquire_date)
            print(len(aux_fields_delayed_acquire_date))
            if(len(aux_fields_delayed_acquire_date) != 3):
                raise ValueError
            #value_processing = "month"
            month = aux_fields_delayed_acquire_date[0]
            #value_processing = "day"
            day = aux_fields_delayed_acquire_date[1]
            aux_fields_delayed_acquire_date = aux_fields_delayed_acquire_date[2].split(" ")
            print("Pillando el espacio")
            print(len(aux_fields_delayed_acquire_date))
            if(len(aux_fields_delayed_acquire_date) != 2):
                raise ValueError
            #value_processing = "year"
            print(aux_fields_delayed_acquire_date)
            year = aux_fields_delayed_acquire_date[0]
            print("Pillando los :")
            aux_fields_delayed_acquire_date = aux_fields_delayed_acquire_date[1].split(":")
            
            print(len(aux_fields_delayed_acquire_date))
            if(len(aux_fields_delayed_acquire_date) != 3):
                raise ValueError
            hours = aux_fields_delayed_acquire_date[0]
            hours_int = int(hours)
            if ((hours_int < 0) or (hours_int > 23)): 
                value_processing = "The hours value must be between 00 and 23"
                raise ValueError
            #value_processing = "minutes"
            minutes = aux_fields_delayed_acquire_date[1]
            minutes_int = int(minutes)
            if ((minutes_int < 0)or(minutes_int > 59)):
                value_processing = "The minutes value must be between 00 and 59"
                raise ValueError
            #value_processing = "seconds"
            seconds=aux_fields_delayed_acquire_date[2]
            seconds_int = int(seconds)
            if ((seconds_int < 0)or(seconds_int > 59)):
                value_processing = "The seconds value must be between 00 and 59"
                raise ValueError
            #value_processing = "year"
            year_int = int(year)
            if ((year_int < 0)or(year_int > 99)):
                value_processing = "The year value must be between 00 and 99"
                raise ValueError
            #value_processing = "month"
            month_int = int(month)
            if ((month_int < 0)or(month_int > 12)):
                value_processing = "The month value must be between 01 and 12"
                raise ValueError
            #value_processing = "day"
            day_int = int(day)
            if((month_int == 1)or(month_int == 3)or(month_int == 5)or(month_int == 7)or(month_int == 8)or(month_int == 10)or(month_int == 12)):
                if((day_int < 0)or(day_int > 31)):
                    value_processing = "The day value for month {} must be between 01 and 31".format(month)
                    raise ValueError
            elif((month_int == 4)or(month_int == 6)or(month_int == 9)or(month_int == 11)):
                if((day_int < 0)or(day_int > 30)):
                    value_processing = "The day value for month {} must be between 01 and 30".format(month)
                    raise ValueError
            else:
                if(((year_int % 4) == 0) and ((year_int % 100) != 0) or ((year_int % 400) == 0)):
                    if((day_int < 0)or(day_int > 29)):
                        value_processing = "The day value for month {} and year {} must be between 01 and 29".format(month, year)
                        raise ValueError
                else:
                    if((day_int < 0)or(day_int > 28)):
                        value_processing = "The day value for month {} and year {} must be between 01 and 28".format(month, year)
                        raise ValueError                    
                    
        except ValueError:
            tkMessageBox.showinfo("Delayed acquisition", value_processing)
            return
        get_datetime()
        valid_delayed_acquisition_date_time = datetime.datetime(year_int+2000, month_int, day_int, hours_int, minutes_int, seconds_int)
        print(valid_delayed_acquisition_date_time)
        print(datalogger_date_time)
        if (valid_delayed_acquisition_date_time < datalogger_date_time):
            tkMessageBox.showinfo("Delayed acquisition", "Acquisition date and time is earlier that the datalogger's date and time")
            return
        aux_utc_seconds_start = str((valid_delayed_acquisition_date_time - datetime.datetime(1970, 1, 1)).total_seconds())
        utc_seconds_start = aux_utc_seconds_start.split(".")[0]
    else:
        utc_seconds_start = "0"
    
         #UTC_SECONDS_ACQUIRE_START

    if (enable_realtime_variable.get() == 1):
        global realtime_file_path
        realtime_file_path = tkFileDialog.asksaveasfilename(title = "Select file to save real time data", filetypes = [("MetOcean data files", "*.mod")])
        if (realtime_file_path == ""):
            return
        
    send_command = SET + "-" + MODE + "(" + TYPE_MODE + "," + ACQUIRING + ";" + UTC_SECONDS_ACQUIRE_START + "," + utc_seconds_start + ";" + MEMORY_WRITE_FLAG + "," + str(store_acquired_variable.get()) +  ")"     
        #print( year, month, day, hours, minutes, seconds)
        #print(fields_delayecd_acquire_date)
    #send_command = SET + "-" + MODE + "(" + TYPE_MODE + "," + ACQUIRING + ")"
    TCP_Tx_Rx(send_command) 
    if (enable_realtime_variable.get() == 1):
        print("TIEMPO PARA EMPEZAR EL TIEMPO REAL: " + utc_seconds_start)
        print(float(utc_seconds_start))
        t = threading.Thread(target=realtime_data, args=(float(utc_seconds_start),))
        t.start()
        
    printLineState("Datalogger in ", "black")
    printLineState("ACQUISITION ", "blue")
    printLineState("mode \n","black")
    if (start_rd.get() == 1):
        printLineState("\t Waiting to start on ", "blue")
        printLineState(month + "/" + day + "/20" + year + " at " + hours + ":" + minutes + ":" + seconds + "\n", "blue")
    principal1.tab(0, state = DISABLED)
    start_bt.config(state = DISABLED)
    stop_bt.config(state = NORMAL)
    start_rb_1.config(state = DISABLED)
    start_rb_2.config(state = DISABLED)
    getdatetme_bt.config(state = DISABLED)
    setdatetme_bt.config(state = DISABLED)
    erasememory_bt.config(state  = DISABLED)
    downloaddata_bt.config(state  = DISABLED)
    enable_realtime_cb.config(state = DISABLED)
    store_data_memory.config(state = DISABLED)

# Send the command for setting the datalogger in configuration mode
def setConfigurationMode():
    global realtime_data_flag
    realtime_data_flag = 0

    time.sleep(2)
    
    send_command = SET + "-" + MODE + "(" + TYPE_MODE + "," + CONFIGURING + ")"
    TCP_Tx_Rx(send_command) 
    printLineState("Datalogger in ", "black")
    printLineState("ADMINISTRATION ", "blue")
    printLineState("mode \n","black")
    principal1.tab(0, state = NORMAL)
    start_bt.config(state = NORMAL)
    stop_bt.config(state = DISABLED)
    start_rb_1.config(state = NORMAL)
    start_rb_2.config(state = NORMAL)
    getdatetme_bt.config(state = NORMAL)
    setdatetme_bt.config(state = NORMAL)
    erasememory_bt.config(state  = NORMAL)
    downloaddata_bt.config(state  = NORMAL)
    enable_realtime_cb.config(state = NORMAL)
    store_data_memory.config(state = NORMAL)
    get_memory_state()


def get_datetime():
    send_command = GET + "-" + DATE_TIME + "(0,0)"
    TCP_Tx_Rx(send_command)
    MetOceanCommand.parseMetOceanCommand(received_MetOcean_data)
    execute_command()
    #datetime_variable.set("Client's date/time: \n \nDatalogger's date/time: ")
    

def set_datetime():
    now = datetime.datetime.utcnow()
    uct_seconds = str((now - datetime.datetime(1970, 1, 1)).total_seconds())
    send_command = SET + "-" + DATE_TIME + "(" + UTC_SECONDS + "," + uct_seconds.split(".")[0] + ")"
    TCP_Tx_Rx(send_command)
    print(uct_seconds.split(".")[0])
   
   
    
# Global variables
sck = socket.socket(socket.AF_INET, socket.SOCK_STREAM)    
    
if __name__ == '__main__':


    # Create the GUI
    top = Tk()
    top.resizable(False, False)
    top.title("MetOcean Datalogger GUI")
    principal1 = ttk.Notebook(top)
    tab1 = ttk.Frame(principal1)
    tab2 = ttk.Frame(principal1)
    principal1.add(tab1, text="Configuration")
    principal1.add(tab2, text="Acquisition", state = DISABLED)
    #principal1.add(tab2, text="Acquisition")
    # Creation and placement of the instruments configuration widgets in the GUI
    frame_row = 0
    frame_column = 0
    for instrument in range(NUMBER_OF_INSTRUMENTS):
        widget_name = "frame_" + str(instrument)
        #label_text = "Instrument "+ str(instrument)
        #create_widget_command = widget_name + " = ttk.Labelframe(tab1, text = label_text)"
        widget_command = create_New_Widget_Command(widget_name, "ttk.Labelframe", ["tab1", "text = \"Instrument {}\".format(str(instrument+1))"])

        # Create the list of this instrument widgets
        widget_list_name = "widget_list_" + str(instrument)
        update_list_command = widget_list_name + "=[]"
        exec(update_list_command)
        
        # Add new widget to the widget list
        instrument_tuple = ('main_frame', widget_name)
        #update_list_command = widget_list_name+".append({})".format(widget_name)
        update_list_command = widget_list_name+".append(instrument_tuple)"
        
        
        
        #print(widget_command + "\n")
        #print(update_list_command + "\n")
        
        exec(widget_command)
        exec(update_list_command)
        #Place the main frame of the instrument configuration
        if frame_column == NUMBER_OF_INSTRUMENTS_PER_ROW:
            frame_column = 0
            frame_row = frame_row +1
            
        exec(widget_name + ".grid(row = {}, column = {}, sticky = W, pady = 2)".format(frame_row, frame_column))
        frame_column = frame_column +1
        #sensor1_frame.grid(row = 0, column = 0, sticky = W, pady = 2)

        
        # Create and place the enable checkbutton
        cb_boolean_variable = "cb_bool_{}".format(str(instrument)) + "= IntVar()"
        exec(cb_boolean_variable)
        #print(cb_boolean_variable)
        widget_name = "cb_enable_{}".format(str(instrument))
        widget_options = ["frame_" + str(instrument), "text = \"Enabled\"", "command = " + "lambda: checkFunc({})".format(str(instrument)), "variable =" + "cb_bool_{}".format(str(instrument)), "state = DISABLED"]
        widget_command = create_New_Widget_Command(widget_name, "ttk.Checkbutton", widget_options)
        #print(widget_command + "\n")

        instrument_tuple = ('Enable', widget_name)
        update_list_command = widget_list_name+".append(instrument_tuple)"
  
       # print(create_widget_command + "\n")
        exec(widget_command)
        exec(update_list_command)
        exec(widget_name + ".grid(row = 5, column = 0, sticky = W, pady = 2)")
        

        
        instruments_checkbuttons_list.append(widget_name)
        print(instruments_checkbuttons_list[instrument])
        

        
        # Create and place the instrument selecction label
        
        #sensor1_lb_sensor = ttk.Label(sensor1_frame, text = "Sensor: ")
        widget_name = "lb_sensor_{}".format(str(instrument))
        widget_options = ["frame_" + str(instrument), "text = \"Instrument: \"", "state = DISABLED"]
        widget_command = create_New_Widget_Command(widget_name, "ttk.Label", widget_options)
        instrument_tuple = ('Ins_Label', widget_name)
        update_list_command = widget_list_name+".append(instrument_tuple)"
 
        #print(widget_command + "\n")
        exec(widget_command)
        exec(update_list_command)
        exec(widget_name + ".grid(row = 0, column = 0, sticky = W, pady = 2)")
 
 
 
        # Create and place the baud selecction label
        #widget_name = "lb_baud_{}".format(str(instrument))
        #widget_options = ["frame_" + str(instrument), "text = \"Baud Rate: \"", "state = DISABLED"]
       # widget_command = create_New_Widget_Command(widget_name, "ttk.Label", widget_options)
        #instrument_tuple = ('Baud_Label', widget_name)
        #update_list_command = widget_list_name+".append(instrument_tuple)"
        #print(widget_command + "\n")
        #exec(widget_command)
        #exec(update_list_command)
        #exec(widget_name + ".grid(row = 1, column = 0, sticky = W, pady = 2)")

 
        # Create and place the instrument selecction combo
        cm_text_variable = "cm_instrument_{}".format(str(instrument)) + "= IntVar()"
        exec(cm_text_variable)
        widget_name = "cm_instrument_{}".format(str(instrument))
        widget_options = ["frame_" + str(instrument), "values = [\"NONE\", \"SBE 37SMP\"]", "width = 12", "textvariable =" + "cm_instrument_{}".format(str(instrument)), "state = DISABLED"]
        widget_command = create_New_Widget_Command(widget_name, "ttk.Combobox", widget_options)
        instrument_tuple = ('Instrument', widget_name)
        update_list_command = widget_list_name+".append(instrument_tuple)"
        #print(widget_command + "\n")
        exec(widget_command)
        exec(update_list_command)
        exec(widget_name + ".grid(row = 0, column = 1, columnspan = 3, sticky = W, pady = 2)")
         

        # Create and place the acquisition interval entry
        widget_name = "en_interval_{}".format(str(instrument))
        widget_options = ["frame_" + str(instrument), "width = 12", "state = DISABLED"]
        widget_command = create_New_Widget_Command(widget_name, "ttk.Entry", widget_options)
        instrument_tuple = ('Acquisition_Interval', widget_name)
        update_list_command = widget_list_name+".append(instrument_tuple)"
        #print(widget_command + "\n")
        exec(widget_command)
        exec(update_list_command)
        exec(widget_name + ".grid(row = 2, column = 1, columnspan = 3, sticky = W, pady = 2)")

 
        # Create and place the samples per acquisition entry
        widget_name = "en_samples_{}".format(str(instrument))
        widget_options = ["frame_" + str(instrument), "width = 12", "state = DISABLED"]
        widget_command = create_New_Widget_Command(widget_name, "ttk.Entry", widget_options)
        instrument_tuple = ('Samples_Acquisition', widget_name)
        update_list_command = widget_list_name+".append(instrument_tuple)"
        #print(widget_command + "\n")
        exec(widget_command)
        exec(update_list_command)
        exec(widget_name + ".grid(row = 3, column = 1, columnspan = 3, sticky = W, pady = 2)")
 


        # Create and place the acquisition interval label
        widget_name = "lb_acquisition_{}".format(str(instrument))
        widget_options = ["frame_" + str(instrument), "text = \"Acquisition Interval (s): \"", "state = DISABLED"]
        widget_command = create_New_Widget_Command(widget_name, "ttk.Label", widget_options)
        instrument_tuple = ('Acquisition_Label', widget_name)
        update_list_command = widget_list_name+".append(instrument_tuple)"
        #print(widget_command + "\n")
        exec(widget_command)
        exec(update_list_command)
        exec(widget_name + ".grid(row = 2, column = 0, sticky = W, pady = 2)")

 
        # Create and place the samples per acquisition label
        widget_name = "lb_samples_{}".format(str(instrument))
        widget_options = ["frame_" + str(instrument), "text = \"Samples per Acquisition: \"", "state = DISABLED"]
        widget_command = create_New_Widget_Command(widget_name, "ttk.Label", widget_options)
        instrument_tuple = ('Samples_Label', widget_name)
        update_list_command = widget_list_name+".append(instrument_tuple)"
        #print(widget_command + "\n")
        exec(widget_command)
        exec(update_list_command)
        exec(widget_name + ".grid(row = 3, column = 0, sticky = W, pady = 2)")
        

        # Create and place the processing method selecction combo
        widget_name = "cm_processing_{}".format(str(instrument))
        widget_options = ["frame_" + str(instrument), "values = [\"NONE\", \"Median\", \"Average\"]", "width = 12", "state = DISABLED"]
        widget_command = create_New_Widget_Command(widget_name, "ttk.Combobox", widget_options)
        instrument_tuple = ('Processing', widget_name)
        update_list_command = widget_list_name+".append(instrument_tuple)"
        #print(widget_command + "\n")
        exec(widget_command)
        exec(update_list_command)
        exec(widget_name + ".grid(row = 4, column = 1, columnspan = 3, sticky = W, pady = 2)")



        # Create and place the processing method label
        widget_name = "lb_processing_{}".format(str(instrument))
        widget_options = ["frame_" + str(instrument), "text = \"Acquisition processing: \"", "state = DISABLED"]
        widget_command = create_New_Widget_Command(widget_name, "ttk.Label", widget_options)
        instrument_tuple = ('Processing_Label', widget_name)
        update_list_command = widget_list_name+".append(instrument_tuple)"
        #print(widget_command + "\n")
        exec(widget_command)
        exec(update_list_command)
        exec(widget_name + ".grid(row = 4, column = 0, sticky = W, pady = 2)")


        # Create and place the upload button
        widget_name = "bt_upload_{}".format(str(instrument))
        widget_options = ["frame_" + str(instrument), "text = \"Upload\"","width = 7", "command = " + "lambda: single_instrument_upload({})".format(str(instrument)), "state = DISABLED"]
        widget_command = create_New_Widget_Command(widget_name, "ttk.Button", widget_options)
        instrument_tuple = ('Upload', widget_name)
        update_list_command = widget_list_name+".append(instrument_tuple)"
        #print(widget_command + "\n")
        exec(widget_command)
        exec(update_list_command)
        exec(widget_name + ".grid(row = 5, column = 3, sticky = W, pady = 2)")


        # Create and place the refresh button
        widget_name = "bt_refresh_{}".format(str(instrument))
        widget_options = ["frame_" + str(instrument), "text = \"Refresh\"","width = 7", "command = " + "lambda: single_instrument_refresh({})".format(str(instrument)), "state = DISABLED"]
        widget_command = create_New_Widget_Command(widget_name, "ttk.Button", widget_options)
        instrument_tuple = ('Refresh', widget_name)
        update_list_command = widget_list_name+".append(instrument_tuple)"
        #print(widget_command + "\n")
        exec(widget_command)
        exec(update_list_command)
        exec(widget_name + ".grid(row = 5, column = 2, sticky = W, pady = 2)")
 
        # Update the array with the last created instruments widgets list
        update_array_command = "widget_instruments_array"+".append({})".format(widget_list_name)
        exec(update_array_command)
    frame_text = ttk.Frame(top)
    scrollb = Scrollbar(frame_text)
    text_state = Text(frame_text, height=8, width = 120, yscrollcommand=scrollb.set, borderwidth = 3, relief = GROOVE)
    scrollb.config(command=text_state.yview)
    text_state.insert(END, "Client disconected. Connect to start.\n")
    text_state.config(state = DISABLED)
    text_state.tag_config("green", foreground="green")
    text_state.tag_config("red", foreground="red")
    text_state.tag_config("black", foreground="black")
    text_state.tag_config("blue", foreground="blue")
    
    frame_text.grid(row = 3, column = 0, columnspan = 5, sticky = W, pady = 2)
    text_state.grid(row = 0, column = 0, sticky = W, pady = 2, padx = 2)
    scrollb.grid(row = 0, column = 1, sticky = NS, pady = 2)
    
    frame_buttons = ttk.Frame(tab1)
    
    button_connect = ttk.Button(tab1, text="Connect", command = connectFunction)
    button_upload = ttk.Button(frame_buttons, text="Upload All", state = DISABLED, command = upload_all)
    button_refresh = ttk.Button(frame_buttons, text="Refresh All", state = DISABLED, command = refresh_all)
    
    button_connect.grid(row = 3, column = 0, sticky = W, pady = 2)
    button_upload.grid(row = 0, column = 1, sticky = E, pady = 2)
    button_refresh.grid(row = 0, column = 0, sticky = W, pady = 2)
    
    frame_buttons.grid(row = 3, column = 3, sticky = E, pady = 2)
    logo = PIL.ImageTk.PhotoImage(PIL.Image.open("logo_v4.png"))
    Logo_Label = Label(top, image=logo)
    Logo_Label.grid(row = 0, column = 0, columnspan = 3, sticky = W, pady = 2)
    principal1.grid(row = 1, column = 0, sticky = W, pady = 2)
    
    ########################################################################################
    ##                          Acquisition tab
    ########################################################################################
    frame_acq = ttk.Frame(tab2)
    frame_acq.grid(column = 0, row = 0)
    frame_start= ttk.Labelframe(frame_acq, text = "Start/Stop Aquisition")
    frame_start.grid(column = 0, row=0, pady = 2)

    start_rd = IntVar()
    start_rd.set(0)
    start_rb_1 = ttk.Radiobutton(frame_start, variable=start_rd, value=0, text = "Start now")
    start_rb_2 = ttk.Radiobutton(frame_start, variable=start_rd, value=1, text = "Start at: ")
    start_rb_1.grid(column = 0, row = 0, sticky = W)
    start_rb_2.grid(column = 0, row = 1, sticky = W)
    start_en = ttk.Entry(frame_start, width =16)
    start_en.grid(column = 1, row = 1)
    start_bt = ttk.Button(frame_start, text = "Start Acquisition", command = setAquisitionMode)
    stop_bt = ttk.Button(frame_start, text = "Stop Acquisition" , command = setConfigurationMode)
    start_bt.grid(column = 0, row =2)
    stop_bt.grid(column = 1, row =2)
 
    store_acquired_variable = IntVar()
    store_data_memory = ttk.Checkbutton(frame_start, text = "Store acquired data to memory", variable = store_acquired_variable)
    store_acquired_variable.set(1)
    store_data_memory.grid(column = 0, columnspan = 2, row = 3, pady = 3, sticky = W)
    enable_realtime_variable = IntVar()
    enable_realtime_cb = ttk.Checkbutton(frame_start, text = "Enable realtime data", variable = enable_realtime_variable)
    enable_realtime_cb.grid(column = 0, columnspan = 2, row = 4, pady = 2, sticky = W)
    enable_realtime_variable.set(0)
    realtime_fr = ttk.Frame(frame_start)
    realtime_rd = IntVar()
    realtime_rb_1 = ttk.Radiobutton(realtime_fr, variable=realtime_rd, value=0, text = "Real Time mode")
    realtime_rb_2 = ttk.Radiobutton(realtime_fr, variable=start_rd, value=1, text = "Delayed mode")
    realtime_rb_1.grid(column = 0, row = 0)
    realtime_rb_2.grid(column = 0, row = 1)


    frame_datetime = ttk.Labelframe(frame_acq, text ="Set/Get Date Time")
    frame_datetime.grid(column = 0, row = 1, sticky = EW, pady = 2)

    getdatetme_bt = ttk.Button(frame_datetime, text = "Get Date Time", command = get_datetime)
    getdatetme_bt.grid(column = 0, row = 0, padx = 8)
    setdatetme_bt = ttk.Button(frame_datetime, text = "Set Date Time", command = set_datetime)
    setdatetme_bt.grid(column = 1, row = 0, padx = 8)
    datetime_variable=StringVar()
    datetime_lb = ttk.Label(frame_datetime, textvariable = datetime_variable, justify = LEFT)
    datetime_variable.set("Client's date/time: \n\nDatalogger's date/time: \n")
    datetime_lb.grid(column = 0, row = 1, columnspan = 2, sticky = W)


    frame_memory = ttk.Labelframe(frame_acq, text ="Memory management")
    frame_memory.grid(column = 0, row = 2, pady = 2, sticky = EW)

    erasememory_bt = ttk.Button(frame_memory, text = "Erase memory", command = erase_memory)
    erasememory_bt.grid(column = 0, row = 0, sticky = W)
    downloaddata_bt = ttk.Button(frame_memory, text = "Download data", command = save_data_file)
    downloaddata_bt.grid(column = 1, row = 0, sticky = E)
    memory_state_variable = StringVar()
    memorystate_lb = ttk.Label(frame_memory, textvariable = memory_state_variable, justify = CENTER)
    memory_state_variable.set("\nMemory used")
    memorystate_lb.grid(column = 0, row = 1)
    frame_text_data = ttk.Frame(tab2)
    scrollb_data = Scrollbar(frame_text_data)
    text_data = Text(frame_text_data, height=20, width = 90, background = "black", foreground = "white", yscrollcommand=scrollb_data.set)
    scrollb_data.config(command=text_data.yview)
    
    frame_text_data.grid(row = 0, column = 1, columnspan = 5, sticky = W, pady = 2)
    text_data.grid(row = 0, column = 0, sticky = W, pady = 2)
    scrollb_data.grid(row = 0, column = 1, sticky = NS, pady = 2)

    
    top.mainloop()
            
    