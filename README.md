# minimeteo
Mini estacion meteorologica con esp8266 y cliente telegram



      =====  NOTAS DE LA VERSION  ===== 

    Versión 0.1       Fecha: 14/05/2021
      - control basico de sensoses
	    - envio informacion a telegram

     Versión 0.2       Fecha: 16/05/2021
      - Control del WDT para evitar los "tipicos" reinicios causados por el wifi 
        en las placas de desarrollo basadas en ESP8266   que solo disponen de un nucleo para todas las tareas
       - se establecen comandos para funciones exclusivas del administrador 
       - registro de minimas y maximas diarias (sin horario)
       - Posibilidad de mensajes Serial para Debug

    Versión 0.3      Fecha: 08/07/2023 
	Revisitando el proyecto...
      	- Control del momento horario en que se producen las minimas y maximas diarias.
	- Control de minimas y maximas para una serie historica (el administrador puede restablecer el periodo)
	- Conserva en eeprom los valores y momentos de minimas y maximas
      	- Funciones para ajuste del horario invierno/verano
      	- Descarta sensores no conectados en los mensajes de informacion.
	- Avisa si todos los sensores fallan y no puede tomar datos de ningun tipo
	- Habilitadas actualizaciones por OTA 
	- Posibilidad de reenviar los mensajes de los clientes al administrador
	- Monitorizacion del estado de la bateria
	- Posibilidad de entrar en modo LightSleep (DeepSleep con control de Timer) si la bateria es baja 
	- Separacion de los parametros mas utilizados en el fichero "config.h" para facilitar modificaciones y personalizacion
	  
	  
	//TO-DO ??
	- ampliar sensores: UV, pluviometro, anemometro... 
	- configurar el tiempo de los envios periodicos mediante mensaje telegram
	- configurar mediante mensajes telegram los niveles de bateria baja para sleep mode
	- configurar mediante mensaje telegram los periodos sleep
	...
	  



## Lista de materiales

- 1x Wemos D1 mini R2 u otra placa de desarrollo con ESp8266 (o ESP32)
- 1x celula solar 5v/6v y 1W o mas. (de su potencia dependerá la rapidez de carga de nuestra bateria de reserva)
- 1x diodo 1N5819 (o similar), para evitar el consumo por parte de la celula solar en las horas de oscuridad
- 2x resistencias 100 MΩ
- 1x bateria lipo 3.7v (teléfono móvil, 18650,...)
- 1x Modulo de carga para baterias Lipo tipo TP4056 (o similar)
- 1x Sensor barométrico BMP/E280 (o similar)
- 1x Sensor humedad HTU21D /HTU31D (o similar)
- 1x protoboard o placa perforada
- cables 
- pines, soldador, estaño (en esta version optamos por montaje en placa perforada) 


## Esquema de montaje

![](./imagenes/sche-wemos-miniMETEO.png)



##  Imagen del montaje real

![](./imagenes/hard-minimeteo-componentes.jpg)



## Escudo sobre placa perforada
Para esta versión se ha obtado por un montaje "un poco mas definitivo" y se ha creado un escudo sobre placa perforada.

![](./imagenes/hard-minimteo-escudo-up.jpg)

![](./imagenes/hard-minimteo-wemos-en-escudo.jpg)



## Imagenes de la interfaz telegram

Algunas imagenes del aspecto de los menús de interacción desde el bot de telegram

Menu accesible por los usuarios públicos. 
Contiene solo un reducido conjunto de comandos:

![](./imagenes/soft-menu-usuario.png)

Muestra de respuestas de minimeteo ante la petición de un comando prohibido para usuarios y la petición del registro histórico de temperaturas mínimas y máximas (comando público)

![](./imagenes/sof-registro-historico.png)

Menú de comandos del administrador. Set de comandos Básicos + set de comandos Extra

![](./imagenes/sof-menu-admin.png)


