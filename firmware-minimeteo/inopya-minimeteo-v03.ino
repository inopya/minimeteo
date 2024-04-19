/*
 #       _\|/_   A ver..., ¿que tenemos por aqui?
 #       (O-O)
 # ---oOO-(_)-OOo---------------------------------

 ####################################################
 # ************************************************ #
 # *   MAYORDOMO VIRTUAL CON ESP8266 Y TELEGRAM   * #
 # *         Aplicado a estacion miniMeteo        * #
 # *         Autor: Eulogio Lopez Cayuela         * #
 # *          https://github.com/inopya/		  * #
 # *      Versión 0.3      Fecha: 08/07/2023      * #
 # *                                              * #
 # ************************************************ #
 ####################################################

 */




/*
      =====  NOTAS DE LA VERSION  ===== 

    Versión 0.1       Fecha: 14/05/2021
      - control basico de sensoses
	  - envio informacion a telegram

     Versión 0.2       Fecha: 16/05/2021
	  - Control del WDT para evitar los "tipicos" reinicios causados por el wifi en las placas de desarrollo basadas en ESP8266 que solo disponen de un nucleo para todas las tareas
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
	  
 */


/*

	=====  FUNCIONES DE LOS PINES  ===== 

  WeMos     ESP-8266    Funciones
  
  D0        GPIO16      DeepSleep out (for timer wakeup) 
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



	=====  DISTRIBUCION PINES WEMOS  ===== 
	
             ___________
            /           \
           /             \
       RST|	   _______	  |Tx
        A0|	  |       |	  |Rx
        D0|	  |  ESP  |	  |D1
        D5|	  | 8266  |	  |D2
        D6|	  |_______|	  |D3
        D7|	  			  |D4
        D8|	  			  |GND
       3v3|	  			  |5V
          |______###______|
                 USB 
    




 */
 



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        IMPORTACION DE LIBRERIAS
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>

#include "enumeradores.h"
#include "pinout.h"
#include "definiciones.h"
#include "constantes.h"

#include "config.h"

#include "variables.h"


#include <ESP8266WiFi.h>
#include <WiFiManager.h>                  //https://github.com/tzapu/WiFiManager

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HTU21DF.h> 


  #ifdef FLAG_SENSOR_TYPE_BME  
    #include <Adafruit_BME280.h>
  #else
    #include <Adafruit_BMP280.h>  
  #endif



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        PROTOTIPADO DE FUNCIONES
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

bool loadConfig( void );
bool save_flag_config( void );
bool check_eeprom_rw( void );

bool reconectar_wifi( uint8_t _intentos=2 ); 
void eliminar_mensajes_viejos( void );
void procesarTelegramas( int );
void send_msg_error_sensores( String );
void set_default_config( void );
void send_estado_banderas( String );
void enviar_lista_comandos_general( String ); 
void enviar_lista_comandos_extra( String );
void enviar_cabecera( String , uint8_t _type=SOLO_VERSION );
void dar_hora( String );
void ajustarHorarioInviernoVerano( void );
void update_eventos_reloj( void );
void update_time( void );
void enviar_informacion_clima( String );
void enviar_informacion_debug_admin( String );
void enviar_min_max_diarios(String );
void enviar_min_max_historicos( String );
void reset_min_max_diarios( void );
void reset_min_max_historicos( void );
void update_sensores( void );
void send_estado_sensores( String user );
uint16_t control_bateria_con_adc_int( uint8_t _modo=0 );
void enviar_estado_bateria_adc_int( String );
int16_t control_bateria_con_A0( uint8_t _modo=0 ); 
void enviar_estado_bateria_A0( String );
void setup_OTA( void );   //permite habilitar la funcionalidad de actualizaciones mediante OTA

void hw_wdt_disable( void );      // Hardware WDT OFF
void hw_wdt_enable( void );       // Hardware WDT ON
void hw_wdt_timeout( uint32_t );  // Establecer TimeOut Hardware WDT



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        CREACION DE OBJETOS
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

  #ifdef FLAG_SENSOR_TYPE_BME  
    Adafruit_BME280 sensor_bmx280;	  	// para bme
  #else
    Adafruit_BMP280 sensor_bmx280;		// para bmp  
  #endif

Adafruit_HTU21DF sensorHTU = Adafruit_HTU21DF();

/* gestor de conexiones y AP */
WiFiManager wifiManager;

#ifndef FLAG_ADC_READ_A0
	ADC_MODE(ADC_VCC);
#endif	



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
   ****************************************************************************************************** 
                                    FUNCION DE CONFIGURACION
   ****************************************************************************************************** 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

void setup()
{
	pinMode(PIN_LED_OnBoard, OUTPUT);
	digitalWrite(PIN_LED_OnBoard, Onboard_LED_OFF);
	
	EEPROM.begin( EEPROM_SIZE );  //reserva xx bytes para eeprom (ojo tambien los nececesita de RAM para manejarlos)
	delay(100);
	loadConfig(); 


	if( FLAG_led_full_setup ){ digitalWrite(PIN_LED_OnBoard, Onboard_LED_ON);}	
	
    if( FLAG_enable_battery_control ){
		//este control se hace antes de iniciar las comunicaciones, así, si no hay bateria suficiente, 
		//evitamos un gasto extra por parte de la wifi y pasamos a deepesleep directamente.
		delay(500);
		int16_t estado_bateria;
		#ifdef FLAG_ADC_READ_A0
			estado_bateria = control_bateria_con_A0(0);
		#else
			estado_bateria = control_bateria_con_adc_int(0);
		#endif		

		if( estado_bateria < WAKEUP_BATTERY_LEVEL ){
			ESP.deepSleep( MINUTOS_DEEP_SLEEP_LOW_BATTERY * MINUTO * uSECONDS );
		} 
    }




	hw_wdt_enable();       					// Hardware WDT ON
	hw_wdt_timeout( TIMEOUT_26_8_SEC );		// establecer timeout para WDT hardware a su maximo tiempo
	ESP.wdtFeed();							// alimentar WDT/wdt

	ESP.wdtDisable();       				// Software WDT OFF


	/* Iniciar Sensores */
	if ( sensor_bmx280.begin(0x76) ) { FLAG_bmx_enable = true; } 	
	if ( sensorHTU.begin() ) { FLAG_htu_enable = true; } 

	
    #ifdef FLAG_SERIAL_DEBUG
		Serial.begin( SERIAL_BAUD_RATE );

		while(!Serial){
			delay(10);
		}
  
		Serial.println(F("\nPuerto serie preparado\n"));

		if(FLAG_bmx_enable) { Serial.println(F("DEBUG: Sensor BMx280 OK")); }
		else{ Serial.println(F("DEBUG: Error de sensor BMx280")); } 
		if(FLAG_htu_enable) { Serial.println(F("DEBUG: Sensor HTU21 OK")); } 
		else{ Serial.println(F("DEBUG: Error de sensor HTU21")); } 			
	#endif	


	secured_client.setTrustAnchors(&cert); // certificado necesario apra la API de telegram (api.telegram.org)

	/* Establecer el modo WiFi */
	WiFi.mode(WIFI_STA);


	if( FLAG_led_conexion_wifi ){ digitalWrite(PIN_LED_OnBoard, Onboard_LED_ON);} //encendemos led para indicar que estamos estableciendo conexion

	/* Intentar conectar y si no... montar un AP para configuracion */	
	wifiManager.setAPStaticIPConfig(IPAddress(192,168,5,1), IPAddress(192,168,5,1), IPAddress(255,255,255,0));    

	/* estabecer timeout para los intentos de conexion wifi */
	wifiManager.setConnectTimeout(TIMEOUT_CONECT_TO_WIFI); 
	
	/* estabecer timeout para el portal de configuracion */
	wifiManager.setConfigPortalTimeout(TIMEOUT_AP_PORTAL);
	
	bool wifi_estatus = wifiManager.autoConnect(SSID_PORTAL, PASS_PORTAL); 	// portal de configuracion si no podemos conectar

	ESP.wdtFeed(); 

	if( !wifi_estatus ) { ESP.reset(); } // si no conecta, reinicio
		 

	if( !FLAG_led_full_setup ){ digitalWrite(PIN_LED_OnBoard, Onboard_LED_OFF);}

    #ifdef FLAG_SERIAL_DEBUG	  
		Serial.println(F("\nWiFi conectada. IP address: "));
		Serial.println(WiFi.localIP()); 
		Serial.println(F("\nActualizando fecha/hora: "));
    #endif	
	
	configTime("GMT-1", "pool.ntp.org", "time.nist.gov"); 
	time_t now = time(nullptr);
	
	while (now < 24 * 3600) {
	    #ifdef FLAG_SERIAL_DEBUG	 
			Serial.print(".");
		#endif	
		delay(1000);
		now = time(nullptr);
	}

	update_time();  //necesario en este punto para actualizar variables de dia y mes
					//aunque puede que el tipo de horario sea incorrecto


	eliminar_mensajes_viejos();		// eliminar mensajes que llegaron mientras la estacion estaba desconectada
									// para evitar responder a cosas que ya no son necesarias
									

	ajustarHorarioInviernoVerano();
	
	update_eventos_reloj();	
	
	ESP.wdtFeed(); 
	
	if( FLAG_aviso_reinicio ){
		//enviar_cabecera(ADMIN_USER);
		//bot.sendMessage( ADMIN_USER, "Escribe  /start  para comenzar" );
		bot.sendMessage( ADMIN_USER, "Me he reiniciado, que lo sepas!" );
		
	}

	ESP.wdtFeed();
	
    #ifdef FLAG_SERIAL_DEBUG
		Serial.println("");
		Serial.println(fecha_hora);
		Serial.println("\n");	
    #endif
	

	send_estado_sensores(  ADMIN_USER );
	
	ESP.wdtFeed();

	#ifdef FLAG_OTA_ENABLE
		setup_OTA();  //habilita las actualizaciones mediante OTA
	#endif
	
	
	digitalWrite(PIN_LED_OnBoard, Onboard_LED_OFF);	
}



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
   ****************************************************************************************************** 
                                  BUCLE PRINCIPAL DEL PROGRAMA
   ****************************************************************************************************** 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

