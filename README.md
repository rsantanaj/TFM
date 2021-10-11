# Oceanographic and Meteorological Datalogger - Master Thesis
This project contains the code developed for making the [CY8CKIT-062-WiFi-BT](https://www.cypress.com/documentation/development-kitsboards/psoc-6-wifi-bt-pioneer-kit-cy8ckit-062-wifi-bt) board to work as a generic datalogger for high quality oceanographic and meteorological data acquisition.
## Motivation
The main motivation for the existence of this project was to carry out my final Master Thesis in Telecomunication Engineering.
The idea behind the project is to develop one generic, easy to use and low power datalogger with capability to acquire, log and send high quality oceanographic and meteorological data. 
The main features of the datalogger are given by my own  professional experience in the field of ambiental sensors and acquisition systems.
## Requirements
* One CY8CKIT-062-WiFi-BT board.
* PC with WLAN IEEE 802.11 link capability.
* [WICED Studio](https://www.cypress.com/products/wiced-software) SDK.
* Python 2.7.
## Instalation
1. Download and install WICED Studio in the computer.
2. In WICED Studio load the datalogger's project.
3. Connect the CY8CKIT-062-WiFi-BT board to the computer.
4. In WICED, compile the project and load the binaries into the CY8CKIT-062-WiFi-BT board.
## Usage
1. Power on the CY8CKIT-062-WiFi-BT.
2. Connect the WLAN IEEE 802.11 interface of the computer to the datalogger's network:
    >* SSID: *MetOcean Datalogger*
    >* Password: *abcd1234*
    >* Security: *WPA2-PSK*
3. Run the *main.py* script.
## Reference
Full documentation of this project can be found [here](http://hdl.handle.net/10609/107488). *(Only spanish version available)*
## License
This project can be distributed under the Creative Commons 3.0 (by-nc-nd)
