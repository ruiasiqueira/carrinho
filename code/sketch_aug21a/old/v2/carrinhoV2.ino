#include <DabbleESP32.h>

// Após treinar e exportar a Arduino Library no Edge Impulse:
// 1. Instale o ZIP da biblioteca no Arduino IDE.
// 2. Troque o nome do cabeçalho abaixo pelo nome real do arquivo exportado.
// 3. Mude TINYML_MODEL_READY de 0 para 1.
#define TINYML_MODEL_READY 0

#if TINYML_MODEL_READY
#define EI_INFERENCING_HEADER "substituir-pelo-projeto_inferencing.h"
#include EI_INFERENCING_HEADER
#endif

// =====================================================
// CONFIGURAÇÕES GERAIS
// =====================================================

#define BLUETOOTH_NAME "My Bluetooth Car"

#define ULTRASONIC_ENABLED true
#define BUZZER_ENABLED true

#define OBSTACLE_DISTANCE_CM 20.0f
#define MAX_FEATURE_DISTANCE_CM 200.0f
#define MODEL_NEAR_PROBABILITY 0.60f

// =====================================================
// L298N - MOTORES
// =====================================================

#define IN1 13
#define IN2 4
#define IN3 16
#define IN4 17

#define LEFT_INVERTED false
#define RIGHT_INVERTED false

// =====================================================
// HC-SR04 E BUZZER
// =====================================================

#define TRIG_PIN 27
#define ECHO_PIN 26
#define BUZZER_PIN 25

// =====================================================
// VARIÁVEIS
// =====================================================

const int WINDOW_SIZE = 5;

float recentDistances[WINDOW_SIZE] = {0};
float runningSum = 0;

int sampleCount = 0;
int samplePosition = 0;

float distanceCm = 999.0f;
float averageCm = 0.0f;
float feature = 0.0f;
float nearProbability = 0.0f;

bool validEcho = false;
bool modelNear = false;
bool obstacleDetected = true;
bool buzzerState = false;

unsigned long lastDistanceCheck = 0;
unsigned long lastBuzzerUpdate = 0;

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  stopMotors();

  Dabble.begin(BLUETOOTH_NAME);

  // Saída para coleta dos dados.
  // Linhas "sem_eco" não devem ser usadas no treinamento.
  Serial.println(
    "tempo_ms,distancia_cm,feature,label,prob_perto,estado"
  );
}

// =====================================================
// PRÉ-PROCESSAMENTO
// =====================================================

void addDistance(float value) {
  if (sampleCount == WINDOW_SIZE) {
    runningSum -= recentDistances[samplePosition];
  } else {
    sampleCount++;
  }

  recentDistances[samplePosition] = value;
  runningSum += value;

  samplePosition = (samplePosition + 1) % WINDOW_SIZE;

  // Média móvel das últimas cinco leituras válidas.
  averageCm = runningSum / sampleCount;

  // Limitação entre 0 e 200 cm e normalização entre 0 e 1.
  feature =
    constrain(averageCm, 0.0f, MAX_FEATURE_DISTANCE_CM)
    / MAX_FEATURE_DISTANCE_CM;
}

// =====================================================
// CLASSIFICAÇÃO TINYML
// =====================================================

bool classifyNear(float input, float &probability) {
#if TINYML_MODEL_READY

  // Este sketch espera um modelo treinado com UMA feature:
  // a distância média normalizada entre 0 e 1.
  if (EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE != 1) {
    return true;
  }

  float values[] = {input};
  signal_t signal;

  if (numpy::signal_from_buffer(values, 1, &signal) != 0) {
    return true;
  }

  ei_impulse_result_t result = {0};

  if (run_classifier(&signal, &result, false) != EI_IMPULSE_OK) {
    return true;
  }

  bool foundNear = false;

  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    String label = String(
      ei_classifier_inferencing_categories[i]
    );

    label.toLowerCase();

    // Aceita os nomes de classe "0" ou "perto".
    if (label == "0" || label == "perto") {
      probability = result.classification[i].value;
      foundNear = true;
    }
  }

  // Se a classe "perto" não for encontrada, bloqueia por segurança.
  return !foundNear ||
         probability >= MODEL_NEAR_PROBABILITY;

#else

  // Antes de instalar a biblioteca, funciona apenas
  // como coleta de dados e proteção pelo limiar de 20 cm.
  probability = 0.0f;
  return false;

