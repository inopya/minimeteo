
/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        CONSTANTES DEL PROGRAMA
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/



//==========================================================
//		WACTH DOG TIMER HARDWARE
//==========================================================

#define REG_WDT        				(0x60000900)
#define WDT_TIMEOUT_REG   			(REG_WDT + 0x8)
#define WRITE_REG(addr, val)    	(*((volatile uint32_t *)(addr))) = (uint32_t)(val)




//==========================================================
//		OTRAS constantes
//==========================================================

#define Onboard_LED_ON    0    //el led interno del wemos esta conectado a una salida Pull-up (enciende con LOW)
#define Onboard_LED_OFF   1

const uint8_t EEPROM_REF_DATA = 101;   //contenido en una posicion de ref. para indicar que la eeporm contiene datos


//==========================================================
//		EEPROM
//==========================================================

#define EEPROM_SIZE					  (200)   // porcion de memoria flash reservada para eeprom

/* (POS_MEM...) direccion  de memoria  --> (...)  tipo de dato que almacena y detalles */

#define POS_MEM_T_MIN_DIARIO   	  		(0)  	// (4 bytes) float
#define POS_MEM_T_MAX_DIARIO      	  	(4)  	// (4 bytes) float

#define POS_MEM_T_MIN_HISTORICO         (8) 	// (4 bytes) float
#define POS_MEM_T_MAX_HISTORICO       	(12) 	// (4 bytes) float

#define POS_MEM_HORA_MIN_DIARIO			(20) 	// (10 bytes) String -> char array 
#define POS_MEM_HORA_MAX_DIARIO			(30) 	// (10 bytes) String -> char array


#define POS_MEM_FECHA_START_HISTORICO  	(40) 	// (20 bytes) String -> char array
#define POS_MEM_FECHA_MIN_HISTORICO  	(60) 	// (30 bytes) String -> char array
#define POS_MEM_FECHA_MAX_HISTORICO 	(90) 	// (30 bytes) String -> char array

#define POS_MEM_AVISO_REINICIO		 	(120) 	// (1 byte)	  bool -> char/byte/uint8_t		
#define POS_MEM_PERIDIC_TASK		 	(122) 	// (1 byte)	  bool -> char/byte/uint8_t
#define POS_MEM_BATTERY_CONTROL		 	(124) 	// (1 byte)	  bool -> char/byte/uint8_t
#define POS_MEM_AVISO_USUARIOS		 	(126) 	// (1 byte)	  bool -> char/byte/uint8_t
#define POS_MEM_LED_WIFI		 		(128) 	// (1 byte)	  bool -> char/byte/uint8_t
#define POS_MEM_LED_SETUP		 		(130) 	// (1 byte)	  bool -> char/byte/uint8_t

#define POS_MEM_DIA_EN_CURSO	 		(132) 	// (1 byte)	  bool -> char/byte/uint8_t

#define POS_MEM_CONFIG_OK	 			(140) 	// (1 byte)	  bool -> char/byte/uint8_t
 
#define  POS_MEM_TEST					(190) 	// (1 byte)	  bool -> char/byte/uint8_t