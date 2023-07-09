# arduino-codes-memoria

De forma de correr los códigos disponibles, estos deben abrirse utilizando la aplicación Arduino IDE. Una vez allí presionando "Upload" archivos comenzaran a correr.

# portenta-códigos

(1) De forma de correr los códigos para Portenta correctamente, primero deben quedar instalados los firmware para Portenta disponibles en https://github.com/hpssjellis/ArduinoCore-stm32l0/tree/vision_shield.

(2) Una vez instalados se corre en primer lugar el código bridge (con el bootloader de la placa activado).

(3) Posteriormente se corre el código grumpy-old-pizza, seleccionando como placa la que fue instalada en el primer paso.

(4) Utilizado nuevamente la placa Portenta normal, se corre en última instancia el código para el serial LoRa.
