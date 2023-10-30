#include <config.h>

// /******************************************************************************************
//  * LECTURA NTC
//  * ****************************************************************************************/
// int leeNtc(void)
// {
//   Vo = analogRead(MAX_CS2);			// lectura de A0
//   R2 = R1 * (4095.0 / (float)Vo - 1.0);	// conversion de tension a resistencia
//   logR2 = log(R2);			// logaritmo de R2 necesario para ecuacion
//   tempNTC = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)); 	// ecuacion S-H
//   tempNTC = tempNTC - 273.15;   // Kelvin a Centigrados (Celsius)
//   Serial.print(R2);
//   Serial.print(" - ");
//   Serial.println(tempNTC);
//    return tempNTC;
// }

/******************************************************************************************
 * PASO POR CERO TENSION DE RED
 * ****************************************************************************************/
void IRAM_ATTR pasoPorcero(void)
{
  static int contador = 0; 
  static int long retardoZC = 0;

  contador ++;

  if ( contador > 1)
  {
    zero = millis() - retardoZC;
    contador = 0;
    retardoZC = 0;
  }
  else
  {
    retardoZC = millis();
  }
}
 
/******************************************************************************************
 * CHEQUEA GIRO Y PULSACION ENCODER
 * ****************************************************************************************/
void IRAM_ATTR encoder(void)
{
  static bool SW_anterior = true;     // Valor anterior pulsador ( Static mantiene valor )
  static bool CK_anterior = false;    // Valor anterior señal CK encoder ( Static mantiene valor )

  static unsigned long retardoCK = 0;	// Variable estatica ultimo valor tiempo de interrupcion CK
  static unsigned long retardoSW = 0;	// Variable estatica ultimo valor tiempo de interrupcion SW

  bool CK_actual = digitalRead(CK_ENCODER);  // Lee valor Reloj encoder actual
  bool SW_actual = digitalRead(SW_ENCODER);  // Lee valor Pulsador actual
  bool DT_actual = digitalRead(DT_ENCODER);  // Lee valor Datos encoder actual

  if (SW_actual != SW_anterior)              // Si se pulso el pulsador
  {
    if (millis() - retardoSW > DELAY_SW)     // Espera retardo para rebotes
    {
      estadoPulsador = true;                 // Estado pulsador "Pulsado"
      retardoSW = millis();                  // Captura tiempo actual
    }
    SW_anterior = SW_actual;                 // Establece estado anterior al actual
  }

  if (CK_actual != CK_anterior)             // Si se giro el encoder
  {
    if (millis() - retardoCK > DELAY_CK)    // Espera retardo para rebotes
    {     
      if (CK_actual == DT_actual)           // Si señal reloj es igual a datos
        {
          posActualEncoder ++;              // Giro encoder sentido horario suma posicion
          if (posActualEncoder > posFinalEncoder) posActualEncoder = posFinalEncoder;
        }
      else                                  // Señal reloj distinta a datos
        { 
          posActualEncoder --;              // Giro encoder contrario reloj resta posicion  
          if (posActualEncoder < 0) posActualEncoder = 0;
        }
      retardoCK = millis();                // Captura valor de tiempo
    }
  CK_anterior = CK_actual;                 // Establece estado anterior encoder al actual
  } 
}
 
/******************************************************************************************
 * ENCIENDE LED ENCODER
 * ****************************************************************************************/
void ledEncoder(int _red, int _green, int _blue)
{
  for (int i=0; i<NUM_LEDS_ENCODER; i++)
  {   
   leds.setPixelColor(i,_red,_green,_blue);
  }
  leds.show();
}
 
/******************************************************************************************
 * CALCULO PWM RESISTENCIA CALEFACTORA Y VENTILADOR
 * ****************************************************************************************/