void loop()
{

	ESP.wdtFeed();  //alimentar el WDT software/hardware. (habilitado solo el WDT hardware)
	
	#ifdef FLAG_OTA_ENABLE
		ArduinoOTA.handle();  //habilita las actualizaciones mediante OTA
	#endif
	
	
	uint32_t momento_actual = millis();
	

	/* ========================================================================= */
	/* ================ COMPROBAR ESTADO DE LA CONEXION WIFI =================== */

	if ( momento_actual - ultimo_chequeo_wifi > (INTERVALO_CHECK_WIFI * miliSECONDS) ) {
		reconectar_wifi();
		ultimo_chequeo_wifi = millis();
	}
	
	
	/* ========================================================================= */
	/* ================= ACCESO TELEGRAM PARA RECOGER MENSAJES ================= */
	
	// >>OPCION A)  Chequeamos y reconectamos si es necesario antes de acceder a telegram
	//if( reconectar_wifi() ) {  
	
	// >>OPCION B)  Solo "si la wifi esta conectada" accedemos a telegram
	if ( WiFi.status()==WL_CONNECTED ){	
		if ( momento_actual - ultimo_acceso_telegram > (SEGUNDOS_INTERVALO_ACCESO_SERVIDOR * miliSECONDS) ) {
			int num_mensajes_pendientes = bot.getUpdates(bot.last_message_received + 1);

			while (num_mensajes_pendientes) {
				procesarTelegramas(num_mensajes_pendientes);
				num_mensajes_pendientes = bot.getUpdates(bot.last_message_received + 1);
			}
			ultimo_acceso_telegram = momento_actual; 
		}
	}

	/* ========================================================================= */
	/* ======================= LECTURA DE LOS SENSORES ========================= */
	
	if ( momento_actual - ultima_lectura_sensores > (SEGUNDOS_INTERVALO_LECTURA_SENSORES * miliSECONDS) ) {
		if(FLAG_bmx_enable || FLAG_htu_enable) { update_sensores(); }
		ultima_lectura_sensores = momento_actual; 
	} 


	/* ========================================================================= */
	/* ======================= CONTROL DE FECHA Y HORA ========================= */
	
	if ( momento_actual - ultimo_control_horario > (SEGUNDOS_INTERVALO_CONTROL_HORARIO * miliSECONDS) ) {
		update_eventos_reloj();
	} 


	/* ========================================================================= */
	/* ==================== CONTROL DEL ESTADO DE LA BATERIA =================== */
	
    if( FLAG_enable_battery_control ){ 
		if ( momento_actual - ultimo_control_bateria > (MINUTOS_INTERVALO_CONTROL_BATERIA * MINUTO * miliSECONDS) ) {
			ultimo_control_bateria = momento_actual; 
			int16_t estado_bateria;
			#ifdef FLAG_ADC_READ_A0
				estado_bateria = control_bateria_con_A0(0);
			#else
				estado_bateria = control_bateria_con_adc_int(0);
			#endif			
			
			
			if(estado_bateria < SLEEP_BATTERY_LEVEL ){ 
				if( FLAG_eeprom_life_extend && FLAG_update_eeprom_data ) { EEPROM.commit(); }  //¿pendiente actualizar la eeprom?
				
				bot.sendMessage(ADMIN_USER, "Bateria baja. Suspension durante " + String(MINUTOS_DEEP_SLEEP_LOW_BATTERY) + " minutos"); 
				ESP.deepSleep( MINUTOS_DEEP_SLEEP_LOW_BATTERY * MINUTO * uSECONDS );
			} 
		} 
	}


	/* ========================================================================= */
	/* ===================== CONTROL DE TAREAS PERIODICAS ====================== */
		
    if( FLAG_enable_periodic_task ) {
		if(momento_actual-ultima_tarea_periodica > (MINUTOS_INTERVALO_TAREA_PERIODICA * MINUTO * miliSECONDS) ) {
			ultima_tarea_periodica = momento_actual;
			enviar_informacion_clima( ADMIN_USER );
			//bot.sendMessage(ADMIN_USER, fecha_hora);  //mensaje con fecha hora paara indicar que seguimos online
				#ifdef FLAG_ADC_READ_A0
					enviar_estado_bateria_A0(ADMIN_USER); 
				#else
					enviar_estado_bateria_adc_int( ADMIN_USER );
				#endif			
		}
	}
      

	/* ========================================================================= */
	/* ================ CONTROL DE GRABACION DE DATOS EN EEPROM ================ */
		
    if( FLAG_eeprom_life_extend && FLAG_update_eeprom_data ) {
		if(momento_actual-ultima_grabacion_eeprom > (MINUTOS_INTERVALO_EEPROM_UPDATE * MINUTO * miliSECONDS) ) {
			ultima_grabacion_eeprom = momento_actual;
			EEPROM.commit();
			FLAG_update_eeprom_data = false;
		}
	}
	
	
	/* ========================================================================= */
	/* =============== CONTROL DE NOTIFICACION + SLEEP PERIODICOS ============== */
	
    #ifdef FLAG_ENABLE_PERIODIC_DEEPSLEEP
		enviar_informacion_clima( ADMIN_USER );
		
		#ifdef FLAG_ADC_READ_A0
			enviar_estado_bateria_A0(ADMIN_USER); 
		#else
			enviar_estado_bateria_adc_int( ADMIN_USER );
		#endif	
		
		//bot.sendMessage(ADMIN_USER, "A dormir " + String(MINUTOS_DEEP_SLEEP_PERIODIC) + " minutos");
		
		if( FLAG_eeprom_life_extend && FLAG_update_eeprom_data ) { EEPROM.commit(); }  //¿pendinte actualizar la eeprom?
		
		ESP.deepSleep( MINUTOS_DEEP_SLEEP_PERIODIC * MINUTO * uSECONDS );
	#endif	

    
}




