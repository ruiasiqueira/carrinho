#include <WiFi.h>
#include <WebServer.h>

// ==============================
// CONFIGURACAO WI-FI
// ==============================
const char* ssid = "Iphone de Rui";
const char* password = "Rui@lindo";

// Servidor web na porta 80
WebServer server(80);

// ==============================
// PINOS L298N
// ==============================

// Motor A
const int motor1Pin1 = 27;  // IN1
const int motor1Pin2 = 26;  // IN2
const int enable1Pin = 14;  // ENA

// Motor B
const int motor2Pin1 = 33;  // IN3
const int motor2Pin2 = 25;  // IN4
const int enable2Pin = 32;  // ENB

// ==============================
// CONFIGURACAO PWM
// ==============================

const int freq = 5000;
const int resolution = 8;

// Velocidade inicial
int dutyCycle = 0;

// ==============================
// PAGINA WEB
// ==============================

void handleRoot() {
    const char html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>

<meta name="viewport" content="width=device-width, initial-scale=1">

<link rel="icon" href="data:,">

<style>

html {
    font-family: Helvetica;
    display: inline-block;
    margin: 0px auto;
    text-align: center;
}

body {
    margin-top: 30px;
}

.button {
    -webkit-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;

    background-color: #4CAF50;

    border: none;

    color: white;

    padding: 12px 28px;

    text-decoration: none;

    font-size: 26px;

    margin: 3px;

    cursor: pointer;

    border-radius: 8px;
}

.button2 {
    background-color: #555555;
}

</style>

<script>

function moveForward() {
    fetch('/forward');
}

function moveLeft() {
    fetch('/left');
}

function stopRobot() {
    fetch('/stop');
}

function moveRight() {
    fetch('/right');
}

function moveReverse() {
    fetch('/reverse');
}

function updateMotorSpeed(pos) {

    document.getElementById('motorSpeed').innerHTML = pos;

    fetch('/speed?value=' + pos);
}

</script>

</head>

<body>

<h1>ESP32 Motor Control</h1>

<p>

<button
    class="button"
    onclick="moveForward()">

    FORWARD

</button>

</p>

<p>

<button
    class="button"
    onclick="moveLeft()">

    LEFT

</button>

<button
    class="button button2"
    onclick="stopRobot()">

    STOP

</button>

<button
    class="button"
    onclick="moveRight()">

    RIGHT

</button>

</p>

<p>

<button
    class="button"
    onclick="moveReverse()">

    REVERSE

</button>

</p>

<p>

Motor Speed:

<span id="motorSpeed">0</span>%

</p>

<input
    type="range"
    min="0"
    max="100"
    step="25"
    id="motorSlider"
    oninput="updateMotorSpeed(this.value)"
    value="0"
/>

</body>

</html>
)rawliteral";

    server.send(200, "text/html", html);
}

// ==============================
// FRENTE
// ==============================

void handleForward() {
    Serial.println("Forward");

    // Motor A para frente
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);

    // Motor B para frente
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, HIGH);

    server.send(200, "text/plain", "Forward");
}

// ==============================
// ESQUERDA
// ==============================

void handleLeft() {
    Serial.println("Left");

    // Motor A parado
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);

    // Motor B para frente
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, HIGH);

    server.send(200, "text/plain", "Left");
}

// ==============================
// PARAR
// ==============================

void handleStop() {
    Serial.println("Stop");

    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);

    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, LOW);

    server.send(200, "text/plain", "Stop");
}

// ==============================
// DIREITA
// ==============================

void handleRight() {
    Serial.println("Right");

    // Motor A para frente
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);

    // Motor B parado
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, LOW);

    server.send(200, "text/plain", "Right");
}

// ==============================
// RE
// ==============================

void handleReverse() {
    Serial.println("Reverse");

    // Motor A para tras
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);

    // Motor B para tras
    digitalWrite(motor2Pin1, HIGH);
    digitalWrite(motor2Pin2, LOW);

    server.send(200, "text/plain", "Reverse");
}

// ==============================
// CONTROLE DE VELOCIDADE
// ==============================

void handleSpeed() {
    if (server.hasArg("value")) {

        String valueString = server.arg("value");

        int value = valueString.toInt();

        // Limita entre 0 e 100
        value = constrain(value, 0, 100);

        if (value == 0) {

            dutyCycle = 0;

            // PWM zero
            ledcWrite(enable1Pin, 0);
            ledcWrite(enable2Pin, 0);

            // Para motores
            digitalWrite(motor1Pin1, LOW);
            digitalWrite(motor1Pin2, LOW);

            digitalWrite(motor2Pin1, LOW);
            digitalWrite(motor2Pin2, LOW);

            Serial.println("Motor speed set to 0%");

        } else {

            /*
              Converte:
              25%  -> 200 PWM
              100% -> 255 PWM

              Comecamos em 200 porque motores DC podem
              nao conseguir partir com PWM muito baixo.
            */

            dutyCycle = map(value, 25, 100, 200, 255);

            // Segurança
            dutyCycle = constrain(dutyCycle, 0, 255);

            ledcWrite(enable1Pin, dutyCycle);
            ledcWrite(enable2Pin, dutyCycle);

            Serial.print("Motor speed set to: ");
            Serial.print(value);
            Serial.print("% | PWM: ");
            Serial.println(dutyCycle);
        }
    }

    server.send(200, "text/plain", "Speed updated");
}

// ==============================
// SETUP
// ==============================

void setup() {
    Serial.begin(115200);

    Serial.println();
    Serial.println("Starting ESP32 Motor Control...");

    // ==============================
    // CONFIGURA PINOS
    // ==============================

    pinMode(motor1Pin1, OUTPUT);
    pinMode(motor1Pin2, OUTPUT);

    pinMode(motor2Pin1, OUTPUT);
    pinMode(motor2Pin2, OUTPUT);

    // ==============================
    // CONFIGURA PWM
    // ==============================

    ledcAttach(enable1Pin, freq, resolution);
    ledcAttach(enable2Pin, freq, resolution);

    // Comeca com motores desligados
    ledcWrite(enable1Pin, 0);
    ledcWrite(enable2Pin, 0);

    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);

    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, LOW);

    // ==============================
    // CONECTA AO WI-FI
    // ==============================

    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected.");

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // ==============================
    // ROTAS DO SERVIDOR
    // ==============================

    server.on("/", handleRoot);
    server.on("/forward", handleForward);
    server.on("/left", handleLeft);
    server.on("/stop", handleStop);
    server.on("/right", handleRight);
    server.on("/reverse", handleReverse);
    server.on("/speed", handleSpeed);

    // ==============================
    // INICIA SERVIDOR
    // ==============================

    server.begin();

    Serial.println("HTTP server started.");
}

// ==============================
// LOOP
// ==============================

void loop() {
    server.handleClient();
}