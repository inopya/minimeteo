
/*mmmmmmmmmmmmmmmmmmmmmm */
/*    MAPA DE PINES      */
/*mmmmmmmmmmmmmmmmmmmmmm */


/*
  ----------------------------------------
           << PINOUT  MAP >>
  ----------------------------------------
  WeMos     ESP-8266    Functiones
  
  D0        GPIO16      DeepSleep out  
  D1        GPIO5       SCL
  D2        GPIO4       SDA
  D3        GPIO0       10k Pull-up
  D4        GPIO2       10k Pull-up, blue LED
  D5        GPIO14      SPI SCK
  D6        GPIO12      SPI MISO
  D7        GPIO13      SPI MOSI
  D8        GPIO15      SPI SSEL, 10k Pull-down
  Rx        GPIO03      Serial Rx
  Tx        GPIO01      Serial Tx
  A0        A0/ADC0     Entrada Analogica

*/
 


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        SECCION DE DEFINICION DE PINES
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#define PIN_LED_OnBoard             D4    // Led on Board Wemos
#define PIN_pulsador_config         D5    // para poner un pulsador pullup para configuracion (sin uso)