/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx 
        BLOQUE DE FUNCIONES: LECTURAS DE SENSORES, COMUNICACION SERIE, INTERRUPCIONES...
   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx 
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    EEPROM
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//========================================================
// LEER DE EEPROM DATOS DE CUALQUIER TIPO
//========================================================
template <typename T> unsigned int eeprom_load (int _memory_pos, T& _value)
{
	byte* p = (byte*)&_value;
	uint16_t i;
	for (i = 0; i < sizeof(_value); i++)
		*p++ = EEPROM.read(_memory_pos++);
	return i;
}


//========================================================
// GRABAR EN EEPROM DATOS DE CUALQUIER TIPO
//========================================================

template <typename T> unsigned int eeprom_save(int _memory_pos, const T& _value)
{
	const byte* p = (const byte*)&_value;
	uint16_t i;
	for (i = 0; i < sizeof(_value); i++)
		EEPROM.write(_memory_pos++, *p++); 
	return i;
}


//========================================================
// LEER CONFIGURACION
//========================================================

bool loadConfig( void )
{
	//comprobar si hay datos en eepom o es un primer inicio del sistema
	uint8_t valor_referencia;
	eeprom_load(POS_MEM_CONFIG_OK, valor_referencia);
	if(valor_referencia != EEPROM_REF_DATA){
		set_default_config();
	}
	
	
	//cargar datos de min/max desde eeprom
	eeprom_load(POS_MEM_T_MIN_DIARIO, temperatura_min_dia);	
	eeprom_load(POS_MEM_T_MAX_DIARIO, temperatura_max_dia);
	
	eeprom_load(POS_MEM_T_MIN_HISTORICO, temperatura_min_historico);
	eeprom_load(POS_MEM_T_MAX_HISTORICO, temperatura_max_historico);
	
	char char_array10[10]="";  //array caracteres
	eeprom_load(POS_MEM_HORA_MIN_DIARIO, char_array10);
	str_hora_Tmin_dia = String(char_array10); 
	
	eeprom_load(POS_MEM_HORA_MAX_DIARIO, char_array10);
	str_hora_Tmax_dia = String(char_array10); 
	
	char char_array20[20]="";
	eeprom_load(POS_MEM_FECHA_START_HISTORICO, char_array20);
	str_fecha_inicio_historico = String(char_array20); 
	
	char char_array30[30]="";  //array caracteres
	eeprom_load(POS_MEM_FECHA_MIN_HISTORICO, char_array30);
	str_fecha_Tmin_historico = String(char_array30); 
	
	eeprom_load(POS_MEM_FECHA_MAX_HISTORICO, char_array30);
	str_fecha_Tmax_historico = String(char_array30); 

	uint8_t lectura_memoria;
	eeprom_load(POS_MEM_AVISO_REINICIO, lectura_memoria);
	FLAG_aviso_reinicio = (lectura_memoria==0)?false:true;

	eeprom_load(POS_MEM_PERIDIC_TASK, lectura_memoria);
	FLAG_enable_periodic_task = (lectura_memoria==0)?false:true;
	
	eeprom_load(POS_MEM_BATTERY_CONTROL, lectura_memoria);	
	FLAG_enable_battery_control = (lectura_memoria==0)?false:true; 
	
	eeprom_load(POS_MEM_AVISO_USUARIOS, lectura_memoria);
	FLAG_aviso_conexion_usuarios = (lectura_memoria==0)?false:true; 

	eeprom_load(POS_MEM_LED_WIFI, lectura_memoria);
	FLAG_led_conexion_wifi = (lectura_memoria==0)?false:true;
	
	eeprom_load(POS_MEM_LED_SETUP, lectura_memoria);
	FLAG_led_full_setup = (lectura_memoria==0)?false:true;
	
	eeprom_load(POS_MEM_DIA_EN_CURSO, dia_encurso);
	

	#ifdef FLAG_SERIAL_DEBUG
		Serial.println(F("VALAORES ALMACENADOS EN EEPROM:\n"));
		Serial.print(F("min_dia: "));Serial.print(temperatura_min_dia);
		Serial.print(F(",  hora: "));Serial.println(str_hora_Tmin_dia);
		Serial.print(F("max_dia "));Serial.print(temperatura_max_dia);
		Serial.print(F(",  hora: "));Serial.println(str_hora_Tmax_dia);
		Serial.print(F("\nFecha inicio: "));Serial.println(str_fecha_inicio_historico);
		Serial.print(F("min_historico: "));Serial.print(temperatura_min_historico);
		Serial.print(F(",  fecha: "));Serial.println(str_fecha_Tmin_historico);
		Serial.print(F("max_historico: "));Serial.print(temperatura_max_historico);
		Serial.print(F(",  fecha "));Serial.println(str_fecha_Tmax_historico);
		Serial.print(F("\nDia en curso: "));Serial.println(dia_encurso);
	#endif

	return true; //a falta de implementar un control de errores
	
}


//========================================================
// SALVAR CONFIGURACION
//========================================================

bool save_flag_config( void )
{	
	uint8_t escritura_memoria;
	
	escritura_memoria = (FLAG_aviso_reinicio)?1:0; 
	EEPROM.write(POS_MEM_AVISO_REINICIO, escritura_memoria); 
	
	escritura_memoria = (FLAG_enable_periodic_task)?1:0;
	EEPROM.write(POS_MEM_PERIDIC_TASK, escritura_memoria);
	
	escritura_memoria = (FLAG_enable_battery_control)?1:0;  
	EEPROM.write(POS_MEM_BATTERY_CONTROL, escritura_memoria);	
	
	escritura_memoria = (FLAG_aviso_conexion_usuarios)?1:0;
	EEPROM.write(POS_MEM_AVISO_USUARIOS, escritura_memoria);

	escritura_memoria = (FLAG_led_conexion_wifi)?1:0;
	EEPROM.write(POS_MEM_LED_WIFI, escritura_memoria);

	escritura_memoria = (FLAG_led_full_setup)?1:0;
	EEPROM.write(POS_MEM_LED_SETUP, escritura_memoria);
	
	EEPROM.commit(); 
	
	return true;  //a falta de implementar un control de errores
}


//========================================================
// COMPROBAR OPERACIONES SOBRE LA EEPROM (para pruebas)
//========================================================

bool check_eeprom_rw( uint16_t _memory_pos )
{
	bool FLAG_eeprom_status = true;
	
	uint16_t variable_1;
	uint16_t variable_2;
	
	eeprom_load(_memory_pos, variable_1);
	delay(10);
	eeprom_load(_memory_pos, variable_2);
	
	if( variable_1!=variable_2 ){ FLAG_eeprom_status = false; }

	return FLAG_eeprom_status;
}




/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    WIFI
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//========================================================
// REVISA CONEXION WIFI y RECONECTA SI ES NECESARIO
//========================================================

