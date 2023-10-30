void menuopciones()
{
  static int giroAnterior = 1;  
  posFinalEncoder = 3;

  menu[0].nombre="Ajustes";
  menu[0].pos_x = 20;
  menu[0].pos_y = 91;

  menu[1].nombre="Reflow";
  menu[1].pos_x = 93;
  menu[1].pos_y = 91;

  menu[2].nombre="Desoldar";
  menu[2].pos_x = 166;
  menu[2].pos_y = 91;

  menu[3].nombre="Wifi";
  menu[3].pos_x = 239;
  menu[3].pos_y = 91;


  tft.drawBitmap(25,96,ajuste_ico_48,48,48,TFT_DARKGREY);
  tft.drawBitmap(98,96,reflow_ico_48,48,48,TFT_DARKGREY);
  tft.drawBitmap(171,96,desol_ico_48,48,48,TFT_DARKGREY);
  tft.drawBitmap(244,96,wifi_ico_48,48,48,TFT_DARKGREY);

  tft.drawRoundRect(menu[posActualEncoder].pos_x,menu[posActualEncoder].pos_y,58,58,6,TFT_WHITE);
  tft.drawRoundRect(menu[posAnteriorEncoder].pos_x,menu[posAnteriorEncoder].pos_y,58,58,6,TFT_BLACK); 

  if (posActualEncoder != giroAnterior)
    {
      giroAnterior = posActualEncoder;
      tft.setTextColor(TFT_DARKGREY,TFT_BLACK);
      tft.setTextFont(FONT4);
      tft.fillRect(90,180,130,20,TFT_BLACK);
      tft.setCursor(90,180);
      tft.print(menu[posActualEncoder].nombre);
    }


      tft.setTextColor(TFT_GREEN,TFT_BLACK);
      tft.setTextFont(FONT2);
      tft.setCursor(0,0);
      tft.print("Actual: ");
      tft.print(posActualEncoder);
      tft.print(" Anterior: ");
      tft.print(posAnteriorEncoder);
      tft.print(" Pulsador: ");
      tft.print(pulsadorEncoder());  

    Serial.println(pulsadorEncoder());

  if (pulsadorEncoder() && posActualEncoder == 1)  {tft.fillScreen(TFT_BLACK);}
}

 /********************************************************************************************
 * PANTALLA DE INICIO
 * ******************************************************************************************/
void pantallaInicio()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextFont(FONT4);
  tft.setCursor(40,90);
  tft.print("HOT-PLATE DV2819");
  tft.setCursor(70,120);
  tft.print("Prodoelec");
    tft.setTextFont(FONT2);
  tft.setTextColor(TFT_BLUE);
  tft.setCursor(60,150);
  tft.print("http://prodoelec.duckdns.org");
  delay(5000);
  return;  
}