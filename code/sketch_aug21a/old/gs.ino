#define ECO_TESTE_SEM_LDR 1  // 1 = simulacao; 0 = leitura real do sensor LDR

#include <DabbleESP32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 
#define IN1 13
#define IN2 4
#define IN3 33
#define IN4 32
#define ENA 19
#define ENB 18
#define BUZZER 5
 
#define TRIG_FRONT 25
#define ECHO_FRONT 26
 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
 
const int PWM_FREQ = 20000;
const int PWM_RES  = 8;
const int SPEED    = 200;
const float DIST_LIMIT_CM = 15.0;
const float DIST_PERIGO_CM = 5.0;          // quase batendo -> cara de susto
const unsigned long TEMPO_SONO_MS = 8000;  // fica parado sem comando por isso -> dorme
const unsigned long MONITOR_INTERVALO_MS = 250;  // periodo do monitor de proximidade
 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// EcoBot: declaracoes das duas funcoes novas (implementadas NO FINAL do arquivo).
void ecoIniciar();
void ecoAtualizar();
 
enum Face {
  FACE_IDLE, FACE_FORWARD, FACE_BACKWARD, FACE_LEFT, FACE_RIGHT,
  FACE_HORN, FACE_MUSIC, FACE_DANCE, FACE_SCARED, FACE_SLEEPY
};
Face currentFace = FACE_IDLE;
Face lastDrawnFace = FACE_MUSIC;
 
bool autopilotAtivo = false;
bool lastTriangleState = false;
unsigned long ultimaAcaoMs = 0;
 
// ---- estado para logs (evita flood no serial) ----
const char* ultimoMotor = "";
unsigned long ultimoMonitorMs = 0;
bool lastSquare = false, lastCircle = false, lastCross = false, lastStart = false;
 
// ================= Logs =================
 
void logTs(const char* tag) {
  Serial.print("[");
  Serial.print(millis());
  Serial.print(" ms] [");
  Serial.print(tag);
  Serial.print("] ");
}
 
void logMsg(const char* tag, const char* msg) {
  logTs(tag);
  Serial.println(msg);
}
 
// Loga so quando a acao do motor muda (os motores sao chamados a cada loop)
void logMotor(const char* acao) {
  if (strcmp(acao, ultimoMotor) != 0) {
    logMsg("MOTOR", acao);
    ultimoMotor = acao;
  }
}
 
const char* classificarDistancia(float d) {
  if (d < 0) return "SEM ECO";
  if (d <= DIST_PERIGO_CM) return "PERIGO";
  if (d <= DIST_LIMIT_CM) return "ATENCAO";
  return "LIVRE";
}
 
void imprimirDistancia(const char* rotulo, float d) {
  Serial.print(rotulo);
  Serial.print("=");
  if (d < 0) Serial.print("--");
  else { Serial.print(d, 1); Serial.print("cm"); }
  Serial.print(" (");
  Serial.print(classificarDistancia(d));
  Serial.print(")");
}
 
// Monitor de proximidade: imprime frente/tras com limite de taxa
void imprimirProximidade(const char* modo, float frente, float tras, bool temTras) {
  logTs("PROX");
  Serial.print(modo);
  Serial.print(" | ");
  imprimirDistancia("frente", frente);
  if (temTras) {
    Serial.print(" | ");
    imprimirDistancia("tras", tras);
  }
  Serial.println();
}
 
void monitorProximidadeManual() {
  if (millis() - ultimoMonitorMs < MONITOR_INTERVALO_MS) return;
  ultimoMonitorMs = millis();
  float f = readDistanceCm(TRIG_FRONT, ECHO_FRONT);
  imprimirProximidade("manual", f, -1, false);
}
 
const char* nomeFace(Face f) {
  switch (f) {
    case FACE_IDLE:     return "IDLE (feliz)";
    case FACE_FORWARD:  return "FORWARD (feliz animado)";
    case FACE_BACKWARD: return "BACKWARD (surpreso)";
    case FACE_LEFT:     return "LEFT";
    case FACE_RIGHT:    return "RIGHT";
    case FACE_HORN:     return "HORN (gritando)";
    case FACE_MUSIC:    return "MUSIC (cantando)";
    case FACE_DANCE:    return "DANCE";
    case FACE_SCARED:   return "SCARED";
    case FACE_SLEEPY:   return "SLEEPY";
  }
  return "?";
}
 
