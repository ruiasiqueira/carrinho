# 🚗 Carrinho Robótico 4WD com ESP32

## 1. Identificação do Projeto

**Projeto:** Construção de Carrinho Robótico
**Tipo:** Carrinho robótico 4WD
**Plataforma:** ESP32
**Controle:** Bluetooth via aplicativo Dabble

Este projeto consiste na construção de um carrinho robótico com quatro motores DC e quatro rodas, controlado por um ESP32. O sistema utiliza Bluetooth para controle remoto através do aplicativo Dabble, uma Ponte H L298N para acionamento dos motores, um sensor ultrassônico HC-SR04 para detecção de obstáculos e um buzzer para alertas.

---

## 2. Integrantes

| Nome | RM |
|---|---:|
| **Rui Amorim Siqueira** | RM98436 |
| **Luigi Ferrara Sinno** | RM98047 |
| **Davi Passanha de Sousa Guerra** | RM551605 |
| **Cauã Gonçalves de Jesus** | RM97648 |
| **Luan Silveira Macea** | RM98290 |

---

# PARTE 1 — SOFTWARE

## 3. Visão Geral do Software

O ESP32 é responsável por receber comandos do aplicativo Dabble através de Bluetooth/BLE e controlar os quatro motores por meio da Ponte H L298N.

O software também possui integração com:

- Sensor ultrassônico HC-SR04;
- Buzzer ativo;
- Sistema de proteção contra obstáculos;
- Configuração para inversão do sentido dos motores.

### Arquitetura

```text
                    CELULAR
                       │
                    DABBLE
                       │
                Bluetooth / BLE
                       │
                       ▼
                    ESP32
             ┌─────────┼─────────┐
             │         │         │
             ▼         ▼         ▼
           L298N     HC-SR04   BUZZER
             │
       ┌─────┴─────┐
       │           │
  2 motores    2 motores
  esquerda      direita
       │           │
       ▼           ▼
   2 rodas       2 rodas
```

---

## 4. Software Necessário

Para executar o projeto, é necessário ter instalado:

