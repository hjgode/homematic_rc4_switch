//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-08-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
#define USE_OTA_BOOTLOADER
#define NDEBUG
// This firmware converts a MAX! into a Homematic
#define HM_LC_SW1_PL 0x00,0x11

#include <AskSinPP.h>

#include <MultiChannelDevice.h>
#include <Switch.h>

// use MightyCore Standard pinout for ATmega32

#define CONFIG_BUTTON_PIN 24 // PA0
#define RELAY1_PIN 0 // PB0

// number of available peers per channel
#define PEERS_PER_CHANNEL 16

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x34,0x17,0x30},       // Device ID
    "HMax000000",           // Device Serial
    {HM_LC_SW1_PL},         // Device Model
    0x16,                   // Firmware Version
    as::DeviceType::Switch, // Device Type
    {0x01,0x00}             // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef AvrSPI<4,5,6,7> RadioSPI; // PB4-PB7
typedef StatusLed<12> LedType; // PD4
typedef AskSin<LedType,NoBattery,Radio<RadioSPI,11> > Hal;  // PD3

template<class HALTYPE,int PEERCOUNT>
class SwChannel : public SwitchChannel<HALTYPE,PEERCOUNT,List0> {
public:
  SwChannel () {};
  virtual ~SwChannel () {};

  virtual void switchState(__attribute__((unused)) uint8_t oldstate,uint8_t newstate,uint32_t delay) {
    // if ON - invert led so it will stay on after sending status
    this->device().led().invert(newstate == AS_CM_JT_ON);
    SwitchChannel<HALTYPE,PEERCOUNT,List0>::switchState(oldstate, newstate, delay);
  }
};

// setup the device with channel type and number of channels
typedef MultiChannelDevice<Hal,SwChannel<Hal,PEERS_PER_CHANNEL>,1> SwitchType;

Hal hal;
SwitchType sdev(devinfo,0x20);
ConfigToggleButton<SwitchType> cfgBtn(sdev);

void initPeerings (bool first) {
  // create internal peerings - CCU2 needs this
  if( first == true ) {
    HMID devid;
    sdev.getDeviceID(devid);
    for( uint8_t i=1; i<=sdev.channels(); ++i ) {
      Peer ipeer(devid,i);
      sdev.channel(i).peer(ipeer);
    }
  }
}

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  bool first = sdev.init(hal);
  sdev.channel(1).init(RELAY1_PIN,false);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  initPeerings(first);
  sdev.initDone();
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
//    hal.activity.savePower<Idle<>>(hal);
  }
}