bool reconectar_wifi(uint8_t _intentos)
{
	bool FLAG_estado_wifi = false;
	uint8_t contador_intentos=0;

	ESP.wdtDisable(); //a veces FreeRTOS reactiva el wdt software :(
	ESP.wdtFeed();

	while( contador_intentos<_intentos && !FLAG_estado_wifi ){ 
		contador_intentos++;
		if ( WiFi.status()==WL_CONNECTED ){
			#ifdef FLAG_SERIAL_DEBUG
				Serial.println(F("Conexion WiFi OK"));
			#endif
			FLAG_estado_wifi = true;
			break;
		}
		else{ 
			#ifdef FLAG_SERIAL_DEBUG
				Serial.println(F("ERROR de Conexion WiFi\n\tReconectando..."));
			#endif	
			wifiManager.autoConnect(SSID_PORTAL, PASS_PORTAL);
			delay(250);			
		}	
		ESP.wdtFeed();
	}
	
	return FLAG_estado_wifi;	
}



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    TELEGRAM
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//========================================================
// ELIMINAR TELEGRAMAS VIEJOS AL REINICIAR
//========================================================

void eliminar_mensajes_viejos()
{
	/* funcion apra eliminar mensajes que han podido llegar mientras estamos desconectados */

	int num_mensajes_pendientes = bot.getUpdates(bot.last_message_received + 1);

	#ifdef FLAG_SERIAL_DEBUG
		while( num_mensajes_pendientes ){
			for( int i = 0; i < num_mensajes_pendientes; i++ ){
				String from_name = bot.messages[i].from_name;  
				String chat_id = bot.messages[i].chat_id; 
				String text = bot.messages[i].text; 
				Serial.print(F("ID: ")); 
				Serial.print( chat_id );
				Serial.print(F(" >>> NOMBRE: ")); 
				Serial.print( from_name );    
				Serial.print(F(" >>> COMANDO: ")); 
				Serial.println( text );
			}
			num_mensajes_pendientes = bot.getUpdates(bot.last_message_received + 1);
		}
	#else
		while(num_mensajes_pendientes ){ num_mensajes_pendientes = bot.getUpdates(bot.last_message_received + 1); }	
	#endif	
}


//========================================================
// PROCESAR TELEGRAMAS RECIBIDOS
//========================================================

void procesarTelegramas(int num_mensajes)
{
	for (int i = 0; i < num_mensajes; i++) {

		String from_name = bot.messages[i].from_name;  
		String chat_id = bot.messages[i].chat_id; 
		String text = bot.messages[i].text; 
		text.toLowerCase(); 

		#ifdef FLAG_SERIAL_DEBUG
			Serial.print(F("ID: ")); 
			Serial.print( chat_id );
			Serial.print(F(" >>> NOMBRE: ")); 
			Serial.print( from_name );    
			Serial.print(F(" >>> COMANDO: ")); 
			Serial.println( text );
		#endif

		if( FLAG_aviso_conexion_usuarios ) {
			if ( chat_id != ADMIN_USER) { 
			  String str_conexiones = from_name + "  >>  " + text;
			  bot.sendMessage( ADMIN_USER, str_conexiones ); 
			}
		}	

		if (text.equals("/start")) { 
			String mensaje_bienvenida = "Bienvenid@ a miniMeteo ";
			mensaje_bienvenida += _ID_STATION_;
			mensaje_bienvenida += "\nEscribe  /ayuda  para ver lo que puedo hacer";
			bot.sendMessage( chat_id, mensaje_bienvenida ); 
		}

		else if (text.equals("/ayuda") || text.equals("/help") ) { 
			enviar_lista_comandos_general(chat_id);
		}

		else if (text.equals("/clima")) { 
			if(FLAG_bmx_enable || FLAG_htu_enable) { enviar_informacion_clima( chat_id ); }
			else{ send_msg_error_sensores(chat_id); }
		}

		else if (text.equals("/minmax")) { 
			if(FLAG_bmx_enable || FLAG_htu_enable) {enviar_min_max_diarios( chat_id ); }   
			else{ send_msg_error_sensores(chat_id); }
		}

		else if (text.equals("/hora")) { dar_hora( chat_id ); }
		
		else if (text.equals("/historico")) { 
			if(FLAG_bmx_enable || FLAG_htu_enable) {enviar_min_max_historicos( chat_id ); }   
			else{ send_msg_error_sensores(chat_id); }
		}
		
		else if ( chat_id == ADMIN_USER ) {
			if ( text.equals("/red") ) { enviar_cabecera( chat_id , SOLO_RED); } 
			
			else if ( text.equals("/version") ) { enviar_cabecera( chat_id , SOLO_VERSION); } 
			
			else if ( text.equals("/info_bot") ) { enviar_cabecera( chat_id , VERSION_Y_RED); } 
			
			else if ( text.equals("/eeprom") ) { 
				if(check_eeprom_rw( POS_MEM_TEST )) {bot.sendMessage( chat_id, "eeprom ok" );} 
				else {bot.sendMessage( chat_id, "eeprom fail" );} 
			} 

			else if ( text.equals("/sensors") || text.equals("/sensores") ) { send_estado_sensores( chat_id ); }	
			
			else if ( text.equals("/status") || text.equals("/estado") ) { send_estado_banderas( chat_id ); }
			
			else if ( text.equals("/aviso_reinicio") ) {
				FLAG_aviso_reinicio=!FLAG_aviso_reinicio;
				save_flag_config();
				if(FLAG_aviso_reinicio){ bot.sendMessage( chat_id, "/aviso_reinicio:  ACTIVO" ); }
				else{ bot.sendMessage( chat_id, "/aviso_reinicio:  OFF" ); }				 
			}
			else if ( text.equals("/aviso_usuarios") ) {
				FLAG_aviso_conexion_usuarios=!FLAG_aviso_conexion_usuarios;
				save_flag_config();
				if(FLAG_aviso_conexion_usuarios){ bot.sendMessage( chat_id, "/aviso_usuarios:  ACTIVO" ); }
				else{ bot.sendMessage( chat_id, "/aviso_usuarios:  OFF" ); }				 
			}
			else if ( text.equals("/periodic_task") ) {
				FLAG_enable_periodic_task=!FLAG_enable_periodic_task;
				save_flag_config();
				if(FLAG_enable_periodic_task){ bot.sendMessage( chat_id, "/periodic_task:  ACTIVAS" ); }
				else{ bot.sendMessage( chat_id, "/periodic_task:  OFF" ); }				 
			}
			else if ( text.equals("/battery_control") ) {
				FLAG_enable_battery_control=!FLAG_enable_battery_control;
				save_flag_config();
				if(FLAG_enable_battery_control){ bot.sendMessage( chat_id, "/battery_control:  ACTIVO" ); }
				else{ bot.sendMessage( chat_id, "/battery_control:  OFF" ); }				 
			}
			else if ( text.equals("/led_wifi") ) {
				FLAG_led_conexion_wifi=!FLAG_led_conexion_wifi;
				save_flag_config();
				if(FLAG_led_conexion_wifi){ bot.sendMessage( chat_id, "/led_wifi:  ACTIVO" ); }
				else{ bot.sendMessage( chat_id, "/led_wifi:  OFF" ); }				 
			}
			else if ( text.equals("/led_setup") ) {
				FLAG_led_full_setup=!FLAG_led_full_setup;
				save_flag_config();
				if(FLAG_led_full_setup){ bot.sendMessage( chat_id, "/led_setup:  ACTIVO" ); }
				else{ bot.sendMessage( chat_id, "/led_setup:  OFF" ); }				 
			}
		
			else if ( text.equals("/default") ) {
				set_default_config();
				bot.sendMessage( chat_id, "**Establecida configuracion por defecto**\nParámetros actuales:\n" );
				send_estado_banderas( chat_id );				
			}
			
			else if ( text.equals("/vcc") || text.equals("/bat") ) {
				#ifdef FLAG_ADC_READ_A0
					enviar_estado_bateria_A0(chat_id); 
				#else
					enviar_estado_bateria_adc_int( chat_id );
				#endif
			}

			else if ( text.equals("/debug") ) { 
				FLAG_debug_comands = !FLAG_debug_comands;
				if( FLAG_debug_comands ){ enviar_lista_comandos_extra(chat_id); }
				else{
					digitalWrite(PIN_LED_OnBoard, Onboard_LED_OFF);
					bot.sendMessage( chat_id, "Comandos para debug, DESATIVADOS.\nLed apagado por precaucion" ); 
				}			  
			}
			
			//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
			else if (text.equals("/clear_dia") && FLAG_debug_comands ) { 
				reset_min_max_diarios();
				bot.sendMessage( chat_id, "min/max diarios, Borrados" ); 
			} 
			else if (text.equals("/clear_all")  && FLAG_debug_comands ) { 
				bot.sendMessage( chat_id, "Ultimo registro de Max y Min antes de restablecer valores:" );
				enviar_min_max_historicos(chat_id);
				bot.sendMessage( chat_id, "*************************" );
				reset_min_max_historicos();
				bot.sendMessage( chat_id, "min/max historicos, Borrados" ); 
			}
			
			else if (text.equals("/update_eeprom") && FLAG_debug_comands ) { 
				ultima_grabacion_eeprom = 0;
				FLAG_update_eeprom_data = true;
				bot.sendMessage( chat_id, "registros salvados en EEPROM" );
			} 
			
			else if ( text.equals("/on") && FLAG_debug_comands ) { 
				digitalWrite(PIN_LED_OnBoard, Onboard_LED_ON);   
				bot.sendMessage( chat_id, "Led ON" );  
			} 

			else if ( text.equals("/off") && FLAG_debug_comands ) { 
				digitalWrite(PIN_LED_OnBoard, Onboard_LED_OFF);   
				bot.sendMessage( chat_id, "Led OFF" );  
			}
			
			else if ( text.equals("/extra_info") && FLAG_debug_comands ) { 
				if(FLAG_bmx_enable || FLAG_htu_enable) { enviar_informacion_debug_admin( chat_id ); }
				else{ bot.sendMessage( chat_id, "fallan todos los sensores,\nRevise a MiniMeteo" ); }			  
			}			
			//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''	
			
			else{ bot.sendMessage( chat_id, "Comando Admin no valido" ); }  

		}

		else{ bot.sendMessage( chat_id, "ok" ); }
	}
}


