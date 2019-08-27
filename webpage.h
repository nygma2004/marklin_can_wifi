const char webPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
  body {
      background: #fff;
      color: #333;
      font-family: "Open Sans",sans-serif;
      font-style: normal;
      font-weight: 400;
      line-height: 1.5;
  }

  button {
    border-radius: 0;
    border-style: solid;
    border-width: 0;
    cursor: pointer;
    font-family: "Open Sans",sans-serif;
    font-weight: 400;
    line-height: normal;
    margin: 0 0 1.25rem;
    position: relative;
    text-align: left;
    text-decoration: none;
    display: inline-block;
    padding: 1rem 2rem 1.0625rem 2rem;
    font-size: 1.125rem;
    background-color: #e2001a;
    border-color: #ccc;
    color: #fff;
    transition: background-color 300ms ease-out;
}
  
</style>  
</head>
<body>
<script>
function getLog() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myarr = this.responseText.split("#");
      document.getElementById("can_log").innerHTML = myarr[0];
      document.getElementById("stats").innerHTML = myarr[1];
    }
  };
  xhttp.open("GET", "getlog", true);
  xhttp.send();
}
window.setInterval(getLog, 1000);

function myFunction() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("reply").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "testsend", true);
  xhttp.send();
}

function pointSwitch(type,address,state) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("reply").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "point?type=" + type + "&address=" + address + "&state=" + state, true);
  xhttp.send();
}

function clearLog() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("reply").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "clearlog");
  xhttp.send();
}

</script>

<h1>M&auml;rklin CAN-Wifi Gateway</h1>
<table width="100%" border="0" cellspacing="0" cellpadding="10">
  <tr>
    <td style="vertical-align:top"><p>This sketch reads data on the M&auml;rklin CAN bus and shows each message on the right. The message log is updated every second. Use a serial monitor (set to 115200 baud) to see messages updated instantly. Statistics (orange section) only updated every 10 seconds. Use the buttons below to control the points.</p>
      <p><button onclick="clearLog()" class="button">Clear Log</button>&nbsp;</p>
      <p><button onclick="myFunction()" class="button">Change Point</button>&nbsp;<span id="reply"></span></p>
      <p><button onclick="pointSwitch('dcc',1,0)" class="button">Point 1 straight</button>&nbsp;<button onclick="pointSwitch('dcc',1,1)" class="button">Point 1 diverging</button></p>
      <p><button onclick="pointSwitch('dcc',2,0)" class="button">Point 2 straight</button>&nbsp;<button onclick="pointSwitch('dcc',2,1)" class="button">Point 2 diverging</button></p>      <p style="font-size: 90%; color: #FF8000;" id="stats"></p>
      <p style="font-size: 80%; color: #08088A;">M&auml;rklin CAN-Wifi Gateway sketch  v1.0 | <a href="mailto:csongor.varga@gmail.com">email me</a> | <a href="https://github.com/nygma2004/marklin_can_wifi">GitHub</a></p>
    </td>
    <td style="vertical-align:top; border: 1px solid Khaki; padding: 30px; background-color: LightYellow;">
      <p style="font-family: Courier New,Courier,Lucida Sans Typewriter,Lucida Typewriter,monospace; font-size: 90%; color: #000000; white-space:nowrap;" id="can_log"></p>
    </td>
  </tr>
</table>
</body>
</html>
)=====";