// ================= Setup / Loop =================
 
void setup() {
  Serial.begin(115200);
  delay(200);
  logMsg("BOOT", "Iniciando carrinho 4WD");
 
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(BUZZER, OUTPUT);
 
  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
  logMsg("BOOT", "Pinos configurados (motores, buzzer, sensor frontal)");
 
  ledcAttach(ENA, PWM_FREQ, PWM_RES);
  ledcAttach(ENB, PWM_FREQ, PWM_RES);
  logMsg("BOOT", "PWM configurado (ENA/ENB)");
 
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    logMsg("ERRO", "Falha ao encontrar o display OLED");
  } else {
    logMsg("BOOT", "Display OLED OK");
  }
 
  Dabble.begin("My Bluetooth Car");
  logMsg("BT", "Dabble iniciado: 'My Bluetooth Car' (aguardando conexao)");
  stopMotors();
  ultimaAcaoMs = millis();
  ecoIniciar();  // NOVO: inicializa o sensor de luminosidade.
  logMsg("BOOT", "Setup concluido");
}
 
void loop() {
  Dabble.processInput();
 
  bool triangle = GamePad.isTrianglePressed();
  if (triangle && !lastTriangleState) {
    autopilotAtivo = !autopilotAtivo;
    if (!autopilotAtivo) stopMotors();
    logMsg("BOTAO", "TRIANGULO pressionado");
    logMsg("MODO", autopilotAtivo ? "Piloto automatico: ON" : "Piloto automatico: OFF");
  }
  lastTriangleState = triangle;
 
  if (autopilotAtivo) {
    rodarPilotoAutomatico();
  } else {
    rodarModoManual();
    monitorProximidadeManual();
  }
 
  // ---- Botões extras (têm prioridade visual sobre movimento) ----
  bool square = GamePad.isSquarePressed();
  bool circle = GamePad.isCirclePressed();
  bool cross  = GamePad.isCrossPressed();
  bool start  = GamePad.isStartPressed();
 
  if (square && !lastSquare) logMsg("BOTAO", "QUADRADO pressionado -> buzina");
  if (!square && lastSquare) logMsg("BOTAO", "QUADRADO solto -> buzina off");
  if (circle && !lastCircle) logMsg("BOTAO", "BOLA pressionado -> melodia");
  if (cross && !lastCross)   logMsg("BOTAO", "X pressionado -> danca");
  if (start && !lastStart)   logMsg("BOTAO", "START pressionado -> sirene");
  lastSquare = square; lastCircle = circle; lastCross = cross; lastStart = start;
 
  if (square) {
    tone(BUZZER, 1000);
    currentFace = FACE_HORN;
    ultimaAcaoMs = millis();
  } else if (circle) {
    tocarMelodia();
    currentFace = FACE_MUSIC;
    ultimaAcaoMs = millis();
  } else if (cross) {
    dancar();
    currentFace = FACE_DANCE;
    ultimaAcaoMs = millis();
  } else if (start) {
    tocarSirene();
    currentFace = FACE_SCARED;
    ultimaAcaoMs = millis();
  } else {
    noTone(BUZZER);
  }
 
  // ---- Modo soninho: sem nenhum comando por um tempo ----
  if (currentFace == FACE_IDLE && (millis() - ultimaAcaoMs > TEMPO_SONO_MS)) {
    currentFace = FACE_SLEEPY;
    logMsg("ESTADO", "Sem comandos ha tempo demais -> modo soninho");
  }
 
  if (currentFace != lastDrawnFace) {
    desenharFace(currentFace);
    lastDrawnFace = currentFace;
  }

  ecoAtualizar();  // NOVO: analisa o LDR e roda a IA sem bloquear o carrinho.
}
 
// ================= Modo manual =================
 
void rodarModoManual() {
  bool up     = GamePad.isUpPressed();
  bool down   = GamePad.isDownPressed();
  bool left   = GamePad.isLeftPressed();
  bool right  = GamePad.isRightPressed();
 
  if (up)        { moveForward();  currentFace = FACE_FORWARD;  ultimaAcaoMs = millis(); }
  else if (down) { moveBackward(); currentFace = FACE_BACKWARD; ultimaAcaoMs = millis(); }
  else if (left) { turnLeft();     currentFace = FACE_LEFT;     ultimaAcaoMs = millis(); }
  else if (right){ turnRight();    currentFace = FACE_RIGHT;    ultimaAcaoMs = millis(); }
  else           { stopMotors();   currentFace = FACE_IDLE; }
}
 