void ajustaPwm(int _idFase)
{
  float error;    // Error tempertura - consigna
  float errorInicial;

  int _pwm_ssr;   // Valor PWM ssr 0 -255
  int _pwm_fan;   // Valor PWM fan 0 -255

  pwm_fan  = 0;   // Valor PWM FAN 0 - 100 %
  pwm_ssr  = 0;   // Valor PWM SSR 0 - 100 %
  _pwm_ssr = 0;
  _pwm_fan = 0;
  error    = 0;
  
  if (estadoHorno)      // Si estado del horno es verdadero
  {
     if (_idFase == 4)
      {
      //   incrementoTemp = (fase[_idFase-1].temperatura - fase[_idFase].temperatura) / fase[_idFase].tiempo;
      //   error = (temperatura - tempConsigna) * fase[_idFase].k_fan; 
      //   if (error > 100) error = 100;
      //   if (error < 0) error = 0;
      //   pwm_fan = error;
      //  _pwm_fan = map(pwm_fan,0,100,0,255);    
      pwm_fan = 100;  
      _pwm_fan = 255;          
      }

     else
      {
        errorInicial = (fase[_idFase].temperatura - fase[_idFase-1].temperatura);
        incrementoTemp = (fase[_idFase].temperatura - fase[_idFase-1].temperatura) / fase[_idFase].tiempo;
        error = (tempConsigna - temperatura) * fase[_idFase].k_pwm; 
        _progreso = temperatura;
        progreso = map(_progreso,fase[_idFase].temperatura,fase[_idFase-1].temperatura,100,0);

        if (error > 100) error = 100;
        if (error < 0) error = 0;
        pwm_ssr = error;
        _pwm_ssr = map(pwm_ssr,0,100,0,255);
      }
  }    

  ledcWrite(PWM_FAN_CHANNEL, _pwm_fan);  // Carga valor pwm control ventilador
  ledcWrite(PWM_SSR_CHANNEL, _pwm_ssr);  // Carga valor pwm control resistencia 

  tft.setTextFont(FONT2);                               // Tamaño texto función elegida  
  tft.setTextColor(TFT_CYAN,TFT_BLACK);                 // Color texto función elegida
  tft.setCursor(30,4);
  tft.print("K2 "); 
  tft.print(int(temperatura_2));                                  // Valor PWM 0 - 255
  tft.setCursor(100,4);
  tft.print("Enc ");
  tft.print(posActualEncoder); 
  tft.setCursor(160,4);                                 // Posición texto función elegida
  tft.print(tempConsigna);                              // Temperatura consigna
  tft.print(" C");   
  tft.setCursor(240,4);  
  tft.print(zero);                            // Incremento temperatura
  tft.print("ms  ");   
      
  
}

/******************************************************************************************
 * PLANTILLA PANTALLA
 * ****************************************************************************************/
