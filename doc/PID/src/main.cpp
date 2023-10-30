#include <TimerOne.h> 
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>
#include <SoftwareSerial.h>

// Definiciones Hardware
#define SSR 11
#define CPU 10
#define T_CONSIGNA 100    // Temperatura de consigna
#define T_MUESTREO 10      // Tiempo de muestreo sistema


// Definiciones lectura de tenmperatura
#define thermoDO  4
#define thermoCS  3
#define thermoCLK 2

// Definiciones puerto serie debug
#define RX 5
#define TX 6


SoftwareSerial SerialDebug(TX, RX); // Rx, TX

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

// Variables Globales
int T_OVEN;                           // Temperatura del horno
int T_SET = T_CONSIGNA;                // Referencia de temperatura
volatile float u = 0.0, u_1 = 0.0;    // Acción de control
int Ts = T_MUESTREO;                  // Periodo de muestreo
char buffer[16];
int x;
int pwm;
int power;
int comando;
bool estado_horno = false;
bool debug = false;
bool aux = false;
String s = " ";
int valor;

//Parámetros del PID
float kp, ti, td;
float q0, q1, q2;  
volatile float e = 0.0, e_1 = 0.0, e_2 = 0.0, conta = 0;
float k=8.59, tau=180.75, theta=36.88+Ts/2;    // Parámetros obtenidos Matlab

/*===========================================================================*/
/*=======================    FUNCION DEL CONTROL PID  =======================*/
/*===========================================================================*/
void PID(void)
{
e = (T_SET - T_OVEN);

// Controle PID
u = u_1 + q0*e + q1*e_1 + q2*e_2;  // Ley del controlador PID discreto

if (u >= 100.0)                   // Saturo la accion de control 'uT'
  {
  u = 100.0;
  }                               // en un tope maximo y minimo
if (u <= 0.0 || T_SET == 0)
  {
  u = 0.0;
  }

//Retorno a los valores reales
e_2 = e_1;
e_1 = e;
u_1 = u;

//La accion calculada la transformo en PWM
power = map(u,0,100,0,100);
pwm = map(u, 0,100, 0,255);

if (estado_horno == true)
  {
  analogWrite(SSR,pwm);
  }
else
  {
  analogWrite(SSR,0);
  power = 0;
  }
}

/*===========================================================================*/
/*=======================    FUNCION INTERRUCPION CADA 1 SEG  ===============*/
/*===========================================================================*/
void SampleTime(void)
{

digitalWrite(CPU, !digitalRead(CPU)); //Led Toggle cada segundo
conta ++;
if (conta >= T_MUESTREO)
  {
    conta = 0;     
    PID();
  }
T_OVEN = thermocouple.readCelsius();
aux = true;
}

/*===========================================================================*/
/*=======================    FUNCION LECTURA PUERTO SERIE====================*/
/*===========================================================================*/
int lee_serie(void)
{
  int c;
 
  c = 0;

  if (Serial.available() > 0)
      {
      c = Serial.read();        // Lee un caracter
      //Serial.println(c);
      }  
      Serial.flush(); 

  return (c);
}

/*===========================================================================*/
/*=======================    FUNCION ACTUALIZACION LCD=======================*/
/*===========================================================================*/
void actualiza_lcd(void)
{
  lcd.setCursor(0,1);
  lcd.print("   ");
  lcd.setCursor(0,1);
  if (estado_horno == true)
    {
    lcd.print("ON");
    }
  else
    {
     lcd.print("OFF"); 
    }


  lcd.setCursor(4,1);
  lcd.print("   ");
  lcd.setCursor(4,1);
  lcd.print(T_SET);
  

  lcd.setCursor(8,1);
  lcd.print("   ");
  lcd.setCursor(8,1);
  lcd.print(power);

  lcd.setCursor(13,1);
  lcd.print("   ");
  lcd.setCursor(13,1);
  lcd.print(T_OVEN);

  return;
}

void setup() 
{
  SerialDebug.begin(9600);
  Serial.begin(9600);
  pinMode(SSR,OUTPUT);
  pinMode(CPU,OUTPUT);
  digitalWrite(CPU,LOW);

  lcd.begin(16,2);
  lcd.backlight();
  lcd.clear();
  lcd.print("Pwr Set Pwm Temp");  

//Valor máximo del Timer es 8.3 Segundos
Timer1.initialize(1000000);          //Configura el TIMER en 1 Segundos
Timer1.attachInterrupt(SampleTime) ; //Configura la interrupción del Timer 1

//*************************************************************************//
//*****************   SINTONIA POR ZIEGLER y NICHOLS    *******************//
//*************************************************************************//
kp = (1.2*tau)/(k*theta); // Calculo propocional
ti = 2.0*theta;           // Calculo integral
td = 0.5*theta;           // Calculo derivativo

//*************************************************************************//
//*****************           PID DIGITAL               *******************//
//*************************************************************************//
// Calculo de control PID digital
q0 =  kp*(1+Ts/(2.0*ti)+td/Ts);
q1 = -kp*(1-Ts/(2.0*ti)+(2.0*td)/Ts);
q2 = (kp*td)/Ts;
}



void loop() 
{ 
  comando = lee_serie();

  
/// Enciende el horno (E) /////////////////////////////////////
  if (comando == 65)
    {
    analogWrite(SSR,0);
    estado_horno = false;
    debug = false;
    Serial.println("Apagado");
    }

/// Apaga el horno (A) ////////////////////////////////////////
  if (comando == 69)
    {
    estado_horno = true;
    Serial.println("Encendido");
    }

/// Activa debug (R) /////////////////////////////////////////
  if (comando == 82)
    {
    Serial.println("Debug");
    debug = true;
    }

/// Desactiva debug (W) /////////////////////////////////////
  if (comando == 87)
    {
    debug = false;
    estado_horno = false;
    }

/// Envía ajustes (V) ///////////////////////////////////////
  if (comando == 86)
    {
    Serial.print("Kp=");
    Serial.print(kp);
    Serial.print("  ti=");
    Serial.print(ti);
    Serial.print("  td=");
    Serial.println(td);
    }

/// Ajusta valor Proporcional ///////////////////////////////
  if (comando == 80)
    {
    Serial.print("Proporcional - ");
    Serial.println(valor);
    }

/// Ajusta valor Proporcional ///////////////////////////////
  if (comando == 73)
    {
    Serial.println("Integral");
    }

/// Ajusta valor Proporcional ///////////////////////////////
  if (comando == 68)
    {
    Serial.println("Derivada");
    }
    
  if (aux)
   {
    actualiza_lcd();

    if (debug == true)
      {
      estado_horno = true;
      Serial.print(T_SET);
      Serial.print(",");
      Serial.print(power); 
      Serial.print(",");
      Serial.println(T_OVEN);
      }
    aux = false;
   }  
  
}

