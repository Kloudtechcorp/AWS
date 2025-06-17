# KT-Weather-Station

## Building

1. Install VSCode and the PlatformIO extension (`platformio.platformio-ide`).
2. Run `Git: Clone` command in VSCode with `git@github.com:Kloudtechcorp/AWS.git` as the repository URL to clone the repository.
3. Once the project is loaded and the dependencies from the `platformio.ini` file are installed, run `PlatformIO: Build` to build the project.

*Note: When getting "Please configure IDF framework to include mbedTLS -> Enable pre-shared-key ciphersuites and activate at least one cipher" error when compiling, see [this commit](https://github.com/gravitech-engineer/AIS_IoT_4G/pull/8/commits/11a26867f73f45a54e46d8132b264b4eb5ff93ad).*

## Uploading

1. Make sure the device is connected to the computer and detectable by PlatformIO.
2. Run `PlatformIO: Upload` or `PlatformIO: Upload and Monitor` in VSCode to upload the firmware to the device.

### Testing

For testing purposes, the resource path of POST request, `/api/{version}/weather/insert-data?serial={serial}`, can be modified, where `{version}` is the API version (e.g., `v1`, `test`) and `{serial}` is the serial number of the station. For example,

```
/api/test/weather/insert-data?serial=XXXX-XXXX-XXXX-XXXX
```

The test server dashboard can then be accessed at `test.kloudtechsea.com`, requiring a valid username and password to log in. The dashboard will display the data sent from the weather station.
