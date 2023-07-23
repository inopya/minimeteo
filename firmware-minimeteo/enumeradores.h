
/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//   ENUMERADORES UTILES
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

//ESTE enum SE ENCUENTRA EN LOS FICHEROS DEL CORE DEL ESP8266 
//pero vscode no lo reconoce, asique lo "re-creamos"

enum esp_hw_wdt_timeout 
{
	TIMEOUT_DISABLE   = 0,
	TIMEOUT_0_84_SEC  = 10,
	TIMEOUT_1_68_SEC  = 11,
	TIMEOUT_3_36_SEC  = 12,
	TIMEOUT_6_71_SEC  = 13,
	TIMEOUT_13_4_SEC  = 14,
	TIMEOUT_26_8_SEC  = 15,
};
							

/* ENUM INTERVALOS DE TIEMPO */ 
enum intervalos_tiempo 
{
	INTERVALO_CHECK_WIFI	= 	60,   // en segundos
	TIMEOUT_CONECT_TO_WIFI  = 	15,   // en segundos
	TIMEOUT_AP_PORTAL		=	60,	  // en segundos
};	


	/* ENUM ESTADOS DE OPERACION */		//sin uso
enum info_type 
{
	SOLO_VERSION ,   
	SOLO_RED ,   	
	VERSION_Y_RED ,     	
};
							
/* ENUM ESTADOS DE OPERACION */		//sin uso
enum machineState 
{
	MODE_CONFIG ,   
	MODE_WAIT  ,   	
	MODE_TEST_WIFI ,  
	MODE_SLEEP ,
	MODE_NOTICE ,    
	MODE_STOP ,   	
};



/* ENUM TIPOS DE PULSACION */		//sin uso
enum tipoPulsacion 
{
	PULSACION_NULA		=	0,
	PULSACION_CORTA		=  	1,   
	PULSACION_DOBLE		=	2,   
	PULSACION_LARGA		=	3, 
	PULSACION_MANTENIDA	=	9, 	
};