// ================= Piloto automático =================
 
void rodarPilotoAutomatico() {
  float distFrente = readDistanceCm(TRIG_FRONT, ECHO_FRONT);
 
  // Monitor de proximidade (limitado em taxa)
  if (millis() - ultimoMonitorMs >= MONITOR_INTERVALO_MS) {
    ultimoMonitorMs = millis();
    imprimirProximidade("auto", distFrente, -1, false);
  }
 
  if (distFrente < 0 || distFrente > DIST_LIMIT_CM) {
    moveForward();
    currentFace = FACE_FORWARD;
    ultimaAcaoMs = millis();
    return;
  }
 
  logMsg("AUTO", "Obstaculo a frente detectado");
  imprimirProximidade("auto/obstaculo", distFrente, -1, false);
 
  // Muito perto - susto de verdade, não só desvio calmo
  if (distFrente <= DIST_PERIGO_CM) {
    logMsg("AUTO", "PERIGO: parando e tocando sirene");
    stopMotors();
    currentFace = FACE_SCARED;
    tocarSirene();
  } else {
    logMsg("AUTO", "Parando para desviar");
    stopMotors();
    currentFace = FACE_BACKWARD;
  }
  ultimaAcaoMs = millis();
  delay(200);
 
  logMsg("AUTO", "Girando a direita para desviar");
  turnRight();
  currentFace = FACE_RIGHT;
  delay(400);
 
  stopMotors();
  logMsg("AUTO", "Manobra de desvio concluida");
}
 
float readDistanceCm(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return -1;
 
  return duration * 0.0343 / 2.0;
}
 
// ================= Motores =================
 
void moveForward() {
  logMotor("FRENTE");
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  setSpeed(SPEED);
}
void moveBackward() {
  logMotor("RE");
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  setSpeed(SPEED);
}
void turnLeft() {
  logMotor("ESQUERDA");
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  setSpeed(SPEED);
}
void turnRight() {
  logMotor("DIREITA");
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  setSpeed(SPEED);
}
void stopMotors() {
  logMotor("PARADO");
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  setSpeed(0);
}
void setSpeed(int speed) {
  ledcWrite(ENA, speed);
  ledcWrite(ENB, speed);
}
 
// Gira alternando rápido pros dois lados - "dancinha" (bloqueia ~600ms)
void dancar() {
  logMsg("DANCA", "Inicio da danca");
  turnLeft();  setSpeed(SPEED); delay(150);
  turnRight(); setSpeed(SPEED); delay(150);
  turnLeft();  setSpeed(SPEED); delay(150);
  turnRight(); setSpeed(SPEED); delay(150);
  stopMotors();
  logMsg("DANCA", "Fim da danca");
}
 
// ================= Buzzer =================
 
void tocarMelodia() {
  logMsg("BUZZER", "Melodia: inicio");
  int notas[]    = {659, 587, 494, 587};
  int duracoes[] = {150, 150, 150, 150};
  for (int i = 0; i < 4; i++) {
    tone(BUZZER, notas[i], duracoes[i]);
    delay(duracoes[i] + 30);
  }
  noTone(BUZZER);
  logMsg("BUZZER", "Melodia: fim");
}
 
// Sirene tipo "carro de polícia" - sweep de frequência
void tocarSirene() {
  logMsg("BUZZER", "Sirene: inicio");
  for (int f = 400; f <= 1200; f += 40) {
    tone(BUZZER, f);
    delay(8);
  }
  for (int f = 1200; f >= 400; f -= 40) {
    tone(BUZZER, f);
    delay(8);
  }
  noTone(BUZZER);
  logMsg("BUZZER", "Sirene: fim");
}
 
// ================= Faces =================
 
void desenharFace(Face f) {
  logTs("DISPLAY");
  Serial.print("Face -> ");
  Serial.println(nomeFace(f));
 
  display.clearDisplay();
  switch (f) {
    case FACE_IDLE:      faceFeliz(false); break;
    case FACE_FORWARD:   faceFeliz(true);  break;
    case FACE_BACKWARD:  faceSurpreso();   break;
    case FACE_LEFT:      faceOlhar(-8);    break;
    case FACE_RIGHT:     faceOlhar(8);     break;
    case FACE_HORN:      faceGritando();   break;
    case FACE_MUSIC:     faceCantando();   break;
    case FACE_DANCE:     faceDancando();   break;
    case FACE_SCARED:    faceAssustado();  break;
    case FACE_SLEEPY:    faceDormindo();   break;
  }
  display.display();
}
 
