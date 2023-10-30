// !!!!!! En la libreria del TFT hay que configurar los pines e invcertir los colores en el fichero User_Setup.h
// #define ILI9341_2_DRIVER     // Alternative ILI9341 driver, see https://github.com/Bodmer/TFT_eSPI/issues/1172 ( descomentar )
// comentar estas
// #define TFT_CS   PIN_D8  // Chip select control pin D8
// #define TFT_DC   PIN_D3  // Data Command control pin
// #define TFT_RST  PIN_D4  // Reset pin (could connect to NodeMCU RST, see next line)
// ###### EDIT THE PIN NUMBERS IN THE LINES FOLLOWING TO SUIT YOUR ESP32 SETUP   ######
// For ESP32 Dev board (only tested with ILI9341 display)
// The hardware SPI can be mapped to any pins
// #define TFT_MISO 19 ( no es necesaria )
// #define TFT_MOSI 23
// #define TFT_SCLK 18
// #define TFT_CS    4  // Chip select control pin
// #define TFT_DC   15  // Data Command control pin
// #define TFT_RST   4  // Reset pin (could connect to RST pin) Comentar esta línea
// #define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST
// If colours are inverted (white shows as black) then uncomment one of the next
// 2 lines try both options, one of the options should correct the inversion.
// #define TFT_INVERSION_ON ( descomentar )

#include <SPI.h>
#include <Wire.h>
#include <max6675.h>
#include <TFT_eSPI.h>
#include <Adafruit_NeoPixel.h>
#include <iconos.h>

/*******************************************************************************
  Ajustes fases de reflow.
  En función del dispositivo descomentar las que correspondan a este.
*******************************************************************************/
// HOT-PLATE
#define READY    {"READY",TFT_WHITE,25.0,0,255,255,255,0,0}
#define PREHEAT  {"PRE-HEAT",TFT_GREENYELLOW,125.0,90.0,153,153,0,1.0}
#define SOAK     {"SOAK",TFT_ORANGE,180.0,90.0,204,102,0,1.8}
#define REFLOW   {"REFLOW",TFT_RED,210.0,30.0,255,0,0,14.0}
#define COOLING  {"COOLING",TFT_BLUE,180.0,30.0,0,0,255,0,2.5}
#define DONE     {"DONE",TFT_GREEN,0,0,0,255,0,0,0}

// OVEN
// #define READY   {"READY",TFT_WHITE,25.0,0,255,255,255,0,0}
// #define PREHEAT {"PRE-HEAT",TFT_GREENYELLOW,125.0,90.0,153,153,0,16.0}
// #define SOAK    {"SOAK",TFT_ORANGE,180.0,90.0,204,102,0,14.0}
// #define REFLOW  {"REFLOW",TFT_RED,210.0,30.0,255,0,0,20.0}
// #define COOLING {"COOLING",TFT_BLUE,180.0,30.0,0,0,255,0,2.5}
// #define DONE    {"DONE",TFT_GREEN,0,0,0,255,0,0,0}

// Definiciones genericas
#define TEMPERATURA_SEGURA    50
#define NUM_LEDS_ENCODER      8

// Carga las fuentes a utilizar
#define FONT2 2
#define FONT4 4
#define FONT6 6
#define FONT7 7
#define FONT8 8

// Definiciones pines MAX6675-1 MAX6675-2
#define MAX_SCK     12    // Reloj Max6675
#define MAX_SO      13    // Salida de datos MAX6675
#define MAX_CS1     14    // Seleccion MAX6675-1
#define MAX_CS2     32    // Seleccion MAX6675-2
#define TIME_SAMPLE 1000  // Tiempo de lectura temperatura cada 1s


// Definiciones encoder
#define CK_ENCODER  35    // Señal reloj encoder
#define DT_ENCODER  34    // Señal de datos encoder
#define SW_ENCODER  33    // Pulsador encoder
#define DELAY_CK    20    // Retardo giro encoder
#define DELAY_SW    300   // Retardo pulsador encoder

// Definiciones salidas y entradas
#define BUZZER        2   // Control zumbador
#define SSR           27  // Rele estado solido resistencia
#define FAN           26  // Rele estado solido ventilador
#define LED_ENCODER   25  // Tira led WS2812 encoder
#define ZERO_CROSSING 39  // Paso por cero AC
#define LED_CPU       5   // Led CPU

// Definiciones puerto serie (Pantalla Nextion)
#define RX_232        17  // Recepción RS-232 
#define TX_232        19  // Transmisión RS-232 

// Definiciones PWM BUZZER
#define PWM_BUZZER_CHANNEL    0     // Canal PWM asigado a buzzer
#define PWM_BUZZER_FREQUENCY  10000 // Frecuencia del canal Hz
#define PWM_BUZZER_RESOLUTION 8     // Resolucion canal

// Definiciones PWM SSR
#define PWM_SSR_CHANNEL       2     // Canal PWM asignado a SSR
#define PWM_SSR_FREQUENCY     5     // Frecuencia del canal Hz
#define PWM_SSR_RESOLUTION    8     // Resolucion del canal

// Definiciones PWM FAN
#define PWM_FAN_CHANNEL       4     // Canal PWM asignado a SSR
#define PWM_FAN_FREQUENCY     10    // Frecuencia del canal Hz
#define PWM_FAN_RESOLUTION    8     // Resolucion del canal

struct menus
{
    String       nombre;
    unsigned int pos_x;
    unsigned int pos_y;
};

struct fases
{   
    String nombre;          // Nombre a mostrar en pantalla
    int    color;           // Color del texto en pantalla
    float  temperatura;     // Temperatura de consigna
    float  tiempo;          // Tiempo para conseguir la temperatura
    int    R;               // Valor rojo led
    int    G;               // Valor verde led
    int    B;               // Valor azul led
    float  k_pwm;           // Constante K PWM SSR
    float  k_fan;           // Constante K PWM FAN
};


// Crea objeto pantalla TFT
TFT_eSPI tft = TFT_eSPI();    // TFT

// Crea objetos termopares 
MAX6675 termopar1;            // MAX6675-1
MAX6675 termopar2;            // MAX6675-2

// Crea objeto tira de led WS2812 Encoder
Adafruit_NeoPixel leds = Adafruit_NeoPixel(NUM_LEDS_ENCODER, LED_ENCODER, NEO_GRB + NEO_KHZ800); // Led WS2812B
 
menus menu[4];

fases fase[6];

// Define variables globales
float   temperatura;                // Temperatura de horno / hot-plate
float   temperatura_2;
int     pwm_ssr            = 0;     // Valor de 0-100% del PWM resistencia
int     pwm_fan            = 0;     // Valor de 0-100% del PWM ventilador
int     posActualEncoder   = 0;     // Posición actual encoder
int     posFinalEncoder    = 7;    // Posición maxima encoder

bool  estadoPulsador     = false;  // Estado pulsador encoder
bool  error              = false;  // Error sonda temperatura
bool  estadoHorno        = false;  // Estado horno

long  zero;

float tempConsigna       = 25.0;  // Temperatura de consigna inicial
float incrementoTemp     = 0;     // Incremento de temperatura         


  int progreso,_progreso;

// // Define variables NTC para calcular la temperatura
// int Vo;
// float R1 = 100000;              // resistencia fija del divisor de tension 
// float logR2, R2, tempNTC;
// float c1 = 2.114990448e-03, c2 = 0.3832381228e-04, c3 = 5.228061052e-07;
// // coeficientes de S-H en pagina: 
// // http://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm
