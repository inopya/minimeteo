/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        SECCION DE DEFINICION DE VARIABLES GLOBALES
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

float tempHTU;  //almacena la temperatura del higrometro
float tempBMX;  //almacena la temperatura del barometro (se usa por defecto si hay barometro)
float humedad; 
float presionABS;
float presionREL; 


float temperatura; //la que se entrega al usuario
float temperatura_min_dia = 1000; 
float temperatura_max_dia = -1000; 

float temperatura_min_historico = 1000; 
float temperatura_max_historico = -1000; 


String fecha_hora="";
String str_hora_Tmin_dia="";
String str_hora_Tmax_dia="";
String str_fecha_inicio_historico="";
String str_fecha_Tmin_historico="";
String str_fecha_Tmax_historico="";

uint32_t momento_actual = 0;

uint32_t ultimo_chequeo_wifi = 0;
uint32_t ultimo_acceso_telegram = 0;
uint32_t ultima_lectura_sensores = 0;
uint32_t ultimo_control_horario = 0;
uint32_t ultimo_control_bateria = 0;
uint32_t ultima_tarea_periodica = 0;
uint32_t ultima_grabacion_eeprom = 0;

bool FLAG_htu_enable = false;  	// los sensores se autodetectan en el setup
bool FLAG_bmx_enable = false;	// los sensores se autodetectan en el setup

uint8_t anno_actual;
uint8_t mes_actual;
uint8_t dia_semana;
uint8_t hora_actual;
uint8_t dia_actual;
uint8_t dia_encurso = 101;  		// iniciado fuera de rango para el control de cambio de dia

bool FLAG_horario_verano = false;  		// elegimos uno al azar, se actualiza en setup
volatile bool FLAG_update_eeprom_data = false;  	// TRUE: permiso para grabar datos de forma diferia

bool FLAG_debug_comands = false;    //TRUE: indica que estan activos los comandos extra
bool FLAG_ahorro_bateria = false;	// TRUE: de desconecta la wifi durante en ciertos periodos //sin implementar aun
		