void plantillaPantalla(void)
{
  int x     = 120;                              // Valor de coordenada x texto temperatura                           
  static bool flag = false;                     // Flag temperatura mayor de 100
  uint32_t color_ssr;                           // Color estado resistencia
  uint32_t color_fan;                           // Color estado ventilador
  uint32_t color_barra_ssr;
  uint32_t color_barra_fan;
  int tempPantalla = temperatura;
   
  /********************  CABECERA PANTALLA REFLOW *****************************************/
  tft.drawLine(230,0,230,27,TFT_WHITE);                 // Línea horizontal
  tft.drawLine(0,27,320,27,TFT_WHITE);                  // Línea vertical
  tft.drawBitmap(296,1,wifi_ico_24,24,24,TFT_DARKGREY); // Icono Wifi
  
  /***************  CAJAS PWM RESISTENCIA && VETILADOR ************************************/
  tft.drawRoundRect(5,35,42,204,4,TFT_ORANGE);         // Recuadro control resitencia
  tft.drawRoundRect(274,35,42,204,4,TFT_BLUE);         // Recuadro control ventilador

  /***************  TEXTO GRADOS TEMPERATURA **********************************************/
  tft.setTextFont(FONT4);                              // Tamaño texto "C" grados
  tft.setTextColor(TFT_DARKGREY);                      // Color texto "C" grados
  tft.setCursor(240,50);                               // Posición "C" grados 
  tft.print("C");                                      // Texto
  tft.setTextFont(FONT2);                              // Tamaño texto "C" grados
  tft.setCursor(230,45);                               // Posición simbolo grados 
  tft.print("o");                                      // muestra simbolo

  /***************  MUESTRA TEMPERATURA ***************************************************/

  if (error) tempPantalla = 000;               // !!!!! CAMBIAR POR PANTALLA DE ERROR SONDAS
  if (tempPantalla> 99)                         // Si temperatura es mayor de 100
    {                                           // establece la coordenada x en 70
      x = 65;
      flag = true;
    }
  if (tempPantalla < 100 && flag)                // Si temperatura es menor de 100
    {
      tft.fillRect(70,50,50,80,TFT_BLACK);      // Borra el digito de las centenas
      flag = false;                             // y no vuelvas a borrarlo
    }

  tft.setTextFont(FONT8);                       // Tamaño del texto temperatura
  tft.setTextColor(TFT_WHITE,TFT_BLACK);        // Color texto temperatura
  tft.setCursor(x,50);                          // Posición texto temperatura
  tft.print(tempPantalla);                     // Muestra temperatura sin decimales

  /***************  DIBUJA ICONOS RESISTENCIA Y VENTILADOR ****************************/
  if(pwm_ssr > 0)               
    {
      color_ssr = TFT_ORANGE;   // Si pwm resistencia es mayor que cero icono naranja
    }
  else
    {
      color_ssr = TFT_DARKGREY; // Si pwm resistencia es menor que cero icono gris
    }  
  if(pwm_fan > 0)
    {
      color_fan = TFT_BLUE;     // 
    }
  else
    {
      color_fan = TFT_DARKGREY;
    }    
  tft.drawBitmap(8,202,heat_ico_36,36,36,color_ssr);   // Dibuja icono resistencia
  tft.drawBitmap(276,202,fan_ico_36,36,36,color_fan);  // Dibuja icono ventilador

  /***************  DIBUJA VALORES PMW SSR *******************************************/
  if (pwm_ssr < 10) 
    {
      tft.fillRect(18,184,16,14,TFT_BLACK);  // Borra decenas y unidades
    }
  if (pwm_ssr < 100) 
    {
      tft.fillRect(26,184,8,14,TFT_BLACK);  // Borra el digito de las unidades
    }
  tft.setTextFont(FONT2);                   // Tamaño texto porcentaje PWM resistencia
  tft.setTextColor(TFT_ORANGE,TFT_BLACK);   // Color texto porcentaje PWM resitencia
  tft.setCursor(10,184);                    // Posición texto PWM resistencia
  tft.print(pwm_ssr);                       // Muestra valor
  tft.setCursor(35,184);                    // Posición texto unidad
  tft.print("%");                           // Muestra unidad de medida

  /***************  DIBUJA VALORES PMW FAN ********************************************/
  if (pwm_fan < 10) 
    {
      tft.fillRect(286,184,16,14,TFT_BLACK); // Borra decenas y unidades
    }
  if (pwm_fan < 100) 
    {
      tft.fillRect(294,184,8,14,TFT_BLACK); // Borra el digito de las unidades
    }
  tft.setTextFont(FONT2);                   // Tamaño texto porcentaje PWM ventilador
  tft.setTextColor(TFT_BLUE,TFT_BLACK);     // Color texto porcentaje PWM ventilador
  tft.setCursor(278,184);                   // Posición texto PWM vetilador
  tft.print(pwm_fan);                       // Muestra valor
  tft.setCursor(303,184);                   // Posición texto unidad
  tft.print("%");                           // Muestra unidad

/***************  ACTUALIZA BARRAS PWM ***********************************************/
  for (int i=1; i<21; i++)
    {
      int y = i * 7;     
      color_barra_ssr = TFT_BLACK;
      color_barra_fan = TFT_BLACK;

      int led_ssr = pwm_ssr / 5;
      int led_fan = pwm_fan / 5;

      if (i > (20 - led_ssr)) color_barra_ssr = TFT_ORANGE;
     
      if (i > (20 - led_fan)) color_barra_fan = TFT_BLUE;
      

      tft.fillSmoothRoundRect(14,35 + y,24,3,2,color_barra_ssr,TFT_BLACK);
      tft.fillSmoothRoundRect(283,35 + y,24,3,2,color_barra_fan,TFT_BLACK);
    }
}

/******************************************************************************************
 * FUNCION REFLOW
 * ****************************************************************************************/
