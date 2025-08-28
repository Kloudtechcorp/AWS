# KT-Weather-Station

## Getting Started

### Cloning the Repository

If you haven't cloned the repository yet, you can do so using one of these methods:

**Option 1: Command Terminal (CMD/PowerShell/Git Bash)**
```bash
git clone https://github.com/Kloudtechcorp/AWS.git
cd AWS
```

**Option 2: VSCode Terminal**
1. Open VSCode
2. Open the integrated terminal (Ctrl+` or View → Terminal)
3. Run the clone command:
```bash
git clone https://github.com/Kloudtechcorp/AWS.git
cd AWS
```

**Option 3: VSCode Git Integration**
1. Open VSCode
2. Press `Ctrl+Shift+P` to open the command palette
3. Run `Git: Clone` command 
4. Enter the repository URL: `https://github.com/Kloudtechcorp/AWS.git`
5. Choose a local folder to clone into

### Pulling Latest Changes

To get the latest changes from the repository:

**Using Terminal:**
```bash
git pull origin main
```

**Using VSCode:**
1. Open the Source Control panel (Ctrl+Shift+G)
2. Click the "..." menu and select "Pull"

## Building

1. Install VSCode and the PlatformIO extension (`platformio.platformio-ide`).
2. Open the cloned project folder in VSCode.
3. Wait for PlatformIO to automatically install the dependencies from the `platformio.ini` file.
4. Run `PlatformIO: Build` to build the project.

*Note: When getting "Please configure IDF framework to include mbedTLS -> Enable pre-shared-key ciphersuites and activate at least one cipher" error when compiling, go to .pio/libdeps/esp32dev/SSLClient/src/ssl_client.cpp. In line 22, change the word error to warning.*

### Build Flags

#### Connectivity Flags

This flag determines the connectivity method for data transmission.

| Flag      | Description                                                                                                             |
| --------- | ----------------------------------------------------------------------------------------------------------------------- |
| `USE_GSM` | Enable GSM module support for remote data transmission. Otherwise, data is sent via Wi-Fi, also defining `WIFI_1` flag. |

#### APN Flags for GSM Module

This flag determines the APN settings for the GSM module. Only one APN flag should be defined at a time.

| Flag        | Description             |
| ----------- | ----------------------- |
| `SMART_APN` | Use SMART APN settings. |
| `GLOBE_APN` | Use GLOBE APN settings. |
| `GOMO_APN`  | Use GOMO APN settings.  |

#### Station Flags

This flag determines the device serial number and the station name.

| Flag                     | Description                   |
| ------------------------ | ----------------------------- |
| `DEMO_MARIA_STATION`     | Demo Station - Maria          |
| `SABANG_STATION`         | Sabang Fish Landing           |
| `BAGAC_STATION`          | BPSU Bagac Campus             |
| `MARIVELES_STATION`      | Mariveles Municipal Hall      |
| `LIMAY_STATION`          | Limay Physical Therapy Center |
| `COMMAND_CENTER_STATION` | One Bataan Command Center     |
| `HERMOSA_STATION`        | Hermosa Municipal Hall        |
| `CABCABEN_STATION`       | Old Cabcaben Pier             |
| `QUINAWAN_STATION`       | Quinawan Integrated School    |
| `KANAWAN_STATION`        | Kanawan Integrated School     |
| `TANATO_STATION`         | Tanato Elementary School      |
| `DINALUPIHAN_STATION`    | BPSU Dinalupihan Campus       |
| `PAGASA_STATION`         | Pag-asa Elementary School     |
| `DEMO_GLENN_STATION`     | Demo Station - Glenn          |
| `PTORIVAS_STATION`       | Pto. Rivas Fish Landing       |

## Uploading

1. Make sure the device is connected to the computer and detectable by PlatformIO.
2. Run `PlatformIO: Upload` or `PlatformIO: Upload and Monitor` in VSCode to upload the firmware to the device.

## Testing

For testing purposes, the resource path of POST request, `/api/{version}/weather/insert-data?serial={serial}`, can be modified in line 9 found in lib/utilities/BaseClient.h, where `{version}` is the API version (e.g., `v1`, `test`) and `{serial}` is the serial number of the station. For example,

```
const String RESOURCE_PATH_PREFIX = "/api/v1/weather/insert-data?serial=";
```

The test server dashboard can then be accessed at `test.kloudtechsea.com`, requiring a valid username and password to log in. The dashboard will display the data sent from the weather station.

