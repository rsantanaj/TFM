
#################################################################
# $ Copyright José Raúl Santana Jiménez $
#
#
#  License Creative Commons 3.0 (by-nc-nd)
#################################################################

NAME := app_METEOCEANOGRAPHIC_DATALOGGER

$(NAME)_SOURCES    := datalogger.c \
						datalogger_MetOcean_TCP_protocol.c \
						datalogger_Sensors.c \
						datalogger_MetOcean_Commands.c \
						datalogger_Acquire.c \
						datalogger_Serial_Port.c \
						datalogger_memory.c

$(NAME)_COMPONENTS += inputs/gpio_button
WIFI_CONFIG_DCT_H := wifi_config_dct.h

VALID_PLATFORMS    := CY8CKIT_062 