void reflow(void)
{
  static int  tiempo         = 0;                // Tiempo transcurrido en segundos
  static int  idFase         = 0;                // Numero de fase inicial
  static long tiempoAnterior = 0;                // Tiempo anterior para muestreo temperatura
  int         minutos;                           // Minutos transcurridos
  int         segundos;                          // Segundos transcurridos  
  bool        flagSegundo    = false;            // Bandera incremento de temperatura consigna
    
  fase[0] = READY;     
  fase[1] = PREHEAT;     
  fase[2] = SOAK;
  fase[3] = REFLOW;
  fase[4] = COOLING;
  fase[5] = DONE;

  if (millis() - tiempoAnterior > TIME_SAMPLE)  // Si pasó tiempo de muestreo 
    {
      flagSegundo = true;                       // Activa flag para incremento de temperatura
      temperatura = termopar1.readCelsius();    // Lee temperatura
      temperatura_2 = termopar2.readCelsius();
      digitalWrite(LED_CPU,!digitalRead(LED_CPU));
      
      if (temperatura > 400) error = true;      // Comprueba que los termopares estan bien
      else error = false;
      tiempoAnterior = millis();                // Captura valor actual de tiempo
      if (estadoHorno) 
        {
          tiempo++;                             // Inicia tiempo si se activo el proceso
          Serial.print(temperatura);            // Envía temperatura al puerto serie
          Serial.print(",");
          Serial.println(tempConsigna);        // Envía valor PWM SSR
        }
    }
  
  if (estadoPulsador)                           // Si se pulsó inicia reflow
    {
      estadoPulsador = false;                   // Resetea estado del pulsador    
      idFase = 1;                               // Establece fase inicial
      estadoHorno = !estadoHorno;               // Inicia/Para proceso reflow
      tempConsigna = temperatura;               // Captura valor de la temperatura actual
      tft.fillRect(65,155,125,20,TFT_BLACK);    // Borra el texto de tiempo
    }

   if (estadoHorno)
      {     
      if (idFase != 4)
        {        
          if (flagSegundo)
            {
              flagSegundo = false;
              tempConsigna = tempConsigna + incrementoTemp;
              if (tempConsigna > fase[idFase].temperatura) tempConsigna = fase[idFase].temperatura;               
            }
          if (temperatura >= fase[idFase].temperatura)  // Si se supero o igualo temperatura fase
            {
              idFase++;                                 // Avanza a la siguiente fase
              tft.fillRect(65,155,125,20,TFT_BLACK);    // Borra texto fase
            }
        }
      else
        {
         if (temperatura < fase[idFase].temperatura) 
            {
              idFase++;  
              tft.fillRect(65,155,125,20,TFT_BLACK);
            }
        }  
      if (idFase == 5)                             // Si se cumplio la fase final
        {
          estadoHorno = false;                    // Desactiva reflow
        }   
     }

  else
    {
      tiempo  = 0;
      estadoHorno = false;
      if (idFase != 5) idFase = 0;
      if (temperatura < TEMPERATURA_SEGURA && idFase == 5) idFase = 0;      
    }   

    ajustaPwm(idFase);   

    plantillaPantalla(); 
    
    /********************  CABECERA PANTALLA REFLOW ******************************************/
    tft.drawBitmap(1,1,heat_ico_24,24,24,TFT_DARKGREY);  // Icono función elegida
    tft.setTextFont(FONT2);                              // Tamaño texto función elegida  
    tft.setTextColor(TFT_DARKGREY);                      // Color texto función elegida
    tft.setCursor(30,4);                                 // Posición texto función elegida
    //tft.print("REFLOW  SMD291AX10");                   // Texto a mostrar
  
    /***************  MUESTRA FASE **********************************************************/
    tft.setTextFont(FONT4);                              // Tamaño del texto de la fase actual
    tft.setTextColor(fase[idFase].color,TFT_BLACK);      // Color del texto de la fase actual
    tft.setCursor(55,155);                               // Posición del texto de la fase actual
    tft.print(fase[idFase].nombre);                      // Muestra el texto

    /***************  MUESTRA TIEMPO ********************************************************/
    if (estadoHorno)
      {
        minutos  = tiempo / 60;                       // Calcula minutos transcurridos
        segundos = tiempo % 60;                       // Calcula segundos transcurridos

        tft.setTextFont(FONT4);                       // Tamaño del texto tiempo transcurrido
        tft.setTextColor(TFT_WHITE,TFT_BLACK);        // Color del texto tiempo transcurrido
        tft.setCursor(185,155);                       // Posición minutos tiempo transcurrido
        if (minutos < 10)                             // Si el tiempo es mayor a 60 segundos
          {
            tft.print("0");                           // añadele un cero delante a los minutos 
          }
        tft.print(minutos);                           // muestra minutos
        tft.setCursor(230,155);                       // Posición swegundos tiempo transcurrido                          
        if(segundos < 10)                             // Si segundos es menor a 10 
          {
            tft.print("0");                           // añadele un cero delante a los segundos  
          }
        tft.print(segundos);                          // muestra segundos 
        tft.setTextFont(FONT2);                       // Tamaño texto minutos, segundos
        tft.setCursor(217,161);                       // posicion miniatura minutos (m)
        tft.print("m ");                              // muestra miniatura (m)
        tft.setCursor(262,161);                       // posicion miniatura segundos   
        tft.print("s ");                              // muestra miniatura (s)        
      }
    else
      {
        tft.fillRect(180,155,90,20,TFT_BLACK);        // Borra tiempo si el hot-plate esta parado
      }
  
    /***************  DIBUJA ICONOS OPCIONES ************************************************/
    tft.drawBitmap(75,202,return_ico_36,36,36,TFT_DARKGREY);
    if (estadoHorno) 
      {
        tft.drawBitmap(142,202,stop_ico_36,36,36,TFT_RED,TFT_BLACK); 
      }
    else 
      {
        tft.drawBitmap(142,202,play_ico_36,36,36,TFT_DARKGREEN,TFT_BLACK);      
      }
    tft.drawBitmap(209,202,graph_ico_36,36,36,TFT_DARKGREY);

    /********************  ENCIENDE LEDS ENCODER ********************************************/
    ledEncoder(fase[idFase].R,fase[idFase].G,fase[idFase].B);
}

 
/******************************************************************************************
 * FUNCION SETUP
 * ****************************************************************************************/
