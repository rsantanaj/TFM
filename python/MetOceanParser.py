#coding=utf-8
#MetOceanParser.py
# Created on: 16/11/2019
#     Author: José Raúl Santana Jiménez


# This file contains functions and declarations for the MetOcean protocol commands parsing

# License Creative Commons 3.0 (by-nc-nd)

#!/usr/bin/env python

################################################################
# Labels for MetOcean principal commands definition
SET = '0'
GET = '1'
# Labels for MetOcean secondary commands definition
INSTRUMENT = '0'
MODE = '1'
DATE_TIME = '2'
REALTIME_DATA = '3'
MEMORY_DOWNLOAD = '4'
MEMORY_STATE = '5'
MEMORY_ERASE = '6'


#Labels for MetOcean commands arguments definition
INSTRUMENT_ID = '0'
TYPE_INSTRUMENT = '1'
TYPE_MODE = '2'
YEAR = '3'
MONTH = '4'
DAY = '5'
HOUR = '6'
MINUTE = '7'
SECOND = '8'
UTC_SECONDS = '9'
UTC_SECONDS_ACQUIRE_START = '10'
REALTIME_DATA_arg = '11'
MEMORY_STATE_Arg = '12'
MEMORY_WRITE_FLAG = '13'
MEMORY_READ_ADDRESS = '14'
MEMORY_READ_BUFFER_LENGTH = '15'
INTERVAL = '16'
SAMPLES_PER_INTERVAL = '17'
PROCESSING = '18'

# Labels for TYPE_INSTRUMENT argument possible values
NONE = '0'
SBE_37SM = '1'

# Labels for PROCESSING argument possible values

NONE_p = '0'
AVERAGE_p = '1'
MEDIAN_p = '2'

# Labels for TYPE_MODE argument possible values
CONFIGURING = '0' 
ACQUIRING = '1'




class MetOcean_object_command():
    def __init__(self, primary_command, secondary_command, arguments):
        self.principal_command = -1
        self.secondary_command = -1
        self.arguments = {}
        
#Function to parse one MetOcean command received from the datalogger
    def parseMetOceanCommand(self, received_command):

        #Obtain primary command
        substrings = str(received_command).split("-")
        print(substrings)
        auxiliar_command = substrings[1];
        self.primary_command = substrings[0];
        print(self.primary_command)
    
        #Obtain secundary command
        substrings = auxiliar_command.split("(")
        print(substrings)
        self.secondary_command = substrings[0]
        auxiliar_command = substrings[1]
    
        if (self.primary_command == GET):
            #Recover all arguments
            substrings = auxiliar_command.split(")")
            print(substrings)
            auxiliar_command = substrings[0]
            self.arguments = auxiliar_command.split(";")
            print(self.arguments)
            
