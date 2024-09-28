#define ARDUINOJSON_DECODE_UNICODE 1
#include <Michom.h>
#include <OLEDModule.h>
#include <GyverOLED.h>
#include <GyverButton.h>

#define BTN_PIN 12

char* id = "OLEDModule";
double VersionUploader = 1.12;
/////////настройки//////////////

GyverOLED<SSD1306_128x32, OLED_BUFFER> oled;
Michome michome(id, OLED, VersionUploader);
OLEDModule MichomeOLED(&michome);

ESP8266WebServer& server1 = michome.GetServer();
RTOS DateTimer(999);
RTOS WIFIIcon(3000);

GButton butt1(BTN_PIN);

bool isAbort = false;
String TextPage = "";

CreateMichome;

void setup(void) 
{
    michome.preInit();
    butt1.setDebounce(50);
    butt1.setTimeout(300);
    butt1.setClickTimeout(600);
            
    oled.init();        // инициализация
    //oled.setContrast(255);
    oled.clear();       // очистка
    oled.setScale(1);   // масштаб текста (1..4)
    oled.home();        // курсор в 0,0
    oled.fastLineH(10, 0, 127);
    oled.setCursor(0, 0);
    oled.print(F("Michome "));
    oled.print(michome.GetModule(0));
    oled.setCursor(0, 2);
    oled.print(F("Start module..."));
    
    if(!michome.IsSaveMode)
      updateData();
    else{
      oled.setCursor(0, 3);
      oled.print(F("Safe mode"));
      oled.update();
    }

    MichomeOLED.UpdatePageData = updatePage;
    MichomeOLED.UpdateData = updateData;
    MichomeOLED.UpdateLight = updateLight;
    MichomeOLED.init();
    michome.init(true);
    
    if(michome.IsSaveMode)
      return;

    /*MichomeOLED.AddMenu((MenuData){0, 0, "Главное меню", ""});
    MichomeOLED.AddMenu((MenuData){1, 0, "Пункт 1", "Значение пункта 1"});
    MichomeOLED.AddMenu((MenuData){2, 0, "Пункт 2", ""});
    MichomeOLED.AddMenu((MenuData){3, 2, "Пункт 2.1", ""});
    MichomeOLED.AddMenu((MenuData){4, 2, "Пункт 2.2", "Значение пункта 2.2"});
    MichomeOLED.AddMenu((MenuData){5, 3, "Пункт 2.1.1", "Значение пункта 2.1.1"});
    MichomeOLED.AddMenu((MenuData){6, 3, "Пункт 2.1.2", "Значение пункта 2.1.2"});
    */
    MichomeOLED.startUpdate();

    Module.addButton(0, BTN_PIN, ButtonUDP(michome));
}

void loop(void)
{
    michome.running();
    MichomeOLED.running();       
    updateService();
    if(michome.IsSaveMode)
      return;        
}

void updatePage(PageData page){
  TextPage = page.PageValue;
  oled.autoPrintln(false);
  oled.clear(0, 17, 129, 32);
  oled.setCursor(70, 0);
  oled.print(MichomeOLED.CurrentPage);
  drawIcon7x7(7); 
  oled.setCursor(0, 2);
  oled.print(page.NamePage); 
}

void updateData(){
  if(MichomeOLED.isMenu){

    
  }
  else{
    if(TextPage != ""){
      if(TextPage.length() > 22){     
        for(int i = 0; abs(i) < abs(128 - (TextPage.length() * 3.8)); i--){
          oled.setCursor(i, 3);
          oled.print(TextPage);
          updateService();
          if(isAbort){
            isAbort = false;
            break;
          }
          delay(2);
          oled.update();
          michome.yieldM();      
        }
      }
      else{
        oled.setCursor(0, 3);
        oled.print(TextPage);
      }
    }
  }
  oled.update();
}

void updateLight(bool state, uint8_t br){
  oled.setPower(state);
  oled.setContrast(br);
}

void updateService(){
  if(michome.IsSaveMode)
    return;
    
  butt1.tick();
  if (butt1.hasClicks()){
    byte clicks = butt1.getClicks();
    if(clicks == 1){
      MichomeOLED.ChangePage();
      isAbort = true; 
    }
    else if(clicks == 2){
      MichomeOLED.isMenu = !MichomeOLED.isMenu;
      isAbort = true; 
    }
    else if(clicks > 2){
      michome.GetUDP().SendTrigger((String)michome.GetModule(0)+"_Button", String(clicks));    
      michome.SendData(michome.ParseJson(F("get_button_press"), String(BTN_PIN)+"="+String(clicks)));
    }
  }
  
  if(butt1.isHold()){
    oled.setCursor(100, 0);
    drawIcon7x7(9);
    oled.update();
    MichomeOLED.startUpdate();
  }
      
  if(DateTimer.IsTick()){
    oled.clear(0, 0, 117, 9);
    oled.setCursor(0, 0);
    drawIcon8x8(5);
    oled.setCursor(10, 0);
    oled.print(michome.GetFormattedTime());
    oled.setCursor(70, 0);
    oled.print(MichomeOLED.CurrentPage);
    drawIcon7x7(7); 
    oled.setCursor(100, 0);
    if(!MichomeOLED.GetEtherFail()){      
      drawIcon7x7(6);
    }
    else{
      oled.print(MichomeOLED.GetAttempted());
      drawIcon7x7(8);
    }
  }
  if(WIFIIcon.IsTick()){
    oled.clear(118, 0, 129, 9);
    WIFISignal rssi = michome.GetSignal();
    oled.setCursor(118, 0);
    if(rssi == HighSignal)
      drawIcon8x8(0);
    else if(rssi == MidHSignal)
      drawIcon8x8(1);
    else if(rssi == MidSSignal)
      drawIcon8x8(2);
    else if(rssi == LowSignal)
      drawIcon8x8(3);
    else if(rssi == NoSignal)
      drawIcon8x8(4);
  } 
}

void drawIcon7x7(byte index) {
  for(unsigned int i = 0; i < 7; i++) {
    oled.drawByte(pgm_read_byte(&(OLED_Icons7x7[index][i])));
  }
}

void drawIcon8x8(byte index) {
  for(unsigned int i = 0; i < 8; i++) {
    oled.drawByte(pgm_read_byte(&(OLED_Icons8x8[index][i])));
  }
}