#endif
}

// =====================================================
// REGISTRO DOS DADOS
// =====================================================

void printCsv(const char *state) {
  Serial.print(millis());
  Serial.print(',');

  if (validEcho) {
    Serial.print(distanceCm, 2);
    Serial.print(',');

    Serial.print(feature, 5);
    Serial.print(',');

    // Rótulo calculado pela média móvel.
    Serial.print(
      averageCm <= OBSTACLE_DISTANCE_CM ? 0 : 1
    );
  } else {
    Serial.print("NA,NA,NA");
  }

  Serial.print(',');

#if TINYML_MODEL_READY
  if (validEcho) {
    Serial.print(nearProbability, 5);
  } else {
    Serial.print("NA");
  }
#else
  Serial.print("NA");
#endif

  Serial.print(',');
  Serial.println(state);
}

// =====================================================
// LOOP PRINCIPAL
// =====================================================

void loop() {
  Dabble.processInput();

  // Lê o sensor aproximadamente a cada 100 ms.
  if (
    ULTRASONIC_ENABLED &&
    millis() - lastDistanceCheck >= 100
  ) {
    lastDistanceCheck = millis();

    distanceCm = readDistance();

    // 999 significa ausência de eco, não distância de 999 cm.
    validEcho =
      distanceCm > 0 &&
      distanceCm != 999.0f;

    if (validEcho) {
      addDistance(distanceCm);

      nearProbability = 0.0f;
      modelNear = classifyNear(
        feature,
        nearProbability
      );

      // A proteção física de 20 cm permanece ativa,
      // mesmo quando o modelo estiver instalado.
      obstacleDetected =
        distanceCm <= OBSTACLE_DISTANCE_CM ||
        modelNear;

      printCsv(
        obstacleDetected ? "bloqueado" : "livre"
      );
    } else {
      // Sem leitura válida, bloqueia o avanço.
      obstacleDetected = true;
      printCsv("sem_eco");
    }
  }

  updateBuzzer();

  // FRENTE
  if (GamePad.isUpPressed()) {
    if (
      ULTRASONIC_ENABLED &&
      obstacleDetected
    ) {
      stopMotors();
    } else {
      moveForward();
    }
  }

  // RÉ
  else if (GamePad.isDownPressed()) {
    moveBackward();
  }

  // ESQUERDA
  else if (GamePad.isLeftPressed()) {
    turnLeft();
  }

  // DIREITA
  else if (GamePad.isRightPressed()) {
    turnRight();
  }

  // PARADO
  else {
    stopMotors();
  }

  delay(10);
}

// =====================================================
// MOVIMENTAÇÃO
// =====================================================

void moveForward() {
  setLeftMotor(true);
  setRightMotor(true);
}

void moveBackward() {
  setLeftMotor(false);
  setRightMotor(false);
}

void turnLeft() {
  setLeftMotor(false);
  setRightMotor(true);
}

void turnRight() {
  setLeftMotor(true);
  setRightMotor(false);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void setLeftMotor(bool forward) {
  bool direction =
    LEFT_INVERTED ? !forward : forward;

  digitalWrite(
    IN1,
    direction ? HIGH : LOW
  );

  digitalWrite(
    IN2,
    direction ? LOW : HIGH
  );
}

void setRightMotor(bool forward) {
  bool direction =
    RIGHT_INVERTED ? !forward : forward;

  digitalWrite(
    IN3,
    direction ? HIGH : LOW
  );

  digitalWrite(
    IN4,
    direction ? LOW : HIGH
  );
}

// =====================================================
// LEITURA DO SENSOR HC-SR04
// =====================================================

float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration =
    pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return 999.0f;
  }

  return duration * 0.0343f / 2.0f;
}

// =====================================================
// BUZZER
// =====================================================

void updateBuzzer() {
  if (!BUZZER_ENABLED) {
    return;
  }

  if (obstacleDetected) {
    if (
      millis() - lastBuzzerUpdate >= 200
    ) {
      lastBuzzerUpdate = millis();

      buzzerState = !buzzerState;

      digitalWrite(
        BUZZER_PIN,
        buzzerState ? HIGH : LOW
      );
    }
  } else {
    buzzerState = false;

    digitalWrite(
      BUZZER_PIN,
      LOW
    );
  }
}