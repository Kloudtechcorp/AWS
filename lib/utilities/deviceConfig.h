#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

extern String deviceSerial;
extern String stationName;
// Station Serial Lists
#ifdef DEMO_MARIA_STATION
  String deviceSerial = "PUSI-CJ6Y-74JL-P5YE";
  String stationName = "Demo Station - Maria";
#endif
#ifdef SABANG_STATION
  String deviceSerial = "6TD0-5YIQ-8QQ0-JRMO";
  String stationName = "Sabang Fish Landing";
#endif
#ifdef BAGAC_STATION
  String deviceSerial = "OBY1-LFE0-5ADX-0U8Z";
  String stationName = "BPSU Bagac Campus";
#endif
#ifdef MARIVELES_STATION
  String deviceSerial = "9ME5-MGTZ-79D1-0JPE";
  String stationName = "Mariveles Municipal Hall";
#endif
#ifdef LIMAY_STATION
  String deviceSerial = "DZMJ-ERI3-DIZV-3M2X";
  String stationName = "Limay Physical Therapy Center";
#endif
#ifdef COMMAND_CENTER_STATION
  String deviceSerial = "MHY3-ZOIV-OA2O-Q0ZT";
  String stationName = "One Bataan Command Center";
#endif
#ifdef HERMOSA_STATION
  String deviceSerial = "VREC-R90V-V7MD-KBVO";
  String stationName = "Hermosa Municipal Hall";
#endif
#ifdef CABCABEN_STATION
  String deviceSerial = "SKGN-9ELZ-B4A7-1VF2";
  String stationName = "Old Cabcaben Pier";
#endif
#ifdef QUINAWAN_STATION
  String deviceSerial = "BZIH-W8DI-62JP-LAH2";
  String stationName = "Quinawan Integrated School";
#endif
#ifdef KANAWAN_STATION
  String deviceSerial = "U7KD-U1ZF-DD30-2H83";
  String stationName = "Kanawan Integrated School";
#endif
#ifdef TANATO_STATION
  String deviceSerial = "QHVM-L9AR-LP84-JB51";
  String stationName = "Tanato Elementary School";
#endif
#ifdef DINALUPIHAN_STATION
  String deviceSerial = "BQ0V-STFB-IGPX-QSJ4";
  String stationName = "BPSU Dinalupihan Campus";
#endif
#ifdef PAGASA_STATION
  String deviceSerial = "8TMY-CICW-5EPQ-1DXX";
  String stationName = "Pag-asa Elementary School";
#endif
#ifdef DEMO_GLENN_STATION
  String deviceSerial = "EDIG-GEKZ-WQSS-JNY7";
  String stationName = "Demo Station - Glenn";
#endif

// Choose whether to use GSM or WiFi network
#ifdef USE_GSM
  #include "GSM_comm.h"
#else 
  #include "WiFi_comm.h"
#endif

// APN Options
#ifdef SMART_APN
  extern const char deviceApn[] = "smartlte";
#endif
#ifdef GLOBE_APN
  extern const char deviceApn[] = "http.globe.com.ph";
#endif
#ifdef GOMO_APN
  extern const char deviceApn[] = "gomo.ph";
#endif

// WiFi Options
#ifdef WIFI_1
  extern const char* ssid = "KT 2.4";
  extern const char* password = "J@yGsumm!t";
#endif

#endif // DEVICE_CONFIG_H