//========================================================
// ENVIAR MENSAJE DE ERROR DE SENSORES
//========================================================

void send_msg_error_sensores( String user )
{ 
	bot.sendMessage( user, "Error General de sensores,\n\tcontacte con el Servicio Tecnico" );  	
}


//========================================================
// ESTABLECER CONFIGURACION POR DEFECTO
//========================================================
void set_default_config()
{
	FLAG_aviso_reinicio=true;
	FLAG_enable_battery_control=true;
	FLAG_enable_periodic_task=false;
	FLAG_aviso_conexion_usuarios=false;
	FLAG_led_conexion_wifi=true;
	FLAG_led_full_setup=false;
	
	//comprobar si hay datos en eepom o es un primer inicio del sistema
	uint8_t valor_referencia;
	eeprom_load(POS_MEM_CONFIG_OK, valor_referencia);
	if(valor_referencia != EEPROM_REF_DATA){
		reset_min_max_diarios();
		reset_min_max_historicos();
		EEPROM.write(POS_MEM_CONFIG_OK, EEPROM_REF_DATA);			
	}
	
	save_flag_config(); //asi aprovechamos el commit();
}
	
	
//========================================================
// ENVIAR ESTADO DE BANDERAS DE CONFIGURACION
//========================================================

void send_estado_banderas( String user)
{
	String msg_estado_banderas = "";
	if( FLAG_aviso_reinicio ){ msg_estado_banderas += "Aviso reinicio:  ACTIVO\n" ; }
	else{ msg_estado_banderas += "Aviso reinicio:  OFF\n" ; }				 

	if( FLAG_aviso_conexion_usuarios ){ msg_estado_banderas += "Aviso conexion usuarios:  ACTIVO\n" ; }
	else{msg_estado_banderas += "Aviso conexion usuarios:  OFF\n" ; }				 

	if( FLAG_enable_periodic_task ){ msg_estado_banderas += "Tareas Periodicas:  ACTIVAS\n" ; }
	else{ msg_estado_banderas += "Tareas Periodicas:  OFF\n" ; }				 

	if( FLAG_enable_battery_control ){ msg_estado_banderas += "Control de bateria:  ACTIVO\n" ; }
	else{ msg_estado_banderas += "Control de bateria:  OFF\n" ; }	

	if( FLAG_led_conexion_wifi ){ msg_estado_banderas += "Led conexion WiFi:  ACTIVO\n" ; }
	else{ msg_estado_banderas += "Led conexion WiFi:  OFF\n" ; }	
	
	if( FLAG_led_full_setup ){ msg_estado_banderas += "Led durante Setup:  ACTIVO\n" ; }
	else{ msg_estado_banderas += "Led durante Setup:  OFF\n" ; }	
	
	bot.sendMessage( user, msg_estado_banderas );
	
}
				
				
				
				
//========================================================
// ENVIAR MENSAJE CON LA LSITA DE COMANDOS GENERAL
//========================================================

void enviar_lista_comandos_general(String usuario)
{
	String mensaje_ayuda = "";
	mensaje_ayuda += "/clima:   Informacion actual\n";
	mensaje_ayuda += "/minmax:   T minima y T maxima del dia actual\n";
	mensaje_ayuda += "/historico:   T minima y T maxima (serie historica)\n";
	mensaje_ayuda += "/hora:   Muestra la fecha y hora actual\n";
	if ( usuario == ADMIN_USER ) {
		mensaje_ayuda += "/vcc:   Estado de la bateria\n";
		mensaje_ayuda += "/red:   Informacion sobre la conexion\n";
		mensaje_ayuda += "/version:   Version Hardware/Firmware\n";				
		mensaje_ayuda += "/info_bot:   Informacion completa del bot\n";				
		mensaje_ayuda += "/eeprom:   Comando para pruebas\n";				
		mensaje_ayuda += "/sensors:   Estado de los sensores\n";	
		mensaje_ayuda += "/status:   Estado de banderas de configuracion\n";					
		mensaje_ayuda += "/aviso_reinicio:   (on/off)  Aviso reinicio\n";	
		mensaje_ayuda += "/aviso_usuarios:   (on/off)  Alerta conexion usuarios\n";
		mensaje_ayuda += "/periodic_task:   (on/off)  Tareas periodicas\n";
		mensaje_ayuda += "/battery_control:  (on/off)  Control estado bateria\n";
		mensaje_ayuda += "/led_wifi:  (on/off)  Led durante conexion WiFi\n";
		mensaje_ayuda += "/led_setup:  (on/off)  Led durante todo el setup\n";
		mensaje_ayuda += "/default:   Establece configuracion por defecto\n";		
		mensaje_ayuda += "/debug:    Activa Comandos Extra (Usar con cuidado)\n";
	}
	
	bot.sendMessage( usuario, mensaje_ayuda ); 
	
	mensaje_ayuda = "";
}


//========================================================
// ENVIAR CABECERA CON DATOS DE CONEXION
//========================================================

