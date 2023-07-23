
/*mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//        ALGUNAS DEFINICIONES PERSONALES PARA MI COMODIDAD AL ESCRIBIR CODIGO
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm*/

#define AND         &&
#define OR          ||
#define NOT          !
#define ANDbit       &
#define ORbit        |
#define XORbit       ^
#define NOTbit       ~

#define getBit(data,y)       ((data>>y) & 1)           // Obtener el valor  del bit (data.y)
#define setBit(data,y)       data |= (1 << y)          // Poner a 1 el bit (data.y) 
#define clearBit(data,y)     data &= ~(1 << y)         // Poner a 0 el bit (data.y)
#define togleBit(data,y)     data ^= (1 << y)          // Invertir el valor del bit (data.y)
#define togleByte(data)      data = ~data              // Invertir el valor del byte (data)

#define ISNAN(X) (!((X)==(X)))

