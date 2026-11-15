#include <DabbleESP32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h> // Guarda calibracao do LDR entre reinicializacoes
 
#define IN1 13
#define IN2 4
#define IN3 33
#define IN4 32
#define ENA 19
#define ENB 18
#define BUZZER 5
 
#define TRIG_FRONT 25
#define ECHO_FRONT 26
#define PINO_LDR 34   // Sinal AO do modulo LDR; GPIO34 e entrada analogica
 
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
bool displayOk = false;

// ================= ECOBOT: CONFIGURACAO =================
// 0 = sensor LDR real; 1 = simular a luz pelo Monitor Serial (a/b).
#define ECO_MODO_SIMULACAO 0
// 0 = alerta em 5 minutos; 1 = alerta em 10 segundos (somente demonstracao).
#define ECO_DEMO_RAPIDA 0

const unsigned long ECO_TEMPO_ALERTA_MS = ECO_DEMO_RAPIDA ? 10000UL : 300000UL;
const unsigned long ECO_INTERVALO_LEITURA_MS = 250UL;
const unsigned long ECO_TEMPO_ESTAVEL_MS = 2000UL;
const unsigned long ECO_INTERVALO_BIP_MS = 5000UL;
const unsigned long ECO_DURACAO_BIP_MS = 180UL;
const int ECO_LUZ_LIGADA_PCT = 55;
const int ECO_LUZ_DESLIGADA_PCT = 45;

Preferences ecoPrefs;
int ecoRawEscuro = -1;
int ecoRawClaro = -1;
int ecoLeitura = 0;
int ecoPercentual = 0;
int ecoSimulacaoADC = 600;
bool ecoLuzCandidata = false;
bool ecoLuzConfirmada = false;
bool ecoAlertaAtivo = false;
bool ecoBipAtivo = false;
bool ecoTelaAtiva = false;
unsigned long ecoMudancaCandidataMs = 0;
unsigned long ecoInicioLuzMs = 0;
unsigned long ecoUltimaLeituraMs = 0;
unsigned long ecoUltimoRelatorioMs = 0;
unsigned long ecoUltimoBipMs = 0;
unsigned long ecoInicioBipMs = 0;
unsigned long ecoUltimaTelaMs = 0;
int ecoClasseIA = 0; // 0=normal, 1=atencao, 2=revisar

// Exemplos DIDATICOS rotulados para k-NN: luz (%) e tempo (0-100 = 0-10 min).
// Nao sao leituras coletadas de residencias reais.
struct EcoAmostra { uint8_t luz; uint8_t tempo; uint8_t classe; };
const EcoAmostra ECO_TREINO[] = {
  {20,0,0}, {40,30,0}, {35,80,0}, {90,0,0}, {75,20,0},
  {65,40,1}, {85,40,1}, {55,30,1}, {60,50,1}, {90,50,1}, {48,55,1},
  {60,60,2}, {80,60,2}, {95,60,2}, {60,90,2}, {85,90,2},
  {95,100,2}, {35,100,0}
};
const int ECO_N_TREINO = sizeof(ECO_TREINO)/sizeof(ECO_TREINO[0]);

// Declaracoes das novas funcoes; mantemos o restante do codigo original.
void ecoIniciar();
void ecoLerSerial();
void ecoMonitorar();
void ecoAlertaSonoro(bool outroSomAtivo);
void ecoAtualizarOLED();
void ecoImprimirEstado();
void ecoAjuda();
bool ecoCalibrado();
int ecoClassificarKNN(int claridadePct, unsigned long tempoMs);

 
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
 
  displayOk = display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  if (!displayOk) {
    logMsg("ERRO", "Falha ao encontrar o display OLED");
  } else {
    logMsg("BOOT", "Display OLED OK");
  }
 
  Dabble.begin("My Bluetooth Car");
  logMsg("BT", "Dabble iniciado: 'My Bluetooth Car' (aguardando conexao)");
  stopMotors();
  ecoIniciar(); // NOVO: LDR, calibracao, temporizador e IA
  ultimaAcaoMs = millis();
  logMsg("BOOT", "Setup concluido");
}
 