void enviar_lista_comandos_extra(String usuario)
{
	String mensaje_ayuda_extra = "";
	mensaje_ayuda_extra += "Comandos para debug, ACTIVOS, usar con cuidado\n\n";
	mensaje_ayuda_extra += "/extra_info:   informacion debug sensores\n";
	mensaje_ayuda_extra += "/clear_dia:   borra el registro minmax diario\n";					
	mensaje_ayuda_extra += "/clear_all:   borra el registro minmax historico \n";
	mensaje_ayuda_extra += "/update_eeprom:   fuerza la grabacion de datos en la eeprom\n";
	mensaje_ayuda_extra += "/on:   enciende el led de pruebas\n";
	mensaje_ayuda_extra += "/off:   apaga el led de pruebas\n\n";
	mensaje_ayuda_extra += "/debug:   desactiva los comandos extra\n";
	
	bot.sendMessage( usuario, mensaje_ayuda_extra ); 
	
	mensaje_ayuda_extra = "";
}


//========================================================
// ENVIAR CABECERA CON DATOS DE CONEXION
//========================================================

void enviar_cabecera(String usuario, uint8_t _type)
{
	String local_ip = String() + WiFi.localIP()[0] + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3];
  
	bool flag_version = false;
	bool flag_red = false;
	
	if(_type==0){flag_version = true;}
	else if(_type==1){flag_red = true;}
	else if(_type==2){
		flag_version = true;
		flag_red = true;
	}
	
	String cabecera="";
	
	if( flag_version ){
	cabecera += _HARDWARE_;
	cabecera += ":  ";
	cabecera += _ID_STATION_;	
	cabecera += "\nFirmware:  ";
	cabecera += _FIRMWARE_;
	cabecera += "\n";
	}
	
	if( flag_red ){
	cabecera += "Conectado a:  ";
	cabecera +=  WiFi.SSID();
	cabecera += "\n";
	cabecera += "IP:  ";
	cabecera += local_ip;
	cabecera += "\n";
	update_time();
	cabecera += "Fecha/Hora:  ";
	cabecera += fecha_hora;
	}
	
	bot.sendMessage( usuario, cabecera );
}


//========================================================
// DAR LA HORA
//========================================================

void dar_hora(String usuario)
{
	update_time();
	
	String mensaje_fecha_hora = "Fecha/Hora:  " + fecha_hora;
	bot.sendMessage( usuario, mensaje_fecha_hora );
}



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    FECHA / HORA
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//========================================================

//    AJUSTE HORARIO INVIERNO / VERANO  
//========================================================

void ajustarHorarioInviernoVerano( void )
{ 
	uint8_t hora_cambio = 2;
	if (mes_actual==10){ hora_cambio = 3 ; } 
	
	if (mes_actual>3 && mes_actual<10) { FLAG_horario_verano = true ; } 
	else if (mes_actual==10 && dia_actual<25) { FLAG_horario_verano = true ; } 

	else if (mes_actual<3 && mes_actual>10) { FLAG_horario_verano = false ; } 
	else if (mes_actual==3 && dia_actual<25) { FLAG_horario_verano = false ; } 

	// ** la franja de tiempo 'delicada' son los ultimos dias de Marzo y de Octubre
	else if (( mes_actual==3 || mes_actual==10 ) && dia_actual>=25 ) { 
		// ** suponemos que es MARZO. Estamos buscando horario de verano
		if (dia_semana == 7) { 
			if (hora_actual < hora_cambio) { FLAG_horario_verano = false ; } 
			if (hora_actual >= hora_cambio) { FLAG_horario_verano = true ; }  		
		}
		else{	
			if (dia_actual-dia_semana>=25) { FLAG_horario_verano = true; } 	
			else { FLAG_horario_verano = false; } 	
		}
		//** si son ULTIMOS DIAS DE OCTUBRE es a la inversa **//
		if (mes_actual==10) { FLAG_horario_verano = !FLAG_horario_verano; } 		
			
	} 

	if (FLAG_horario_verano) { configTime("GMT-2", "pool.ntp.org", "time.nist.gov"); }  // VERANO OK
	else { configTime("GMT-1", "pool.ntp.org", "time.nist.gov"); }                      // INVIERNO OK      
	
	update_time();
}


//========================================================
// CONTROL DEL CAMBIO DE DIA
//========================================================

void update_eventos_reloj()
{ 
	update_time();
	
	static uint8_t hora_en_curso = 101;  	//iniciado fuera de rango
	
	if (dia_encurso != dia_actual) { 
		dia_encurso = dia_actual;
		
		EEPROM.write(POS_MEM_DIA_EN_CURSO, dia_encurso);
		EEPROM.commit(); 
	
		reset_min_max_diarios();
		
	}
	
	if (hora_en_curso != hora_actual) { 
		hora_en_curso = hora_actual;
		ajustarHorarioInviernoVerano();
	}
}


//========================================================
// ACTUALIZAR FECHA Y HORA
//========================================================

void update_time() 
{                                           //          1         2
	time_t now = time(nullptr);             //0123456789012345678901234
	const char *date_time = ctime(&now);    //Wed Jul 14 18:43:14 2021  --> string.substring(from, to) 
										    //Wed Jul 14 2021 / 18:43:14 
	String fecha = String(date_time);

	fecha_hora="";

	fecha_hora += fecha.substring(0, 4);    //'wed '
	fecha_hora += fecha.substring(8, 11);   //'14 '
	fecha_hora += fecha.substring(4, 8);    //'Jul '
	fecha_hora += fecha.substring(20,24);   //'2021'
	fecha_hora += " / ";
	fecha_hora += fecha.substring(11, 19);  //'hora... '
	

	String str_dia = fecha.substring(0, 3); //'Wed' (sin el espacio extra)
	str_dia.toLowerCase();                  // convertir a minusculas para las comparaciones

	if(str_dia=="mon"){ dia_semana=1; }
	else if(str_dia=="tue"){ dia_semana=2; }
	else if(str_dia=="wed"){ dia_semana=3; }
	else if(str_dia=="thu"){ dia_semana=4; }
	else if(str_dia=="fri"){ dia_semana=5; }
	else if(str_dia=="sat"){ dia_semana=6; }
	else if(str_dia=="sun"){ dia_semana=7; }


	dia_actual = fecha.substring(8, 10).toInt();  //'14' (sin el espacio extra)

	String str_mes = fecha.substring(4, 7);       //'Jul' (sin el espacio extra)
	str_mes.toLowerCase();                        // convertir a minusculas para las comparaciones

	if(str_mes=="jan"){ mes_actual=1; }
	else if(str_mes=="feb"){ mes_actual=2; }
	else if(str_mes=="mar"){ mes_actual=3; }
	else if(str_mes=="apr"){ mes_actual=4; }
	else if(str_mes=="may"){ mes_actual=5; }
	else if(str_mes=="jun"){ mes_actual=6; }
	else if(str_mes=="jul"){ mes_actual=7; }
	else if(str_mes=="aug"){ mes_actual=8; }
	else if(str_mes=="sep"){ mes_actual=9; }
	else if(str_mes=="oct"){ mes_actual=10; }
	else if(str_mes=="nov"){ mes_actual=11; }
	else if(str_mes=="dec"){ mes_actual=12; }

	anno_actual = fecha.substring(20,24).toInt();          //'2023'

	hora_actual = fecha.substring(11, 13).toInt();

}



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    SENSORES
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//========================================================
// ENVIAR INFORMACION CLIMATICA
//========================================================

void enviar_informacion_clima(String usuario)
{
	update_sensores();

	String   info_clima = "Temperatura:  ";               
	info_clima += String(temperatura);
	info_clima += " ºC\n";

	if( FLAG_bmx_enable ) {
		info_clima += "Presion:  ";
		info_clima += String(presionREL);
		info_clima += " mb\n";
	}
	if( FLAG_htu_enable ) {
		info_clima += "Humedad:  "; 
		info_clima += String(humedad);
		info_clima += " %\n";
	}

	bot.sendMessage( usuario, info_clima );
}


