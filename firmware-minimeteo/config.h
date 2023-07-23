/*
 #       _\|/_   A ver..., ¿que tenemos por aqui?
 #       (O-O)
 # ---oOO-(_)-OOo---------------------------------


 # *   ARCHIVO DE CONFIGURACION PARA ESTACION mini METEO   * #


 */


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        BANDERAS DE CONFIGURACION RAPIDA
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#define FLAG_ADC_READ_A0					// comentar si no tenemos lectura de la bateria en A0 y se usará ESP.getVcc()
//#define FLAG_SENSOR_TYPE_BME  			// descomentar para usar el BME280 (por defecto BMP280)

bool FLAG_aviso_reinicio = true;			// TRUE: recibir avisos en cada reinicio del sistema
bool FLAG_enable_battery_control = false; 	// TRUE: habilitar deepsleep por bateria baja
bool FLAG_enable_periodic_task = false;		// TRUE: para realizar una tarea periodica	
bool FLAG_aviso_conexion_usuarios = false;  // TRUE: para recibir (nombre y comando) de los usuarios que se conectan
bool FLAG_led_conexion_wifi = true;  		// TRUE: mantiene encendido led onboard mientras se establece la conexion wifi
bool FLAG_led_full_setup = false;  			// TRUE: mantiene encendido led onboard d el setup
bool FLAG_eeprom_life_extend = true;		// se recomienda TRUE: solo actualiza la eeprom cada 'MINUTOS_INTERVALO_EEPROM_UPDATE'

//#define FLAG_ENABLE_PERIODIC_DEEPSLEEP    // comentar para deshabilitar deepsleep periodico
//#define FLAG_SERIAL_DEBUG   				// descomentar para mostrar mensajes de DEBUG
#define FLAG_OTA_ENABLE						// comentar si se desea desactivar la funcionalidad OTA




/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//       PUERTO SERIAL y BATERIA, ....
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#define SERIAL_BAUD_RATE    115200

#define FACTOR_AJUSTE_ADC_INT   1.711		// factor de correcion para lecturas con adc interno 
#define FACTOR_AJUSTE_A0  		1.083  		// factor de correcion para lecturas con A0 
#define SLEEP_BATTERY_LEVEL 	 3400		// 3.4v
#define WAKEUP_BATTERY_LEVEL	 3700   	// 3.7v

#define ALTITUD 287.0      					// Altitud de la estacion (en metros), para el calculo de la presion relativa



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        ESTACION ID
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#define _HARDWARE_        	"minimeteo"
#define _ID_STATION_   		"Pruebas 001"    		// ID de estacion, codigo para distinguirlas (ha de ser una cadena)
#define _FIRMWARE_    		"INOPYA: mM0.3-20230708"


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        PORTAL WIFI 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#define SSID_PORTAL		"INOPYA_IoT-minimeteo"		// nombre de red apra el portal web de configuracion
#define PASS_PORTAL		"minimeteo" 				// contraseña del portal web de configuracion




/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        OTA 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#define OTA_DEVICE_NAME		"MiniMeteo-pruebas"		// comentar para que nombre de dispositivo sea el generico: "ESP8266-xxxx
#define OTA_UPDATE_PASS		"pruebas"   	    	// comentar para permitir actualizaciones sin clave (no se recomienda)



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        TELEGRAM
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#define BOT_TOKEN "xxxxxxx:zzzzzzzzzzzzzzzzzzzzzzzz"    // telegram bot token
#define ADMIN_USER   "xxxxxxxxx"  						// telegram user ID



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        TIEMPOS Intervalos configurables (  ¡ OJO a las unidaes de tiempo !  )
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#define SEGUNDOS_INTERVALO_ACCESO_SERVIDOR     	     5		// en sedundos,   >> entre busquedas de mensajes de telegram
#define SEGUNDOS_INTERVALO_LECTURA_SENSORES   	    10		// en sedundos,   >> entre refrescos automaticos de variables clima
#define SEGUNDOS_INTERVALO_CONTROL_HORARIO	   	    60	 	// en sedundos,   >> entre refrescos del reloj interno

#define MINUTOS_INTERVALO_EEPROM_UPDATE	   	   		10	 	// en minutos,   >> minimo entre operaciones commit() de la eeprom. Alargar su vida util
#define MINUTOS_INTERVALO_CONTROL_BATERIA	        10		// en minutos,    >> entre comprobaciones de bateria
#define MINUTOS_INTERVALO_TAREA_PERIODICA       	15 		// en minutos, 	  >> entre envios de señal de 'vivo'
#define MINUTOS_DEEP_SLEEP_LOW_BATTERY	   		    60 	 	// en minutos,    >> tiempo que suspendemos si bateria baja
#define MINUTOS_DEEP_SLEEP_PERIODIC	   		         5 	 	// en minutos,    >> tiempo que suspendemos periodicamente
															// Util para usar como datalogger, tomando una muestra cada cierto tiempo y susendiendo para ahorrar bateria


/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        TIEMPOS CONSTANTES (no modificar)
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#define 	uSECONDS	   	           	  1e6	  //  microsegundos en 1 segundo 
#define 	miliSECONDS	   	           	  1e3	  //  milisegundos  en 1 segundo 
#define 	MINUTO	   	                   60	  //  segundos      en 1 minuto 




