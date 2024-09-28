#define EnableINA

#include <Michom.h>
#include <PowerMonitors.h>

#ifdef EnableINA
  #include <GyverINA.h>
#endif

#ifndef EnableExternalUnits
  #error This firmware work only with EnableExternalUnits
#endif

char* id = "PowerMonitor";
const char* type = PowerMods;
double Version = 1.01;
/////////настройки//////////////

Michome michome(id, type, Version);
ExternalUnits& EU = michome.GetExternalUnits();
PowerMonitorModules powers(&michome);

CreateMichome;

#ifdef EnableINA
  #define INA226_ADDRESS_START 0x40
  #define INA226_ADDRESS_END 0x4F
  #define SHUNT 0.004f
  #define MAXCURRENT 20.0000000f
  #define MAX_INA 16
  #define IS_INA(id) (id >= 0 && id < MAX_INA)
  const uint8_t INA_addr[MAX_INA] = {INA226_ADDRESS_START, INA226_ADDRESS_START+1, INA226_ADDRESS_START+2, INA226_ADDRESS_START+3, INA226_ADDRESS_START+4, INA226_ADDRESS_START+5, INA226_ADDRESS_START+6, INA226_ADDRESS_START+7, INA226_ADDRESS_START+8, INA226_ADDRESS_START+9, INA226_ADDRESS_START+10, INA226_ADDRESS_START+11, INA226_ADDRESS_START+12, INA226_ADDRESS_START+13, INA226_ADDRESS_START+14, INA226_ADDRESS_END};
  INA226 ina[MAX_INA] = {INA226(SHUNT, MAXCURRENT, INA_addr[0]), INA226(SHUNT, MAXCURRENT, INA_addr[1]), INA226(INA_addr[2]), INA226(INA_addr[3]), INA226(INA_addr[4]), INA226(INA_addr[5]), INA226(INA_addr[6]), INA226(INA_addr[7]), INA226(INA_addr[8]), INA226(INA_addr[9]), INA226(INA_addr[10]), INA226(INA_addr[11]), INA226(INA_addr[12]), INA226(INA_addr[13]), INA226(INA_addr[14]), INA226(INA_addr[15])};
#endif

uint8_t CountPowers = 0;

void setup ( void ) {
  
  powers.GetPowerMonitorHandler(GetPower);

  powers.GetResetMonitorHandler([](uint8_t idPowers) 
  {
    return true;  
  });

  powers.GetInitMonitorHandler([](uint8_t idPowers) 
  {
    bool tmp = false;
    #ifdef EnableINA
    if(IS_INA(idPowers) && EU.isWIREFound(INA226_ADDRESS_START + idPowers)){
        tmp = ina[idPowers].begin();
        ina[idPowers].setSampleTime(INA226_VBUS, INA226_CONV_2116US);   // Повысим время выборки напряжения вдвое
        ina[idPowers].setSampleTime(INA226_VSHUNT, INA226_CONV_8244US); // Повысим время выборки тока в 8 раз
        ina[idPowers].setAveraging(INA226_AVG_X4);
    }   
    #endif
    return tmp;  
  });

  #ifdef EnableINA
    CountPowers = CountPowers + MAX_INA;
  #endif

  powers.SetCountPowerMonitors(CountPowers);

  michome.SetRefreshData([](){powers.SendGateway();});
 
  michome.init(false);
  if(michome.IsSaveMode) return;
  
  powers.init();
  
  michome.Refresh();
}

void loop ( void ) {
  michome.running();
  
  if(michome.IsSaveMode) return;
  powers.running();
  
  if(michome.GetSettingRead()){
    powers.ChangeTime(michome.GetSettingToInt("update", 600000));
    powers.AlarmMinimumVoltage = michome.GetSettingToInt("minvoltage", 3);
    powers.AlarmFastVoltage = michome.GetSettingToInt("fastvoltage", 5);
  } 
}

PowerMonitorData GetPower(uint8_t idPower){
  PowerMonitorData tmp = {0,0,0};
  #ifdef EnableINA
  if(IS_INA(idPower) && powers.EnablePowers[idPower]){
    tmp.Voltage = ina[idPower].getVoltage();
    if(tmp.Voltage == 0){
      delay(50);
      tmp.Voltage = ina[idPower].getVoltage();
    }
    tmp.Current = ina[idPower].getCurrent();
    tmp.Power = ina[idPower].getPower();
  }
  #endif
  return tmp;
}