//========================================================
// ENVIAR INFORMACION CLIMATICA - debug sensores
//========================================================

void enviar_informacion_debug_admin(String usuario)
{
	update_sensores();

	String   info_clima = "";               

	if( FLAG_bmx_enable ) {
		info_clima = "DEBUG Temperatura BMP:  ";
		info_clima += String(tempBMX);
		info_clima += " ºC\n";
		info_clima += "DEBUG Presion ABS:  ";
		info_clima += String(presionABS);
		info_clima += " mb\n";
		info_clima += "DEBUG Presion REL:  ";
		info_clima += String(presionREL);
		info_clima += " mb\n";
	}	
	if( FLAG_htu_enable ) {	
		info_clima += "DEBUG Temperatura HTU:  ";        //HTU
		info_clima += String(tempHTU);
		info_clima += " ºC\n";
		info_clima += "DEBUG Humedad:  ";                //HTU
		info_clima += String(humedad);
		info_clima += " %\n";
	}

	bot.sendMessage( usuario, info_clima );
}


//========================================================
// ENVIAR INFORMACION DE MIN Y MAX
//========================================================

void enviar_min_max_diarios(String usuario)
{
	update_sensores();

	String info_temp = "Temperatura Actual:  "; 
	info_temp += String(temperatura);
	info_temp += " ºC";
	info_temp += "\n\nminima hoy:  ";  
	info_temp += String(temperatura_min_dia);
	info_temp += " ºC\n";
	info_temp += "Hora Tmin:  ";
	info_temp += str_hora_Tmin_dia;
	info_temp += "\n\nMaxima hoy:  ";  
	info_temp += String(temperatura_max_dia);
	info_temp += " ºC\n";
	info_temp += "Hora Tmax:  ";
	info_temp += str_hora_Tmax_dia;
	bot.sendMessage( usuario, info_temp );
}


//========================================================
// ENVIAR INFORMACION DE MIN Y MAX (historico)
//========================================================

void enviar_min_max_historicos(String usuario)
{
	update_sensores();

	String info_temp = "Minimos y maximos historicos:\n(Desde  "; 
	info_temp += str_fecha_inicio_historico;
	info_temp += "  hasta hoy)\n\n";
	info_temp += "T. MIN:  ";  
	info_temp += String(temperatura_min_historico);
	info_temp += " ºC\n";
	info_temp += "\tfecha:  ";
	info_temp += str_fecha_Tmin_historico;
	info_temp += "\n\nT. MAX:  ";  
	info_temp += String(temperatura_max_historico);
	info_temp += " ºC\n";
	info_temp += "\tfecha:  ";
	info_temp += str_fecha_Tmax_historico;

	bot.sendMessage( usuario, info_temp );
}


//========================================================
// RESET DE MIN/MAX  DIARIOS
//========================================================

void reset_min_max_diarios()
{
	temperatura_min_dia = 1000;
	temperatura_max_dia = -1000;
	update_sensores();
	EEPROM.commit();
}


//========================================================
// RESET DE MIN/MAX  HISTORICOS
//========================================================

void reset_min_max_historicos()
{
	temperatura_min_historico = 1000; 
	temperatura_max_historico = -1000;
	
	update_sensores();
	
	str_fecha_inicio_historico = fecha_hora.substring(0, 15);  //solo la fecha	
	
	char char_array20[20];  //array caracteres
	str_fecha_inicio_historico.toCharArray(char_array20, str_fecha_inicio_historico.length()+1);
	
	eeprom_save(POS_MEM_FECHA_START_HISTORICO, char_array20);
	EEPROM.commit();
}


//========================================================
// ACTUALIZAR VARIABLES CLIMATICAS 
//========================================================

void update_sensores()
{
	if (FLAG_bmx_enable){ 
		tempBMX = sensor_bmx280.readTemperature();
		
		if( ! ISNAN(tempBMX) ){ temperatura = tempBMX; }
		
		presionABS = sensor_bmx280.readPressure()/100.0;
		
		if( ! ISNAN(presionABS) ){
			
			presionREL = (presionABS / pow(1.0-ALTITUD/44330, 5.255));       //calculada por formula 

			//#ifdef FLAG_SERIAL_DEBUG
			//	Serial.println(F("DEBUG Temperatura BME280:  "));
			//	Serial.println(temperatura);
			//	Serial.println(F("DEBUG PresionREL:  "));
			//	Serial.println(presionREL);
			//#endif		
		}
	}

	
	if (FLAG_htu_enable){ 	
		humedad = sensorHTU.readHumidity();
		tempHTU = sensorHTU.readTemperature(); 
		
		if( ! ISNAN(humedad) ){ 

			//#ifdef FLAG_SERIAL_DEBUG
			//	Serial.println(F("DEBUG Humedad RAW:  "));
			//	Serial.println(humedad);
			//#endif		

			if(humedad<0) { humedad=0; }
			if(humedad>100) { humedad=100; }

			//#ifdef FLAG_SERIAL_DEBUG
			//	Serial.println(F("DEBUG Temperatura HTU21:  "));
			//	Serial.println(tempHTU);
			//	Serial.println(F("DEBUG Humedad Ajustada:  "));
			//	Serial.println(humedad);
			//#endif		
		}	   
		
		if (!FLAG_bmx_enable){
				if( !ISNAN(tempHTU) ){ temperatura = tempHTU; }
		}
		
	}

	//control de min/max
	if( !ISNAN(temperatura) ){
		bool FLAG_need_eeprom_commit = false;	// TRUE: datos para grabarse en eeprom (diferidos o en tiempo real)
		if(temperatura<temperatura_min_dia) { 
			temperatura_min_dia=temperatura; 
			eeprom_save(POS_MEM_T_MIN_DIARIO, temperatura_min_dia);

			update_time();
			str_hora_Tmin_dia = fecha_hora.substring(18);  //solo la hora:min:seg
			char  char_array[10]="";
			str_hora_Tmin_dia.toCharArray(char_array, str_hora_Tmin_dia.length()+1);
			eeprom_save(POS_MEM_HORA_MIN_DIARIO, char_array); //char array  -> eeprom	
			FLAG_need_eeprom_commit = true;
			
			#ifdef FLAG_SERIAL_DEBUG
				Serial.print(F("min_dia:"));Serial.print(temperatura_min_dia);
				Serial.print(F(", a las  "));Serial.println(str_hora_Tmin_dia);
			#endif
		}
		
		if(temperatura>temperatura_max_dia) { 
			temperatura_max_dia=temperatura;
			eeprom_save(POS_MEM_T_MAX_DIARIO, temperatura_max_dia);

			update_time();
			str_hora_Tmax_dia = fecha_hora.substring(18);  //solo la hora:min:seg 
			
			char  char_array[10]="";
			str_hora_Tmax_dia.toCharArray(char_array, str_hora_Tmax_dia.length()+1);
			eeprom_save(POS_MEM_HORA_MAX_DIARIO, char_array); //char array  -> eeprom	
			FLAG_need_eeprom_commit = true;
			
			#ifdef FLAG_SERIAL_DEBUG			
				Serial.print(F("max_dia:"));Serial.print(temperatura_max_dia);
				Serial.print(F(", a las  "));Serial.println(str_hora_Tmax_dia);
			#endif
		}
		
 		if(temperatura<temperatura_min_historico) { 
			temperatura_min_historico=temperatura; 
			eeprom_save(POS_MEM_T_MIN_HISTORICO, temperatura_min_historico);		

			update_time();
			str_fecha_Tmin_historico = fecha_hora; 
			
			char  char_array[30]="";
			str_fecha_Tmin_historico.toCharArray(char_array, str_fecha_Tmin_historico.length()+1);
			eeprom_save(POS_MEM_FECHA_MIN_HISTORICO, char_array); //char array  -> eeprom
			FLAG_need_eeprom_commit = true;
			
			#ifdef FLAG_SERIAL_DEBUG
				Serial.print(F("min_historico:"));Serial.print(temperatura_min_historico);
				Serial.print(F(", a las  "));Serial.println(str_fecha_Tmin_historico);	
			#endif
		}
		
		if(temperatura>temperatura_max_historico) { 
			temperatura_max_historico=temperatura;
			eeprom_save(POS_MEM_T_MAX_HISTORICO, temperatura_max_historico);
			
			update_time();
			str_fecha_Tmax_historico = fecha_hora; 

			char  char_array[30]="";
			str_fecha_Tmax_historico.toCharArray(char_array, str_fecha_Tmax_historico.length()+1);
			eeprom_save(POS_MEM_FECHA_MAX_HISTORICO, char_array); //char array  -> eeprom	
			FLAG_need_eeprom_commit = true;
			
			#ifdef FLAG_SERIAL_DEBUG			
				Serial.print(F("max_historico:"));Serial.print(temperatura_max_historico);
				Serial.print(F(", a las  "));Serial.println(str_fecha_Tmax_historico);
			#endif
		}
		if( FLAG_need_eeprom_commit ){
			if (FLAG_eeprom_life_extend){ FLAG_update_eeprom_data=true; }  // postergamos la grabacion de datos en eeprom
			else{ EEPROM.commit();  }									   // grabacion en tiempo real
		}
	}	
}


