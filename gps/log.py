import serial
from datetime import datetime
  
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
f = open("gpslog.txt", 'w')

while True:
    reading = ser.readline()
    date = datetime.now().strftime("%m/%d/%Y, %H:%M:%S")

    f.write(date + ") " + str(reading) + "\n")
    print(reading)