void sorrisoBase() {
  for (int x = 0; x <= 40; x++) {
    int y = 45 + (x * (40 - x)) / 60;
    display.drawPixel(44 + x, y, SSD1306_WHITE);
    display.drawPixel(45 + x, y, SSD1306_WHITE);
  }
}
 
void faceFeliz(bool animado) {
  display.fillCircle(40, 24, 8, SSD1306_WHITE);
  display.fillCircle(88, 24, 8, SSD1306_WHITE);
  if (animado) {
    display.fillCircle(64, 48, 7, SSD1306_WHITE);
  } else {
    sorrisoBase();
  }
}
 
void faceSurpreso() {
  display.drawCircle(40, 26, 9, SSD1306_WHITE);
  display.drawCircle(88, 26, 9, SSD1306_WHITE);
  display.fillCircle(40, 26, 4, SSD1306_WHITE);
  display.fillCircle(88, 26, 4, SSD1306_WHITE);
  display.fillCircle(64, 48, 6, SSD1306_WHITE);
}
 
void faceOlhar(int deslocamento) {
  display.drawCircle(40, 24, 8, SSD1306_WHITE);
  display.drawCircle(88, 24, 8, SSD1306_WHITE);
  display.fillCircle(40 + deslocamento, 24, 4, SSD1306_WHITE);
  display.fillCircle(88 + deslocamento, 24, 4, SSD1306_WHITE);
  sorrisoBase();
}
 
void faceGritando() {
  display.fillCircle(40, 22, 7, SSD1306_WHITE);
  display.fillCircle(88, 22, 7, SSD1306_WHITE);
  display.fillRoundRect(50, 40, 28, 16, 4, SSD1306_WHITE);
}
 
void faceCantando() {
  display.drawLine(32, 26, 48, 20, SSD1306_WHITE);
  display.drawLine(48, 20, 32, 14, SSD1306_WHITE);
  display.drawLine(80, 26, 96, 20, SSD1306_WHITE);
  display.drawLine(96, 20, 80, 14, SSD1306_WHITE);
  sorrisoBase();
  display.fillCircle(110, 44, 3, SSD1306_WHITE);
  display.drawLine(113, 44, 113, 30, SSD1306_WHITE);
  display.drawLine(113, 30, 118, 32, SSD1306_WHITE);
}
 
// Olhos em espiral (tontura) - aproximado com círculos concêntricos
void faceDancando() {
  display.drawCircle(40, 24, 8, SSD1306_WHITE);
  display.drawCircle(40, 24, 5, SSD1306_WHITE);
  display.drawCircle(40, 24, 2, SSD1306_WHITE);
  display.drawCircle(88, 24, 8, SSD1306_WHITE);
  display.drawCircle(88, 24, 5, SSD1306_WHITE);
  display.drawCircle(88, 24, 2, SSD1306_WHITE);
  // boca ondulada
  display.drawLine(44, 48, 54, 44, SSD1306_WHITE);
  display.drawLine(54, 44, 64, 48, SSD1306_WHITE);
  display.drawLine(64, 48, 74, 44, SSD1306_WHITE);
  display.drawLine(74, 44, 84, 48, SSD1306_WHITE);
}
 
// Olhos em "X" (assustado)
void faceAssustado() {
  display.drawLine(32, 18, 48, 32, SSD1306_WHITE);
  display.drawLine(48, 18, 32, 32, SSD1306_WHITE);
  display.drawLine(80, 18, 96, 32, SSD1306_WHITE);
  display.drawLine(96, 18, 80, 32, SSD1306_WHITE);
  display.fillCircle(64, 48, 8, SSD1306_WHITE); // boca em "O" de grito
}
 
// Olhos fechados retos + "Z" de sono
void faceDormindo() {
  display.drawLine(32, 24, 48, 24, SSD1306_WHITE);
  display.drawLine(80, 24, 96, 24, SSD1306_WHITE);
  sorrisoBase();
  display.setTextSize(1);
  display.setCursor(100, 4);
  display.print("z");
  display.setCursor(108, 0);
  display.print("Z");
}