//========================================================
// NOTIFICAR EL ESTADO DE LOS SENSORES CONECTADOS
//========================================================

void send_estado_sensores( String user )
{	
	if(!FLAG_bmx_enable && !FLAG_htu_enable) {
		send_msg_error_sensores(user);  
	}

	else if(!FLAG_bmx_enable || !FLAG_htu_enable) {
		String mensaje_error = "Error en algunos sensores,\n\tcontacte con el Servicio Tecnico\n\n";
		
		if(!FLAG_bmx_enable ) {mensaje_error+= "BMx280:  FAIL\n"; }
		else { mensaje_error += "BMx280:  OK\n"; }
		
		if(!FLAG_htu_enable ) {mensaje_error+= "HTU21D:  FAIL"; }
		else { mensaje_error += "HTU21D:  OK"; }
		
		bot.sendMessage( user,  mensaje_error);     
	}
}



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    BATERIA
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//========================================================
// CONTROL ESTADO BATERIA (con ADC interno)
//========================================================

uint16_t control_bateria_con_adc_int( uint8_t _modo )
{
	float voltaje = (ESP.getVcc() / 1023.00f) * FACTOR_AJUSTE_ADC_INT;

	int16_t porcentaje = 100-(420-voltaje*100);
	if(porcentaje<0) { porcentaje=0; }
	else if (porcentaje>100) { porcentaje=100; }


	if(_modo==1){ return porcentaje; }  	// retorna %
	else{ return int(voltaje*1000); }		// retorna mV
}


//========================================================
// ENVIAR ESTADO BATERIA (con ADC interno)
//========================================================

void enviar_estado_bateria_adc_int( String user ) 
{
	float voltaje = control_bateria_con_adc_int(0)/1000.0;
	int16_t porcentaje = control_bateria_con_adc_int(1);
	String mensaje_vcc = "Estado bateria:  " + String(voltaje) + " V  -- ( " +  String(porcentaje) + " % )";
	bot.sendMessage( user, mensaje_vcc ); 	
}


//========================================================
// CONTROL ESTADO BATERIA (con Divisor de tension en A0)
//========================================================

int16_t control_bateria_con_A0( uint8_t _modo ) 
{
	float voltaje=0.00f;  			//en  voltios
	const float carga_max = 4.2; 	// V
	const float acd_max = 3.3; 		// V
	
	float analog_raw = analogRead(A0);
	
    voltaje = (FACTOR_AJUSTE_A0*2*acd_max*analog_raw)/1023.00f;
	int16_t porcentaje = 100-(420-voltaje*100);

	if(porcentaje<0) { porcentaje=0; }
	else if (porcentaje>100) { porcentaje=100; }
	
	if(_modo==1){ return porcentaje; }  	// retorna %
	else{ return int(voltaje*1000); }		// retorna mV
}


//========================================================
// ENVIAR ESTADO BATERIA (Lectura A0)
//========================================================

void enviar_estado_bateria_A0( String user ) 
{
	float voltaje = control_bateria_con_A0(0)/1000.0;
	int16_t porcentaje = control_bateria_con_A0(1);
	String mensaje_vcc = "Estado bateria:  " + String(voltaje) + " V  -- ( " +  String(porcentaje) + " % )";
	bot.sendMessage( user, mensaje_vcc ); 
}



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//    HABILITAR FUNCIONALIDAD OTA
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/
 
void setup_OTA() 
{
	// Port defaults to 8266
	// ArduinoOTA.setPort(8266);

	
    #ifdef OTA_DEVICE_NAME
      // Hostname defaults to esp8266-[ChipID]
      ArduinoOTA.setHostname(OTA_DEVICE_NAME);  //comentar apra dejar el nombre por defecto
    #endif
    
    #ifdef OTA_UPDATE_PASS
      ArduinoOTA.setPassword(OTA_UPDATE_PASS);  //comentar para dejar sin clave
      // Password can be set with it's md5 value as well
      // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
      // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");	
    #endif	
	
  
	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH) {
			type = "sketch";
		} else { // U_SPIFFS
			type = "filesystem";
		}

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		#ifdef FLAG_SERIAL_DEBUG
			Serial.print(F("Start updating "));
			Serial.println(type);
		#endif
	});

	#ifdef FLAG_SERIAL_DEBUG
		ArduinoOTA.onEnd([]() {
			
				Serial.println(F("\nEnd"));

		});

		ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
			Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
		});

		ArduinoOTA.onError([](ota_error_t error) {
			Serial.printf("Error[%u]: ", error);
			if (error == OTA_AUTH_ERROR) {
				Serial.println(F("Auth Failed"));
			} else if (error == OTA_BEGIN_ERROR) {
				Serial.println(F("Begin Failed"));
			} else if (error == OTA_CONNECT_ERROR) {
				Serial.println(F("Connect Failed"));
			} else if (error == OTA_RECEIVE_ERROR) {
				Serial.println(F("Receive Failed"));
			} else if (error == OTA_END_ERROR) {
				Serial.println(F("End Failed"));
			}
		});
	#endif
	
	ArduinoOTA.begin();
	
	#ifdef FLAG_SERIAL_DEBUG
		Serial.println(F("\n\nOTA iniciado\n"));
	#endif
}



/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//       CONTROL DEL WACTH DOG TIMER
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//========================================================
// DESHABILITAR WACTH DOG TIMER HARDWARE
//========================================================

void hw_wdt_disable(){
	*((volatile uint32_t*) REG_WDT) &= ~(1); // Hardware WDT OFF
}


//========================================================
// HABILITAR WACTH DOG TIMER HARDWARE
//========================================================

void hw_wdt_enable(){
	*((volatile uint32_t*) REG_WDT) |= 1; // Hardware WDT ON
}


//========================================================
// ESTABLECER TIMEOUT PARA WACTH DOG TIMER HARDWARE
//========================================================

void hw_wdt_timeout(uint32_t _timeout)
{
	//WRITE_REG(WDT_TIMEOUT_REG, _timeout); 
	(*((volatile uint32_t *)(WDT_TIMEOUT_REG))) = (uint32_t)(_timeout);
}




//*******************************************************
//                    FIN DE PROGRAMA
//*******************************************************






