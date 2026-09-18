#include <DabbleESP32.h>

// =====================================================
// CONFIGURAÇÕES GERAIS
// =====================================================

#define BLUETOOTH_NAME "My Bluetooth Car"

// Sensores
#define ULTRASONIC_ENABLED true
#define BUZZER_ENABLED true

// Distância mínima de segurança
#define OBSTACLE_DISTANCE_CM 20


// =====================================================
// L298N - MOTORES
// =====================================================

// Lado esquerdo
#define IN1 13
#define IN2 4

// Lado direito
#define IN3 16
#define IN4 17


// =====================================================
// HC-SR04
// =====================================================

#define TRIG_PIN 27
#define ECHO_PIN 26


// =====================================================
// BUZZER
// =====================================================

#define BUZZER_PIN 25


// =====================================================
// INVERSÃO DOS MOTORES
// =====================================================

// Caso algum lado fique invertido após a montagem,
// basta trocar false por true.

#define LEFT_INVERTED false
#define RIGHT_INVERTED false


// =====================================================
// VARIÁVEIS
// =====================================================

float distanceCm = 999.0;

bool obstacleDetected = false;

bool buzzerState = false;

unsigned long lastDistanceCheck = 0;
unsigned long lastBuzzerUpdate = 0;

String lastCommand = "";


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  // ---------------------------------------------------
  // MOTORES
  // ---------------------------------------------------

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // ---------------------------------------------------
  // HC-SR04
  // ---------------------------------------------------

  if (ULTRASONIC_ENABLED) {

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    digitalWrite(TRIG_PIN, LOW);
  }

  // ---------------------------------------------------
  // BUZZER
  // ---------------------------------------------------

  if (BUZZER_ENABLED) {

    pinMode(BUZZER_PIN, OUTPUT);

    digitalWrite(BUZZER_PIN, LOW);
  }

  // ---------------------------------------------------
  // BLUETOOTH
  // ---------------------------------------------------

  Dabble.begin(BLUETOOTH_NAME);

  // ---------------------------------------------------
  // SEGURANÇA
  // ---------------------------------------------------

  stopMotors();

  // ---------------------------------------------------
  // SERIAL
  // ---------------------------------------------------

  Serial.println();
  Serial.println("========================================");
  Serial.println("          CARRINHO ESP32");
  Serial.println("========================================");

  Serial.println("Sistema iniciado.");
  Serial.println("Bluetooth: My Bluetooth Car");

  if (ULTRASONIC_ENABLED) {
    Serial.println("HC-SR04: ATIVO");
  } else {
    Serial.println("HC-SR04: DESATIVADO");
  }

  if (BUZZER_ENABLED) {
    Serial.println("BUZZER: ATIVO");
  } else {
    Serial.println("BUZZER: DESATIVADO");
  }

  Serial.print("Distancia de seguranca: ");
  Serial.print(OBSTACLE_DISTANCE_CM);
  Serial.println(" cm");

  Serial.println("========================================");
}


// =====================================================
// LOOP PRINCIPAL
// =====================================================

void loop() {

  // ---------------------------------------------------
  // DABBLE
  // ---------------------------------------------------

  Dabble.processInput();


  // ---------------------------------------------------
  // HC-SR04
  // ---------------------------------------------------

  if (ULTRASONIC_ENABLED) {

    if (millis() - lastDistanceCheck >= 100) {

      lastDistanceCheck = millis();

      distanceCm = readDistance();

      if (distanceCm > 0 &&
          distanceCm <= OBSTACLE_DISTANCE_CM) {

        obstacleDetected = true;

      } else {

        obstacleDetected = false;
      }
    }
  }


  // ---------------------------------------------------
  // BUZZER
  // ---------------------------------------------------

  updateBuzzer();


  // ---------------------------------------------------
  // CONTROLE DO CARRINHO
  // ---------------------------------------------------

  // FRENTE
  if (GamePad.isUpPressed()) {

    // Não permite avançar contra obstáculo

    if (ULTRASONIC_ENABLED &&
        obstacleDetected) {

      stopMotors();

      printCommand("OBSTACULO - BLOQUEADO");

    } else {

      moveForward();

      printCommand("FRENTE");
    }
  }


  // RÉ
  else if (GamePad.isDownPressed()) {

    moveBackward();

    printCommand("RE");
  }


  // ESQUERDA
  else if (GamePad.isLeftPressed()) {

    turnLeft();

    printCommand("ESQUERDA");
  }


  // DIREITA
  else if (GamePad.isRightPressed()) {

    turnRight();

    printCommand("DIREITA");
  }


  // NENHUM COMANDO
  else {

    stopMotors();

    printCommand("PARADO");
  }

  delay(10);
}


// =====================================================
// FRENTE
// =====================================================

void moveForward() {

  setLeftMotor(true);
  setRightMotor(true);
}


// =====================================================
// RÉ
// =====================================================

void moveBackward() {

  setLeftMotor(false);
  setRightMotor(false);
}


// =====================================================
// ESQUERDA
// =====================================================

void turnLeft() {

  setLeftMotor(false);
  setRightMotor(true);
}


// =====================================================
// DIREITA
// =====================================================

void turnRight() {

  setLeftMotor(true);
  setRightMotor(false);
}


// =====================================================
// PARAR
// =====================================================

void stopMotors() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}


// =====================================================
// MOTOR ESQUERDO
// =====================================================

void setLeftMotor(bool forward) {

  bool direction = forward;

  if (LEFT_INVERTED) {
    direction = !direction;
  }

  if (direction) {

    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

  } else {

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }
}


// =====================================================
// MOTOR DIREITO
// =====================================================

void setRightMotor(bool forward) {

  bool direction = forward;

  if (RIGHT_INVERTED) {
    direction = !direction;
  }

  if (direction) {

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

  } else {

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }
}


// =====================================================
// LEITURA DO HC-SR04
// =====================================================

float readDistance() {

  // Início do pulso
  digitalWrite(TRIG_PIN, LOW);

  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);

  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);


  // Mede o retorno
  long duration = pulseIn(
    ECHO_PIN,
    HIGH,
    30000
  );


  // Se não houve retorno
  if (duration == 0) {

    return 999.0;
  }


  // Converte para centímetros
  float distance =
    duration * 0.0343 / 2.0;


  return distance;
}


// =====================================================
// BUZZER
// =====================================================

void updateBuzzer() {

  if (!BUZZER_ENABLED) {

    return;
  }


  if (obstacleDetected) {

    // Liga/desliga a cada 200 ms

    if (millis() - lastBuzzerUpdate >= 200) {

      lastBuzzerUpdate = millis();

      buzzerState = !buzzerState;

      digitalWrite(
        BUZZER_PIN,
        buzzerState
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


// =====================================================
// SERIAL - COMANDOS
// =====================================================

void printCommand(String command) {

  if (command != lastCommand) {

    Serial.print("Comando: ");
    Serial.println(command);

    lastCommand = command;
  }
}