// ============================================================================
// ECOBOT | ASSISTENTE DE USO CONSCIENTE DE ENERGIA RESIDENCIAL
// ============================================================================
// MODIFICACOES MINIMAS NO CODIGO ORIGINAL:
// 1) Dois prototipos declarados depois da criacao do display.
// 2) ecoIniciar() chamado no setup().
// 3) ecoAtualizar() chamado no final do loop().
// TODO O RESTANTE DO CODIGO ACIMA E O PROJETO ORIGINAL.
//
// HARDWARE NOVO: 1 modulo LDR com pino AO (analogico).
//   LDR VCC -> ESP32 3V3
//   LDR GND -> ESP32 GND
//   LDR AO  -> ESP32 GPIO 34
//   LDR DO  -> NAO CONECTAR
// NUNCA ligue o modulo em 5 V: a entrada do ESP32 nao e tolerante a 5 V.
//
// COMO TESTAR (Monitor Serial 115200, fim de linha pode ser qualquer um):
//   e   = memoriza a leitura com o sensor coberto/escuro
//   c   = memoriza a leitura com o sensor iluminado/claro
//   1   = informa que uma lampada residencial ESTA LIGADA (simulacao)
//   0   = informa que a lampada ESTA DESLIGADA (simulacao)
//   ?   = mostra a ajuda
// No modo real, o LDR e real. No modo teste, as leituras sao simuladas.
// O estado da lampada e informado por comando,
// porque este carrinho NAO tem sensor de corrente, rele, LED ou tomada.
//
// IA REAL, MAS SIMPLES: classificador k-NN (k=3) com exemplos rotulados.
// As amostras de treinamento sao cenarios DIDATICOS, nao medicoes reais.
// O sistema oferece RECOMENDACOES; ele nao mede kWh nem controla a rede.
// ============================================================================

const uint8_t ECO_PIN_LDR = 34;
bool ecoSimularClaro = false;  // false = 600 ADC, true = 3300 ADC (somente em simulacao)
const unsigned long ECO_INTERVALO_MS = 2000;
const unsigned long ECO_INTERVALO_TELA_MS = 4000;
const uint8_t ECO_K = 3;

// Calibracao local, refeita a cada reinicio para dispensar EEPROM/bibliotecas.
int ecoLeituraEscuro = -1;
int ecoLeituraClaro = -1;
int ecoLeituraAtual = 0;
float ecoClaridadePercentual = 0.0f;
bool ecoLampadaLigada = false;
bool ecoLampadaInformada = false;
bool ecoAlertaDesperdicio = false;
unsigned long ecoUltimaAnaliseMs = 0;
unsigned long ecoUltimaTelaMs = 0;

struct EcoExemplo {
  uint8_t claridade;    // 0 = escuro, 100 = muito claro
  bool lampadaLigada;   // estado da lampada INFORMADO pelo usuario
  bool desperdicio;     // rotulo usado na classificacao
};

// Base de treinamento DEMONSTRATIVA: pode ser ampliada com dados coletados.
const EcoExemplo ECO_EXEMPLOS[] = {
  {  0, false, false}, { 25, false, false}, { 50, false, false},
  { 75, false, false}, {100, false, false},
  {  0, true,  false}, { 10, true,  false}, { 20, true,  false},
  { 30, true,  false}, { 40, true,  false}, { 50, true,  false},
  { 60, true,   true}, { 70, true,   true}, { 80, true,   true},
  { 90, true,   true}, {100, true,   true}
};
const size_t ECO_TOTAL_EXEMPLOS = sizeof(ECO_EXEMPLOS) / sizeof(ECO_EXEMPLOS[0]);

void ecoAjuda() {
  Serial.println();
  Serial.println("========== ECOBOT - COMANDOS ==========");
#if ECO_TESTE_SEM_LDR
  Serial.println("MODO TESTE: sem sensor fisico conectado");
  Serial.println("e = calibrar ESCURO SIMULADO (600 ADC)");
  Serial.println("c = calibrar CLARO SIMULADO (3300 ADC)");
  Serial.println("a = simular ambiente CLARO");
  Serial.println("b = simular ambiente ESCURO");
#else
  Serial.println("e = calibrar no ESCURO (cubra o LDR)");
  Serial.println("c = calibrar no CLARO  (ilumine o LDR)");
#endif
  Serial.println("1 = informar LAMPADA LIGADA (simulada)");
  Serial.println("0 = informar LAMPADA DESLIGADA (simulada)");
  Serial.println("? = mostrar esta ajuda");
  Serial.println("IA = k-NN, 3 vizinhos, com exemplos didaticos");
  Serial.println("=======================================");
  Serial.println();
}

