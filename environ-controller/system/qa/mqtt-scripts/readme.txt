This folder has MQQT test client runnable command files. They are used to test EC as stand alone device
without having any mobile APP separately using mosquitto server .

ALL these files can be run from any Linux machine having MQTT Test client . 


To Install MQTT clients, pl run the following 

# sudo apt-get install mosquitto-clients

pl refer for any other details  https://www.vultr.com/docs/how-to-install-mosquitto-mqtt-broker-server-on-ubuntu-16-04  


After installation  of mqtt client , Just run the individual files of interest . For eg , to send EC to start grow cycle 

# ./grow-req


To get the status information or replies from EC  

# mosquitto_sub -t "test" -h "localveggy.com" -p 1883  -u "pulsespectrum" -P "welcome"