- [Arduino IDE](https://www.arduino.cc/en/software)
- Pacote ESP32 para Arduino
- Driver CP2102 USB to UART
- Biblioteca DabbleESP32
- Aplicativo Dabble no celular

---

## 5. Instalação do Arduino IDE

Instalar o Arduino IDE no computador.

Após a instalação, abrir o programa.

---

## 6. Instalação do ESP32

No Arduino IDE:

**File → Preferences**

Em **Additional Boards Manager URLs**, adicionar:

```text
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

Depois acessar:

**Tools → Board → Boards Manager**

Pesquisar:

```text
esp32
```

Instalar:

```text
ESP32 by Espressif Systems
```

---

## 7. Driver USB do ESP32

A placa utilizada neste projeto possui o conversor:

```text
CP2102 USB to UART
```

Com o ESP32 conectado ao computador, o Windows deverá reconhecer uma porta semelhante a:

```text
Silicon Labs CP210x USB to UART Bridge (COMx)
```

Essa porta será utilizada pelo Arduino IDE para comunicação com o ESP32.

---

## 8. Configuração da Placa

Com o ESP32 conectado ao computador:

```text
Tools
  ↓
Board
  ↓
ESP32 Arduino
  ↓
ESP32 Dev Module
```

Depois:

```text
Tools
  ↓
Port
  ↓
COMx
```

Selecionar a porta correspondente ao ESP32.

---

## 9. Biblioteca DabbleESP32

No Arduino IDE:

```text
Sketch
  ↓
Include Library
  ↓
Manage Libraries
```

Pesquisar:

```text
DabbleESP32
```

Instalar a biblioteca.

A biblioteca é responsável pela comunicação entre o aplicativo Dabble e o ESP32.

---

## 10. Aplicativo Dabble

No celular:

1. Instalar o aplicativo **Dabble**.
2. Ativar o Bluetooth.
3. Abrir o aplicativo.
4. Procurar pelo dispositivo:

```text
My Bluetooth Car
```

5. Conectar ao dispositivo.
6. Abrir o módulo:

```text
GamePad
```

---

## 11. Código do Projeto

O código-fonte principal do carrinho está armazenado neste repositório Git.

O programa implementa:

- Comunicação Bluetooth;
- Controle por GamePad;
- Controle dos quatro motores;
- Controle da Ponte H L298N;
- Leitura do sensor HC-SR04;
- Detecção de obstáculos;
- Acionamento do buzzer;
- Configuração de inversão dos motores.

O código não é reproduzido neste README, pois está armazenado no repositório do projeto.

---

## 12. GPIOs Utilizados

### 12.1 Ponte H L298N

| ESP32 | L298N | Função |
| --- | --- | --- |
| GPIO 13 | IN1 | Controle do lado esquerdo |
| GPIO 4 | IN2 | Controle do lado esquerdo |
| GPIO 16 | IN3 | Controle do lado direito |
| GPIO 17 | IN4 | Controle do lado direito |
| GND | GND | Referência comum |

### 12.2 HC-SR04

| ESP32 | HC-SR04 | Função |
| --- | --- | --- |
| GPIO 27 | TRIG | Disparo do sensor |
| GPIO 26 | ECHO | Leitura do retorno |

### 12.3 Buzzer

| ESP32 | Buzzer | Função |
| --- | --- | --- |
| GPIO 25 | Controle | Acionamento do buzzer |

---

## 13. Execução do Software

### Passo 1 — Conectar o ESP32

```text
ESP32 → USB → Notebook
```

### Passo 2 — Abrir o Arduino IDE

Abrir o projeto e o código armazenado no Git.

### Passo 3 — Selecionar a placa

```text
Tools → Board → ESP32 Dev Module
```

### Passo 4 — Selecionar a porta

```text
Tools → Port → COMx
```

### Passo 5 — Abrir o código

Abrir o código-fonte armazenado no Git do projeto.

### Passo 6 — Compilar

Clicar em:

```text
Verify
```

### Passo 7 — Fazer upload

Clicar em:

```text
Upload
```

### Passo 8 — Abrir o Serial Monitor

Abrir:

```text
Tools → Serial Monitor
```

Configurar:

```text
115200 baud
```

### Passo 9 — Conectar ao Dabble

Abrir o aplicativo Dabble e conectar ao:

```text
My Bluetooth Car
```

Depois acessar:

```text
GamePad
```

---

## 14. Comandos do Carrinho

| Comando | Função |
| --- | --- |
| ↑ | Frente |
| ↓ | Ré |
| ← | Direita/Esquerda conforme o controle |
| → | Direita/Esquerda conforme o controle |
| Soltar botão | Parar |

### Controle de direção

```text
↑ → FRENTE
↓ → RÉ
← → ESQUERDA
→ → DIREITA
```

---

## 15. Sistema de Segurança

O sensor ultrassônico verifica a distância entre o carrinho e possíveis obstáculos.

A distância configurada no software é:

```text
20 cm
```

### Caminho livre

```text
Distância > 20 cm
        ↓
Movimento normal
```

### Obstáculo detectado

```text
Distância ≤ 20 cm
        ↓
Avanço bloqueado
        ↓
Buzzer acionado
```

A ré e os movimentos laterais permanecem disponíveis para permitir que o carrinho se afaste do obstáculo.

---

# PARTE 2 — HARDWARE

## 16. Ficha de Requisitos

### 16.1 Dimensões do Chassi

Como as dimensões externas definitivas do chassi ainda não foram especificadas, foi adotada inicialmente uma dimensão de referência de **20 cm × 15 cm**.

| Característica | Especificação |
| --- | --- |
| Comprimento do chassi | **20 cm** |
| Largura do chassi | **15 cm** |
| Espessura sugerida | **3 mm** |
| Formato | Retangular |
| Carenagem | Sim, removível |
| Material | A definir pelo grupo |

> **Observação:** as dimensões de 20 × 15 cm são uma proposta inicial e deverão ser validadas durante a montagem.

---

## 17. Lista de Componentes

| Quantidade | Componente | Dimensões aproximadas |
| ---: | --- | --- |
| 4 | Motores DC | **7,1 × 3,7 × 2,3 cm** |
| 4 | Rodas de borracha | A definir |
| 1 | ESP32 | **5,2 × 2,9 × 1,2 cm** |
| 1 | Ponte H L298N | **4,4 × 4,4 × 2,7 cm** |
| 1 | Suporte para 4 pilhas AA | **7,6 × 2,1 × 2,0 cm** |
| 4 | Pilhas AA | **1,8 × 1,8 × 6,56 cm** |
| 1 | HC-SR04 | **4,6 × 2,6 × 1,6 cm** |
| 1 | Suporte para HC-SR04 | A definir |
| 4 | Discos de encoder | A definir |
| 1 | Buzzer ativo 5 V | A definir |
| 1 | Mini protoboard | A definir |
| 1 | Cabo USB | — |
| — | Jumpers | — |
| — | Fios | — |
| — | Parafusos | — |
| — | Porcas | — |
| — | Espaçadores | — |

---

## 18. Motores

O carrinho utilizará **4 motores DC**, sendo:

- Motor dianteiro esquerdo;
- Motor traseiro esquerdo;
- Motor dianteiro direito;
- Motor traseiro direito.

### Dimensões aproximadas

- **Comprimento:** 7,1 cm
- **Largura:** 3,7 cm
- **Altura:** 2,3 cm

Os motores serão fixados nas laterais do chassi utilizando suportes, parafusos e elementos de fixação disponíveis.

---

## 19. Rodas

Serão utilizadas **4 rodas de borracha**, uma para cada motor.

### Disposição

```text
                   FRENTE

          E1                 D1
          O                  O
          │                  │
          │                  │
          E2                 D2
          O                  O

                    TRÁS
```

Os motores deverão permanecer alinhados para garantir estabilidade e movimentação adequada.

---

## 20. ESP32

O ESP32 será responsável pelo controle central do carrinho.

### Dimensões aproximadas

- **Comprimento:** 5,2 cm
- **Largura:** 2,9 cm
- **Altura:** 1,2 cm

### Funções

- Receber comandos via Bluetooth;
- Controlar os motores;
- Ler o sensor ultrassônico;
- Controlar o buzzer;
- Processar futuramente os encoders.

A placa deverá ser posicionada na região central do chassi e fixada utilizando parafusos e espaçadores.

---

## 21. Ponte H L298N

A Ponte H L298N será responsável pelo controle dos quatro motores.

### Dimensões aproximadas

- **Comprimento:** 4,4 cm
- **Largura:** 4,4 cm
- **Altura:** 2,7 cm

A placa deverá ser posicionada próxima aos motores para facilitar a organização da fiação.

### Organização dos motores

#### Canal A — lado esquerdo

```text
OUT1 / OUT2
     ↓
Motor esquerdo dianteiro
Motor esquerdo traseiro
```

#### Canal B — lado direito

```text
OUT3 / OUT4
     ↓
Motor direito dianteiro
Motor direito traseiro
```

---

## 22. Bateria

O sistema utilizará um suporte para **4 pilhas AA**.

### Dimensões aproximadas das pilhas

- **Comprimento:** 6,56 cm
- **Largura:** 1,8 cm
- **Altura/diâmetro:** 1,8 cm

A bateria será posicionada na região traseira/central do carrinho.

---

## 23. Suporte da Bateria

### Dimensões aproximadas

- **Comprimento:** 7,6 cm
- **Largura:** 2,1 cm
- **Altura:** 2,0 cm

O suporte poderá ser fixado utilizando:

- Parafusos;
- Fita dupla-face;
- Velcro.

---

## 24. Ligação ESP32 → L298N

```text
ESP32                  L298N

GPIO 13 ────────────── IN1
GPIO 4  ────────────── IN2

GPIO 16 ────────────── IN3
GPIO 17 ────────────── IN4

GND ────────────────── GND
```

### ⚠️ Importante

O **GND do ESP32 e o GND da L298N devem estar conectados em comum**.

---

## 25. Ligação dos Motores

### Lado esquerdo

```text
OUT1 ───┬── Motor esquerdo dianteiro
        │
        └── Motor esquerdo traseiro

OUT2 ───┬── Motor esquerdo dianteiro
        │
        └── Motor esquerdo traseiro
```

### Lado direito

```text
OUT3 ───┬── Motor direito dianteiro
        │
        └── Motor direito traseiro

OUT4 ───┬── Motor direito dianteiro
        │
        └── Motor direito traseiro
```

---

## 26. Jumpers ENA e ENB

Caso a L298N possua os jumpers:

```text
ENA
ENB
```

eles deverão permanecer instalados durante o primeiro teste.

Nesta versão do projeto, o controle PWM de velocidade não será utilizado.

---

## 27. Alimentação dos Motores

O suporte de 4 pilhas AA alimentará os motores através da L298N.

```text
Suporte 4xAA

(+) ─────────── L298N VIN / 12V
(-) ─────────── L298N GND
```

A tensão de alimentação deve permanecer compatível com os motores utilizados.

### Alimentação do ESP32 durante o primeiro teste

```text
Notebook
   │
  USB
   │
   ▼
 ESP32
```

Enquanto:

```text
4x AA
  │
  ▼
L298N
  │
  ▼
Motores
```

---

## 28. Sensor Ultrassônico HC-SR04

O sensor será instalado na parte frontal e central do carrinho.

### Dimensões aproximadas

- **Comprimento:** 4,6 cm
- **Largura:** 2,6 cm
- **Altura:** 1,6 cm

### Função

Detectar obstáculos localizados à frente do carrinho.

### GPIOs

```text
TRIG → GPIO 27
ECHO → GPIO 26
```

### ⚠️ Atenção ao ECHO

No HC-SR04 tradicional, o sinal `ECHO` pode operar em 5 V, enquanto o ESP32 utiliza lógica de 3,3 V.

Por isso, o sinal `ECHO` deverá passar por um **divisor de tensão ou level shifter** antes de chegar ao GPIO 26.

### Exemplo de divisor de tensão

```text
HC-SR04 ECHO
      │
     1 kΩ
      │
      ├────────── GPIO 26
      │
     2 kΩ
      │
     GND
```

**Não conectar diretamente um sinal ECHO de 5 V ao GPIO do ESP32.**

---

## 29. Buzzer

O buzzer ativo de 5 V será utilizado para alertar quando um obstáculo for detectado.

### GPIO

```text
GPIO 25
```

A forma definitiva de acionamento deverá ser validada conforme o modelo do buzzer utilizado.

Caso seja necessário fornecer mais corrente do que o GPIO do ESP32 suporta, deverá ser utilizado um transistor para realizar o acionamento.

---

## 30. Encoders

O projeto possui **4 discos de encoder**.

Eles poderão ser utilizados futuramente para:

- Medição de velocidade;
- Medição de distância percorrida;
- Controle de velocidade;
- Odometria.

Os discos, isoladamente, não realizam a medição.

É necessário utilizar sensores capazes de detectar a rotação dos discos.

A implementação dos encoders será realizada após a identificação dos sensores disponíveis no kit.

---

## 31. Posicionamento dos Componentes

A disposição inicial dos componentes será:

- **HC-SR04:** parte frontal e central;
- **Motor esquerdo dianteiro:** lateral esquerda dianteira;
- **Motor esquerdo traseiro:** lateral esquerda traseira;
- **Motor direito dianteiro:** lateral direita dianteira;
- **Motor direito traseiro:** lateral direita traseira;
- **ESP32:** região central;
- **Ponte H:** região central/traseira;
- **Bateria:** região traseira/central;
- **Suporte da bateria:** junto à bateria;
- **Buzzer:** região protegida e audível;
- **Carenagem:** cobertura superior removível.

A disposição busca manter o carrinho equilibrado, facilitar a manutenção e reduzir a quantidade de fios entre os componentes.

---

## 32. Croqui do Chassi

### Vista superior

```text
                       FRENTE
                         ↑

          ┌─────────────────────────┐
          │                         │
          │       ┌─────────┐       │
          │       │ HC-SR04 │       │
          │       └─────────┘       │
          │                         │
          │  O                   O  │
          │ E1                  D1  │
          │                         │
          │    ┌─────────────┐      │
          │    │    ESP32    │      │
          │    └─────────────┘      │
          │                         │
          │    ┌─────────────┐      │
          │    │    L298N    │      │
          │    └─────────────┘      │
          │                         │
          │  O                   O  │
          │ E2                  D2  │
          │                         │
          │    ┌─────────────┐      │
          │    │   BATERIA   │      │
          │    │   4 x AA    │      │
          │    └─────────────┘      │
          │                         │
          └─────────────────────────┘

              Largura: 15 cm
              Comprimento: 20 cm
```

---

## 33. Vista Lateral

```text
                 CARENAGEM
        ┌─────────────────────────┐
        │                         │
        │      ESP32 / L298N      │
        │                         │
        └─────────────────────────┘
        ┌─────────────────────────┐
        │         CHASSI          │
        └─────────────────────────┘
           O                   O
         RODA                RODA
```

---

## 34. Carenagem

O carrinho terá uma **carenagem superior removível**, responsável pela proteção dos componentes eletrônicos.

A carenagem deverá:

- Proteger o ESP32 e a L298N;
- Evitar contato acidental com os componentes;
- Permitir ventilação;
- Permitir acesso para manutenção;
- Possuir espaço para passagem dos cabos;
- Não interferir no movimento das rodas e motores.

---

## 35. Ordem de Montagem

A montagem deverá seguir a seguinte sequência.

### Etapa 1 — Chassi

Fixar:

- Placas do chassi;
- Espaçadores;
- Quatro motores;
- Quatro rodas.

### Etapa 2 — Fixação dos componentes

Fixar:

- ESP32;
- L298N;
- Suporte da bateria;
- Suporte do HC-SR04.

### Etapa 3 — Motores

Conectar:

```text
2 motores esquerdos → Canal A
2 motores direitos → Canal B
```

### Etapa 4 — ESP32 → L298N

Conectar:

```text
GPIO 13 → IN1
GPIO 4  → IN2
GPIO 16 → IN3
GPIO 17 → IN4
GND     → GND
```

### Etapa 5 — Alimentação

Conectar:

```text
4x AA (+) → VIN/12V L298N
4x AA (-) → GND L298N
```

### Etapa 6 — Primeiro teste

Testar somente:

```text
ESP32 + L298N + 4 motores
```

### Etapa 7 — HC-SR04

Instalar o sensor na parte frontal.

Realizar a ligação do `ECHO` utilizando divisor de tensão ou level shifter.

### Etapa 8 — Buzzer

Instalar e conectar o buzzer.

### Etapa 9 — Teste completo

Testar:

```text
Bluetooth
   ↓
ESP32
   ↓
L298N
   ↓
4 motores
   ↓
HC-SR04
   ↓
Buzzer
```

---

## 36. Testes do Sistema

### Teste 1 — Bluetooth

Verificar se o celular encontra:

```text
My Bluetooth Car
```

### Teste 2 — Frente

Pressionar:

```text
↑
```

As quatro rodas devem girar para frente.

### Teste 3 — Ré

Pressionar:

```text
↓
```

As quatro rodas devem girar para trás.

### Teste 4 — Esquerda

Pressionar:

```text
←
```

O carrinho deve realizar a curva para a esquerda.

### Teste 5 — Direita

Pressionar:

```text
→
```

O carrinho deve realizar a curva para a direita.

### Teste 6 — Parada

Ao soltar o comando:

```text
Parar
```

Todos os motores devem ser desligados.

### Teste 7 — Obstáculo

Colocar um objeto a menos de 20 cm do HC-SR04.

Resultado esperado:

```text
Obstáculo detectado
        ↓
Avanço bloqueado
        ↓
Buzzer acionado
```

---

## 37. Correção do Sentido dos Motores

Dependendo da posição física dos motores, um dos lados pode apresentar sentido invertido.

O código possui configurações específicas para inversão dos lados esquerdo e direito.

Caso o lado esquerdo esteja invertido, alterar a configuração correspondente no código.

Caso o lado direito esteja invertido, alterar a configuração correspondente no código.

Não será necessário alterar a lógica principal do carrinho.

---

## 38. Requisitos Funcionais

1. O carrinho deverá possuir quatro motores.
2. O carrinho deverá possuir quatro rodas.
3. Os motores deverão ser controlados pela Ponte H L298N.
4. O ESP32 deverá controlar o sistema.
5. O carrinho deverá ser controlado por Bluetooth através do aplicativo Dabble.
6. O sensor ultrassônico deverá estar localizado na parte frontal.
7. O sistema deverá detectar obstáculos.
8. O avanço deverá ser bloqueado quando houver obstáculo a até 20 cm.
9. O buzzer deverá emitir um alerta em caso de obstáculo.
10. Os componentes deverão estar firmemente fixados ao chassi.
11. A carenagem deverá proteger os componentes eletrônicos.
12. A carenagem deverá ser removível para manutenção.
13. Os motores deverão permanecer alinhados.
14. A fiação deverá ser organizada e protegida contra partes móveis.
15. O sistema deverá permitir manutenção e substituição dos componentes.

---

## 39. Métodos de Fixação

| Componente | Método de fixação |
| --- | --- |
| Motor esquerdo dianteiro | Parafusos + suporte |
| Motor esquerdo traseiro | Parafusos + suporte |
| Motor direito dianteiro | Parafusos + suporte |
| Motor direito traseiro | Parafusos + suporte |
| ESP32 | Parafusos + espaçadores |
| Ponte H L298N | Parafusos + espaçadores |
| Bateria | Suporte da bateria |
| Suporte da bateria | Parafusos ou fita dupla-face/Velcro |
| HC-SR04 | Suporte do sensor |
| Buzzer | Fixação ao chassi |
| Carenagem | Parafusos removíveis |

---

## 40. Checklist — Software

- [ ] Arduino IDE instalado
- [ ] Pacote ESP32 instalado
- [ ] Driver CP2102 instalado
- [ ] Biblioteca DabbleESP32 instalada
- [ ] ESP32 reconhecido pelo Windows
- [ ] Porta COM identificada
- [ ] ESP32 Dev Module selecionado
- [ ] Código baixado/clonado do Git
- [ ] Código compilado
- [ ] Código enviado ao ESP32
- [ ] Serial Monitor testado
- [ ] Dabble instalado no celular
- [ ] Bluetooth ativado
- [ ] My Bluetooth Car localizado
- [ ] GamePad conectado
- [ ] Comandos testados

---

## 41. Checklist — Hardware

- [ ] Chassi montado
- [ ] 4 motores fixados
- [ ] 4 rodas instaladas
- [ ] ESP32 fixado
- [ ] L298N fixada
- [ ] 4 motores conectados à L298N
- [ ] GPIOs conectados
- [ ] GND comum conectado
- [ ] Suporte de bateria instalado
- [ ] 4 pilhas AA instaladas
- [ ] HC-SR04 instalado
- [ ] Divisor de tensão ou level shifter do ECHO instalado
- [ ] Buzzer instalado
- [ ] Fiação organizada
- [ ] Carenagem instalada
- [ ] Teste de movimento realizado
- [ ] Teste do sensor realizado
- [ ] Teste do buzzer realizado
- [ ] Teste completo realizado

---

## 42. Fluxo Completo de Execução

```text
INICIAR PROJETO
      │
      ▼
Instalar Arduino IDE
      │
      ▼
Instalar pacote ESP32
      │
      ▼
Instalar driver CP2102
      │
      ▼
Instalar DabbleESP32
      │
      ▼
Configurar placa e porta
      │
      ▼
Baixar/abrir código do Git
      │
      ▼
Compilar
      │
      ▼
Upload para ESP32
      │
      ▼
Testar Bluetooth
      │
      ▼
Montar chassi
      │
      ▼
Instalar 4 motores
      │
      ▼
Instalar 4 rodas
      │
      ▼
Instalar L298N
      │
      ▼
Conectar ESP32
      │
      ▼
Conectar alimentação
      │
      ▼
Testar 4 motores
      │
      ▼
Instalar HC-SR04
      │
      ▼
Instalar buzzer
      │
      ▼
Teste de obstáculos
      │
      ▼
TESTE FINAL
```

---

## 43. Observações Importantes

### Alimentação

Não conectar os motores diretamente aos GPIOs do ESP32.

Os motores deverão ser acionados através da L298N.

### GND

O ESP32 e a L298N devem possuir GND em comum.

### HC-SR04

Não conectar diretamente um sinal ECHO de 5 V ao GPIO do ESP32.

Utilizar divisor de tensão ou level shifter.

### Motores

Durante os primeiros testes, manter o carrinho suspenso para que as rodas possam girar livremente.

### Encoders

Os discos de encoder dependem de sensores de leitura para fornecer dados ao ESP32.

---

## 44. Status do Projeto

### Software

- ✅ ESP32 reconhecido pelo computador
- ✅ Driver CP2102 instalado
- ✅ Arduino IDE configurado
- ✅ Biblioteca DabbleESP32 funcionando
- ✅ Bluetooth funcionando
- ✅ Dabble funcionando
- ✅ GamePad funcionando
- ✅ Comandos de movimento testados

### Hardware

- ⬜ Montagem do chassi
- ⬜ Instalação dos 4 motores
- ⬜ Instalação das 4 rodas
- ⬜ Instalação da L298N
- ⬜ Ligação da alimentação
- ⬜ Instalação do HC-SR04
- ⬜ Instalação do buzzer
- ⬜ Teste completo
- ⬜ Implementação dos encoders