int ecoLerLDR() {
#if ECO_TESTE_SEM_LDR
  // Simula sensor calibravel em 0-100%, sem pino externo conectado.
  return ecoSimularClaro ? 3300 : 600;
#else
  long soma = 0;
  for (int i = 0; i < 8; i++) {
    soma += analogRead(ECO_PIN_LDR);
    delayMicroseconds(150);
  }
  return (int)(soma / 8);
#endif
}

bool ecoEstaCalibrado() {
  // O sinal pode subir ou descer quando se ilumina o LDR: a calibracao
  // funciona nos dois sentidos e independe do tipo de modulo analogico.
  return ecoLeituraEscuro >= 0 && ecoLeituraClaro >= 0 &&
         abs(ecoLeituraClaro - ecoLeituraEscuro) >= 80;
}

float ecoConverterParaPercentual(int leitura) {
  float faixa = (float)(ecoLeituraClaro - ecoLeituraEscuro);
  float percentual = 100.0f * (leitura - ecoLeituraEscuro) / faixa;
  if (percentual < 0.0f) percentual = 0.0f;
  if (percentual > 100.0f) percentual = 100.0f;
  return percentual;
}

// Classificador k-NN: encontra os 3 cenarios rotulados mais proximos.
// A penalidade grande (200) impede que "lampada ON" seja comparada
// prioritariamente a exemplos de "lampada OFF".
// Nao depende de internet, API externa ou biblioteca de IA adicional.
bool ecoPreverDesperdicioKNN(float claridade, bool lampadaLigada) {
  float menoresDistancias[ECO_K] = {9999.0f, 9999.0f, 9999.0f};
  bool rotulos[ECO_K] = {false, false, false};

  for (size_t i = 0; i < ECO_TOTAL_EXEMPLOS; i++) {
    float distancia = fabsf(claridade - ECO_EXEMPLOS[i].claridade);
    if (lampadaLigada != ECO_EXEMPLOS[i].lampadaLigada) {
      distancia += 200.0f;
    }
    for (int pos = 0; pos < ECO_K; pos++) {
      if (distancia < menoresDistancias[pos]) {
        for (int j = ECO_K - 1; j > pos; j--) {
          menoresDistancias[j] = menoresDistancias[j - 1];
          rotulos[j] = rotulos[j - 1];
        }
        menoresDistancias[pos] = distancia;
        rotulos[pos] = ECO_EXEMPLOS[i].desperdicio;
        break;
      }
    }
  }

  int votosDesperdicio = 0;
  for (int i = 0; i < ECO_K; i++) {
    if (rotulos[i]) votosDesperdicio++;
  }
  return votosDesperdicio >= 2;
}

void ecoProcessarComandos() {
  while (Serial.available() > 0) {
    char tecla = (char)Serial.read();
    if (tecla == 'e' || tecla == 'E') {
#if ECO_TESTE_SEM_LDR
      ecoSimularClaro = false;
#endif
      ecoLeituraEscuro = ecoLerLDR();
      Serial.print("[ECO] Escuro registrado. ADC = ");
      Serial.println(ecoLeituraEscuro);
    } else if (tecla == 'c' || tecla == 'C') {
#if ECO_TESTE_SEM_LDR
      ecoSimularClaro = true;
#endif
      ecoLeituraClaro = ecoLerLDR();
      Serial.print("[ECO] Claro registrado. ADC = ");
      Serial.println(ecoLeituraClaro);
#if ECO_TESTE_SEM_LDR
    } else if (tecla == 'a' || tecla == 'A') {
      ecoSimularClaro = true;
      Serial.println("[ECO TESTE] Ambiente CLARO simulado (3300 ADC)");
    } else if (tecla == 'b' || tecla == 'B') {
      ecoSimularClaro = false;
      Serial.println("[ECO TESTE] Ambiente ESCURO simulado (600 ADC)");
#endif
    } else if (tecla == '1') {
      ecoLampadaInformada = true;
      ecoLampadaLigada = true;
      Serial.println("[ECO] Lampada simulada: LIGADA");
    } else if (tecla == '0') {
      ecoLampadaInformada = true;
      ecoLampadaLigada = false;
      Serial.println("[ECO] Lampada simulada: DESLIGADA");
    } else if (tecla == '?') {
      ecoAjuda();
    }
    // Ignora automaticamente \r, \n e outros caracteres.
  }
}

