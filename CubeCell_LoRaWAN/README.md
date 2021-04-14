# Heltec CubeCell HTCC-AB01

Heltec CubeCell besitzt die komplette Infrastruktur zum Aufbau eines solar-betriebenen LoRaWAN-Knotens on-board.  
Sensor-Erweiterungen können über den I2C-Bus erfolgen. Es sind beim CubeCell keine PullUp-Widerstände vorhanden.   
Diese sind extern zu berücksichtigen, was ohnehin meist bei Einsatz eines Breakout Boards der Fall ist.   
Die PullUp-Widerstände sollten nicht mit VDD sondern mit Vext verbunden werden.   
Vext kann aktiviert  werden bevor der I2C-Bus aktiv ist und kann nach dem I2C-Bus-Transfer wieder abgeschaltet werden.  

Nach einem Hinweis aus dem Heltec Forum 
- After uploading the sketch and configure unplug the usb cable and press reset. If not the usb serial chip will be drawing power.
- And dont make serial prints</strong>.</li></ul>
habe ich am 20.07.2020 einen neuen Laufzeittest mit einem 1000 mAh LiPo-Akku gestartet.  
Hier sind die Ergebnisse zu finden:
https://ckarduino.wordpress.com/heltec-cubecell/ läuft ein 
https://thingspeak.com/channels/1256979
