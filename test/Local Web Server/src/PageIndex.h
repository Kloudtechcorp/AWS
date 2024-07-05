const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" rel="stylesheet">
    <style>
        html {
            font-family: Arial;
            display: inline-block;
            margin: 0px auto;
            text-align: center;
        }
        h1 {
            font-size: 2.0rem;
        }
        p {
            font-size: 2.0rem;
        }
        .units {
            font-size: 1.2rem;
        }
        .bme-labels {
            font-size: 1.5rem;
            vertical-align: middle;
            padding-bottom: 15px;
        }
        .button {
            display: inline-block;
            background-color: #4CAF50;
            border: none;
            color: white;
            padding: 16px 40px;
            text-decoration: none;
            font-size: 20px;
            margin: 2px;
            cursor: pointer;
        }
        .button:hover {
            background-color: #45a049;
        }
    </style>
</head>
<body>
    <h1>ESP32 Monitoring Sensor BME</h1>
    <p>
        <i class="fa fa-thermometer-half" style="font-size:3.0rem;color:#62a1d3;"></i>
        <span class="bme-labels">Temperature : </span>
        <span id="TemperatureValue">0</span>
        <sup class="units">&deg;C</sup>
    </p>
    <p>
        <i class="fa fa-tint" style="font-size:3.0rem;color:#75e095;"></i>
        <span class="bme-labels">Humidity : </span>
        <span id="HumidityValue">0</span>
        <sup class="units">%</sup>
    </p>
    <p>
        <button class="button" onclick="downloadCSV()">Download CSV</button>
        <button class="button" onclick="refreshData()">Refresh Data</button>
    </p>
    <p>
      <i class="far fa-clock" style="font-size:1.0rem;color:#e3a8c7;"></i>
      <span style="font-size:1.0rem;">Time </span>
      <span id="time" style="font-size:1.0rem;"></span>
      
      <i class="far fa-calendar-alt" style="font-size:1.0rem;color:#f7dc68";></i>
      <span style="font-size:1.0rem;">Date </span>
      <span id="date" style="font-size:1.0rem;"></span>
    </P>
    <script>
        setInterval(function () {
            getTemperatureData();
            getHumidityData();
        }, 60000);

        setInterval(function() {
        // Call a function repetatively with 1 Second interval
            Time_Date();
        }, 1000); 

        function getTemperatureData() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("TemperatureValue").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readTemperature", true);
            xhttp.send();
        }

        function getHumidityData() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("HumidityValue").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readHumidity", true);
            xhttp.send();
        }

        function Time_Date() {
            var t = new Date();
            document.getElementById("time").innerHTML = t.toLocaleTimeString();
            var d = new Date();
            const dayNames = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday","Saturday"];
            const monthNames = ["January", "February", "March", "April", "May", "June","July", "August", "September", "October", "November", "December"];
            document.getElementById("date").innerHTML = dayNames[d.getDay()] + ", " + d.getDate() + "-" + monthNames[d.getMonth()] + "-" + d.getFullYear();
        }
        function downloadCSV() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    var blob = new Blob([this.responseText], { type: 'text/csv' });
                    var url = window.URL.createObjectURL(blob);
                    var a = document.createElement('a');
                    a.href = url;
                    a.download = 'data.csv';
                    document.body.appendChild(a);
                    a.click();
                    document.body.removeChild(a);
                }
            };
            xhttp.open("GET", "downloadCSV", true);
            xhttp.send();
        }

        function refreshData() {
            getTemperatureData();
            getHumidityData();
        }
    </script>
</body>
</html>
)=====";