void ecoDesenharResumo() {
  // Nao interrompe as expressoes faciais enquanto o robo esta se movendo,
  // dançando, tocando ou em piloto automatico.
  if (autopilotAtivo || currentFace != FACE_IDLE) return;
  if (millis() - ultimaAcaoMs < 2000) return;
  if (millis() - ecoUltimaTelaMs < ECO_INTERVALO_TELA_MS) return;
  ecoUltimaTelaMs = millis();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("ECOBOT - ENERGIA");
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

  if (!ecoEstaCalibrado()) {
    display.setCursor(0, 20);
    display.println("Calibre o LDR:");
    display.println("e = escuro");
    display.println("c = claro");
  } else if (!ecoLampadaInformada) {
    display.setCursor(0, 20);
    display.print("Luz ambiente: ");
    display.print((int)ecoClaridadePercentual);
    display.println("%");
    display.println("Informe lampada:");
    display.println("1=ON  0=OFF");
  } else {
    display.setCursor(0, 18);
    display.print("Claridade: ");
    display.print((int)ecoClaridadePercentual);
    display.println("%");
    display.print("Lampada: ");
    display.println(ecoLampadaLigada ? "ON" : "OFF");
    display.println(ecoAlertaDesperdicio ? "IA: POSSIVEL" : "IA: USO OK");
    if (ecoAlertaDesperdicio) display.println("DESPERDICIO!");
    else display.println("Economia consciente");
  }
  display.display();
}

void ecoIniciar() {
  pinMode(ECO_PIN_LDR, INPUT);
  analogReadResolution(12);  // ESP32: leituras analogicas 0..4095.
#if ECO_TESTE_SEM_LDR
  Serial.println("[ECO TESTE] Sem LDR fisico: simulacao ATIVA.");
  Serial.println("[ECO TESTE] Use e, c, 1, 0, a e b para testar a IA.");
#else
  Serial.println("[ECO] Sensor LDR no GPIO 34 pronto.");
#endif
  Serial.println("[ECO] Comece com e (escuro) e c (claro) no Monitor Serial.");
  ecoAjuda();
}

void ecoAtualizar() {
  ecoProcessarComandos();
  unsigned long agora = millis();
  if (agora - ecoUltimaAnaliseMs < ECO_INTERVALO_MS) return;
  ecoUltimaAnaliseMs = agora;

  ecoLeituraAtual = ecoLerLDR();
  Serial.print("[ECO] LDR ADC = ");
  Serial.print(ecoLeituraAtual);

  if (!ecoEstaCalibrado()) {
    Serial.println(" | Calibracao pendente (use e e c com luzes diferentes)");
    ecoDesenharResumo();
    return;
  }

  ecoClaridadePercentual = ecoConverterParaPercentual(ecoLeituraAtual);
  Serial.print(" | Claridade = ");
  Serial.print(ecoClaridadePercentual, 0);
  Serial.print("%");

  if (!ecoLampadaInformada) {
    Serial.println(" | Informe 1 (lampada ON) ou 0 (OFF)");
    ecoDesenharResumo();
    return;
  }

  ecoAlertaDesperdicio = ecoPreverDesperdicioKNN(
    ecoClaridadePercentual, ecoLampadaLigada
  );
  Serial.print(" | Lampada = ");
  Serial.print(ecoLampadaLigada ? "ON" : "OFF");
  if (ecoAlertaDesperdicio) {
    Serial.println(" | IA: POSSIVEL DESPERDICIO! Considere apagar a lampada.");
    // Estimativa ilustrativa: 10 W * 4 h/dia * 30 dias / 1000 = 1,2 kWh/mes.
    // NAO e uma medicao real de consumo ou economia.
    Serial.println("[ECO] Exemplo de economia: 10 W por 4 h/dia = 1.2 kWh/mes.");
  } else {
    Serial.println(" | IA: NENHUM DESPERDICIO IDENTIFICADO NESTE CENARIO");
  }
  ecoDesenharResumo();
}