void loop() {
  Dabble.processInput();
  ecoMonitorar();    // NOVO: conta iluminacao por 5 minutos sem delay()
  ecoLerSerial();    // NOVO: comandos de calibracao e teste
 
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
  } else if (!ecoBipAtivo) {
    noTone(BUZZER);  // Nao corta o bip de alerta do EcoBot
  }
 
  // ---- Modo soninho: sem nenhum comando por um tempo ----
  if (currentFace == FACE_IDLE && (millis() - ultimaAcaoMs > TEMPO_SONO_MS)) {
    currentFace = FACE_SLEEPY;
    logMsg("ESTADO", "Sem comandos ha tempo demais -> modo soninho");
  }
 
  if (currentFace != lastDrawnFace && !ecoTelaAtiva) {
    desenharFace(currentFace);
    lastDrawnFace = currentFace;
  }

  ecoAlertaSonoro(square || circle || cross || start);
  ecoAtualizarOLED();  // Mostra resumo por alguns segundos e devolve as carinhas
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
 
  if (distFrente < 0) {
    stopMotors(); // Seguranca: sem leitura confiavel, nao avanca as cegas
    currentFace = FACE_SCARED;
    return;
  }
  if (distFrente > DIST_LIMIT_CM) {
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
  if (!displayOk) return;
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

// ====================================================================
// ECOBOT - ASSISTENTE DE USO CONSCIENTE DE ENERGIA RESIDENCIAL
// Este LDR percebe CLARIDADE. Ele NAO distingue sol de lampada acesa.
// O aviso de 5 min pede para VERIFICAR a iluminacao, nao afirma consumo.
// ====================================================================

bool ecoCalibrado() {
  return ecoRawEscuro >= 0 && ecoRawClaro >= 0 &&
         abs(ecoRawClaro - ecoRawEscuro) >= 200;
}

void ecoAjuda() {
  Serial.println("\n[ECO] COMANDOS DO MONITOR SERIAL (115200, enviar cada letra):");
  Serial.println("  e = calibrar ESCURO (cobrir sensor)");
  Serial.println("  c = calibrar CLARO (iluminar sensor)");
  Serial.println("  s = mostrar estado atual");
  Serial.println("  r = reiniciar contagem dos 5 minutos");
  Serial.println("  ? = ajuda");
#if ECO_MODO_SIMULACAO
  Serial.println("  a = simular claridade; b = simular escuro");
#endif
  Serial.println("[ECO] OBS: o LDR mede luz ambiente, nao consumo de energia.\n");
}

void ecoIniciar() {
  analogReadResolution(12); // leitura 0..4095
  pinMode(PINO_LDR, INPUT);
#if ECO_MODO_SIMULACAO
  ecoRawEscuro = 600;
  ecoRawClaro = 3300;
  ecoSimulacaoADC = 600;
  Serial.println("[ECO] SIMULACAO ATIVA: a=claro, b=escuro");
#else
  ecoPrefs.begin("ecobot-ldr", false);
  ecoRawEscuro = ecoPrefs.getInt("escuro", -1);
  ecoRawClaro = ecoPrefs.getInt("claro", -1);
  if (!ecoCalibrado()) {
    Serial.println("[ECO] PRIMEIRO USO: calibrar com e (escuro) e c (claro).");
  } else {
    Serial.println("[ECO] Calibracao recuperada da memoria ESP32.");
  }
#endif
  ecoUltimoBipMs = millis();
  ecoAjuda();
}

void ecoReiniciarContagem() {
  ecoLuzCandidata = false;
  ecoLuzConfirmada = false;
  ecoAlertaAtivo = false;
  ecoMudancaCandidataMs = millis();
  ecoInicioLuzMs = 0;
  ecoClasseIA = 0;
  if (ecoBipAtivo) {
    noTone(BUZZER);
    ecoBipAtivo = false;
  }
}

void ecoLerSerial() {
  // Os comandos sao opcionais: o carro continua controlado pelo Dabble.
  while (Serial.available()) {
    char comando = (char)Serial.read();
    if (comando == '\n' || comando == '\r' || comando == ' ') continue;
    switch (comando) {
      case '?': ecoAjuda(); break;
      case 's': ecoImprimirEstado(); break;
      case 'r':
        ecoReiniciarContagem();
        Serial.println("[ECO] Contagem reiniciada.");
        break;
      case 'e':
#if !ECO_MODO_SIMULACAO
        ecoLeitura = analogRead(PINO_LDR);
        ecoRawEscuro = ecoLeitura;
        ecoPrefs.putInt("escuro", ecoRawEscuro);
        ecoReiniciarContagem();
        Serial.printf("[ECO] Escuro calibrado: %d ADC\n", ecoRawEscuro);
        break;
#else
        Serial.println("[ECO] Simulacao: calibracao e=600 ja definida.");
        break;
#endif
      case 'c':
#if !ECO_MODO_SIMULACAO
        ecoLeitura = analogRead(PINO_LDR);
        ecoRawClaro = ecoLeitura;
        ecoPrefs.putInt("claro", ecoRawClaro);
        ecoReiniciarContagem();
        Serial.printf("[ECO] Claro calibrado: %d ADC\n", ecoRawClaro);
        if (!ecoCalibrado())
          Serial.println("[ECO] ATENCAO: valores proximos! Repita e/c em condicoes diferentes.");
        break;
#else
        Serial.println("[ECO] Simulacao: calibracao c=3300 ja definida.");
        break;
#endif
#if ECO_MODO_SIMULACAO
      case 'a':
        ecoSimulacaoADC = 3300;
        Serial.println("[ECO] Ambiente CLARO simulado.");
        break;
      case 'b':
        ecoSimulacaoADC = 600;
        Serial.println("[ECO] Ambiente ESCURO simulado.");
        break;
#endif
      default: break;
    }
  }
}

// k-NN de 3 vizinhos sobre exemplos didaticos rotulados.
// Faz classificacao de situacoes, NAO mede corrente ou energia eletrica.
int ecoClassificarKNN(int claridadePct, unsigned long tempoMs) {
  int tempoEscala = (int)min(100UL, tempoMs / 6000UL); // 0..100 para 0..10 min
  long melhoresDist[3] = { 2147483647L, 2147483647L, 2147483647L };
  int melhoresClasses[3] = {0, 0, 0};
  for (int i = 0; i < ECO_N_TREINO; i++) {
    long dl = claridadePct - (int)ECO_TREINO[i].luz;
    long dt = tempoEscala - (int)ECO_TREINO[i].tempo;
    long distancia = dl * dl + dt * dt;
    for (int j = 0; j < 3; j++) {
      if (distancia < melhoresDist[j]) {
        for (int k = 2; k > j; k--) {
          melhoresDist[k] = melhoresDist[k - 1];
          melhoresClasses[k] = melhoresClasses[k - 1];
        }
        melhoresDist[j] = distancia;
        melhoresClasses[j] = ECO_TREINO[i].classe;
        break;
      }
    }
  }
  int votos[3] = {0,0,0};
  for (int i=0; i<3; i++) votos[melhoresClasses[i]]++;
  int vencedora = 0;
  if (votos[1] > votos[vencedora]) vencedora = 1;
  if (votos[2] > votos[vencedora]) vencedora = 2;
  return vencedora;
}

const char* ecoNomeClasse(int classe) {
  if (classe == 2) return "REVISAR USO";
  if (classe == 1) return "ATENCAO";
  return "NORMAL";
}

void ecoMonitorar() {
  unsigned long agora = millis();
  if (agora - ecoUltimaLeituraMs < ECO_INTERVALO_LEITURA_MS) return;
  ecoUltimaLeituraMs = agora;
#if ECO_MODO_SIMULACAO
  ecoLeitura = ecoSimulacaoADC;
#else
  ecoLeitura = analogRead(PINO_LDR);
#endif

  if (!ecoCalibrado()) {
    if (agora - ecoUltimoRelatorioMs >= 4000UL) {
      ecoUltimoRelatorioMs = agora;
      Serial.printf("[ECO] ADC=%d | Calibre: e=escuro, c=claro\n", ecoLeitura);
    }
    return; // sem calibracao confiavel nao gera alarmes
  }

  // Funciona mesmo que um modulo LDR tenha curva invertida.
  long percentual = (long)(ecoLeitura - ecoRawEscuro) * 100L /
                    (long)(ecoRawClaro - ecoRawEscuro);
  ecoPercentual = constrain((int)percentual, 0, 100);

  // Histerese para evitar falso disparo perto do limiar.
  bool candidato = ecoLuzCandidata
    ? (ecoPercentual >= ECO_LUZ_DESLIGADA_PCT)
    : (ecoPercentual >= ECO_LUZ_LIGADA_PCT);

  if (candidato != ecoLuzCandidata) {
    ecoLuzCandidata = candidato;
    ecoMudancaCandidataMs = agora;
  }

  // So muda o estado apos 2s estaveis; filtra sombras momentaneas.
  if (ecoLuzConfirmada != ecoLuzCandidata &&
      agora - ecoMudancaCandidataMs >= ECO_TEMPO_ESTAVEL_MS) {
    ecoLuzConfirmada = ecoLuzCandidata;
    if (ecoLuzConfirmada) {
      ecoInicioLuzMs = agora;
      ecoAlertaAtivo = false;
      Serial.println("[ECO] Claridade detectada: iniciando contagem.");
    } else {
      ecoReiniciarContagem();
      Serial.println("[ECO] Ambiente escureceu: contagem zerada.");
    }
  }

  unsigned long tempoLuzMs = ecoLuzConfirmada ? agora - ecoInicioLuzMs : 0UL;
  ecoClasseIA = ecoLuzConfirmada ? ecoClassificarKNN(ecoPercentual, tempoLuzMs) : 0;

  if (ecoLuzConfirmada && !ecoAlertaAtivo && tempoLuzMs >= ECO_TEMPO_ALERTA_MS) {
    ecoAlertaAtivo = true;
    ecoUltimoBipMs = agora - ECO_INTERVALO_BIP_MS; // primeiro bip imediato
    logMsg("ECO", "ALERTA: iluminacao continua acima do tempo permitido!");
    Serial.println("[ECO] RECOMENDACAO: verificar se ha lampada ligada desnecessariamente.");
  }

  if (agora - ecoUltimoRelatorioMs >= 2000UL) {
    ecoUltimoRelatorioMs = agora;
    ecoImprimirEstado();
  }
}

void ecoImprimirEstado() {
  unsigned long tempo = ecoLuzConfirmada ? (millis() - ecoInicioLuzMs) / 1000UL : 0UL;
  Serial.printf("[ECO] ADC=%d | Luz=%d%% | Estado=%s | Tempo=%02lu:%02lu | IA=%s | Alerta=%s\n",
                ecoLeitura, ecoPercentual,
                ecoCalibrado() ? (ecoLuzConfirmada ? "CLARO" : "ESCURO") : "SEM CALIBRACAO",
                (unsigned long)(tempo/60UL), (unsigned long)(tempo%60UL),
                ecoNomeClasse(ecoClasseIA), ecoAlertaAtivo ? "SIM" : "NAO");
}

// Bip curto a cada 5s apos o tempo limite, SEM delay() nem novo buzzer.
void ecoAlertaSonoro(bool outroSomAtivo) {
  unsigned long agora = millis();
  if (!ecoAlertaAtivo || outroSomAtivo) {
    if (ecoBipAtivo) { noTone(BUZZER); ecoBipAtivo = false; }
    return;
  }
  if (ecoBipAtivo) {
    if (agora - ecoInicioBipMs >= ECO_DURACAO_BIP_MS) {
      noTone(BUZZER);
      ecoBipAtivo = false;
    }
  } else if (agora - ecoUltimoBipMs >= ECO_INTERVALO_BIP_MS) {
    tone(BUZZER, 1400);
    ecoBipAtivo = true;
    ecoInicioBipMs = agora;
    ecoUltimoBipMs = agora;
  }
}

// Mostra status no OLED em janelas curtas para preservar as carinhas.
void ecoAtualizarOLED() {
  if (!displayOk) return;
  unsigned long agora = millis();
  bool mostrar = ((agora % 10000UL) < (ecoAlertaAtivo ? 5000UL : 2500UL));
  if (!mostrar) {
    if (ecoTelaAtiva) {
      ecoTelaAtiva = false;
      desenharFace(currentFace);
      lastDrawnFace = currentFace;
    }
    return;
  }
  if (ecoTelaAtiva && agora - ecoUltimaTelaMs < 600UL) return;
  ecoTelaAtiva = true;
  ecoUltimaTelaMs = agora;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("ECOBOT  |  CITIZENTECH");
  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
  display.setCursor(0, 17);
  if (!ecoCalibrado()) {
    display.println("CALIBRAR SENSOR LDR");
    display.println("Serial: e e depois c");
    display.println("115200 baud");
  } else {
    unsigned long tempo = ecoLuzConfirmada ? (agora - ecoInicioLuzMs) / 1000UL : 0UL;
    display.printf("Luz: %d%%  ADC:%d\n", ecoPercentual, ecoLeitura);
    display.printf("Tempo: %02lu:%02lu\n", (unsigned long)(tempo / 60UL),
                   (unsigned long)(tempo % 60UL));
    display.print("IA: ");
    display.println(ecoNomeClasse(ecoClasseIA));
    display.println(ecoAlertaAtivo ? "ALERTA: REVER LUZ" : "Monitorando...");
  }
  display.display();
}
