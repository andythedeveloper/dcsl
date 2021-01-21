
# README 

## Official Software
- u-center

# Receiving GPS data on RPi

## Protocol
- NMEA

## Checking USB devices
- $ `lsusb`
  
Bus 001 Device 008: ID 1546:01a8 U-Blox AG [u-blox 8]

- $ `usb-devices`
  
> T:  Bus=01 Lev=02 Prnt=02 Port=01 Cnt=02 Dev#=  8 Spd=12  MxCh= 0 \
> D:  Ver= 1.10 Cls=02(commc) Sub=00 Prot=00 MxPS=64 #Cfgs=  1 \
> P:  Vendor=1546 ProdID=01a8 Rev=03.01 \
> S:  Manufacturer=u-blox AG - www.u-blox.com \
> S:  Product=u-blox GNSS receiver \
> C:  #Ifs= 2 Cfg#= 1 Atr=c0 MxPwr=100mA \
> I:  If#=0x0 Alt= 0 #EPs= 1 Cls=02(commc) Sub=02 Prot=01 Driver=cdc_acm \
> I:  If#=0x1 Alt= 0 #EPs= 2 Cls=0a(data ) Sub=00 Prot=ff Driver=cdc_acm \

- $ `ls -l /dev/tty*`

> crw-rw---- 1 root dialout 166,  0 Jan 21 13:51 /dev/ttyACM0

## Shows data on terminal
<code>
$ screen /dev/ttyACM0 \
$ cat /dev/ttyACM0 \
$ stty -F /dev/ttyACM0  9600  raw -clocal -echo icrnl \
</code>

## UBX library
- [pyubx2](https://pypi.org/project/pyubx2/)
- [SparkFun](https://github.com/sparkfun/SparkFun_Ublox_Arduino_Library)
- RTKLIB
- [Arduino M8](https://github.com/creatica-soft/Gnss)
- [cc.ublox.commsdsl](https://github.com/commschamp/cc.ublox.commsdsl)
- [ublox](https://github.com/GAVLab/ublox)
- [for SparkFun](https://github.com/sparkfun/Qwiic_Ublox_Gps_Py)

## References
- [Serial Communication on RPi](https://www.electronicwings.com/raspberry-pi/raspberry-pi-uart-communication-using-python-and-c)
- [Serial Communication Library](https://github.com/wjwwood/serial)
- [Serial Communication (elinux)](https://elinux.org/RPi_Serial_Connection)
- [UART Communication (official)](https://www.raspberrypi.org/documentation/configuration/uart.md)

- sparkfun-ublox-gps-1.1.3 installed