void setup(void)   
{
  Serial.begin(9600);

  pinMode(CK_ENCODER,INPUT);      // Establece como entrada CK encoder
  pinMode(DT_ENCODER,INPUT);      // Establece como entrada DT encoder
  pinMode(SW_ENCODER,INPUT);      // Establece como entrada SW encoder
  pinMode(ZERO_CROSSING,INPUT);   // Establece como entrada paso por cero AC
  pinMode(LED_CPU,OUTPUT);        // Led CPU

  attachInterrupt(SW_ENCODER, encoder, CHANGE);         // Activa interrupcion pulsador encoder
  attachInterrupt(CK_ENCODER, encoder, CHANGE);         // Activa interrupcion giro encoder
  attachInterrupt(ZERO_CROSSING, pasoPorcero, RISING);  // Activa interrupcion paso por cero AC  
  
  ledcSetup(PWM_BUZZER_CHANNEL, PWM_BUZZER_FREQUENCY, PWM_BUZZER_RESOLUTION); // Configuración PWM SSR    
  ledcSetup(PWM_SSR_CHANNEL, PWM_SSR_FREQUENCY, PWM_SSR_RESOLUTION);          // Configuración PWM SSR 
  ledcSetup(PWM_FAN_CHANNEL, PWM_FAN_FREQUENCY, PWM_FAN_RESOLUTION);          // Configuración PWM FAN
  ledcAttachPin(BUZZER, PWM_BUZZER_CHANNEL); // Asignación pin PWM BUZZER   
  ledcAttachPin(SSR, PWM_SSR_CHANNEL);       // Asignación pin PWM SSR
  ledcAttachPin(FAN, PWM_FAN_CHANNEL);       // Asignación pin PWM FAN

  // Emite tono de inicio
  ledcWriteNote(PWM_BUZZER_CHANNEL,NOTE_B,4);
  delay(500);
  ledcWrite(PWM_BUZZER_CHANNEL,0);   

  // Configura MAX6675
  termopar1.begin(MAX_SCK, MAX_CS1, MAX_SO);   // Inicializa MAX6675-1
  termopar2.begin(MAX_SCK, MAX_CS2, MAX_SO);   // Inicializa MAX6675-2

  // Configura pantalla
  delay(250);                 // Espera estabilización pantalla
  tft.begin();                // Inicia pantalla
  tft.setRotation(1);         // Rota pantalla 180 grados 3 era el original
  tft.fillScreen(TFT_BLACK);  // Establece fondo patalla en negro  
 
  // Configura Led WS2812
  leds.begin();               // Inicializa leds  
  leds.setBrightness(30);     // Establece el brillo al 30%
  ledEncoder(0,0,255);        // Enciende los leds en azul
  delay(500);                 // Espera medio segundo
}

/******************************************************************************************
 * FUNCION LOOP
 ******************************************************************************************/
void loop(void) 
{   
  reflow();
} 

