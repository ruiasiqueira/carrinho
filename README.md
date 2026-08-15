# Documentação Técnica — Construção de Carrinho

## 1. Identificação do Projeto

**Projeto:** Construção de Carrinho
**Tipo:** Carrinho robótico
**Documentação:** Ficha de requisitos, posicionamento dos componentes e croqui do chassi.

### Integrantes

| Nome                              |       RM |
| --------------------------------- | -------: |
| **Rui Amorim Siqueira**           |  RM98436 |
| **Luigi Ferrara Sinno**           |  RM98047 |
| **Davi Passanha de Sousa Guerra** | RM551605 |
| **Cauã Gonçalves de Jesus**       |  RM97648 |
| **Luan Silveira Macea**           |  RM98290 |

---

# 2. Ficha de Requisitos

## 2.1 Dimensões do chassi

Como as dimensões externas definitivas do chassi ainda não foram especificadas, foi adotada inicialmente uma dimensão de referência de **20 cm × 15 cm**.

| Característica        | Especificação        |
| --------------------- | -------------------- |
| Comprimento do chassi | 20 cm                |
| Largura do chassi     | 15 cm                |
| Espessura sugerida    | 3 mm                 |
| Formato               | Retangular           |
| Carenagem             | Sim, removível       |
| Material              | A definir pelo grupo |

> **Observação:** as dimensões de 20 × 15 cm são uma proposta inicial e deverão ser validadas durante a montagem.

---

## 2.2 Motores

O carrinho utilizará **2 motores**, sendo:

* 1 motor esquerdo;
* 1 motor direito.

Dimensões aproximadas de cada motor:

* Comprimento: 7,1 cm;
* Largura: 3,7 cm;
* Altura: 2,3 cm.

Os motores serão fixados nas laterais do chassi utilizando **parafusos em suportes/mancais**.

---

## 2.3 Placa Controladora

Será utilizada uma placa **Arduino/ESP32** como unidade de controle do carrinho.

Dimensões aproximadas:

* Comprimento: 5,2 cm;
* Largura: 2,9 cm;
* Altura: 1,2 cm.

A placa será posicionada na região central do chassi e fixada utilizando **parafusos e espaçadores (standoffs)**.

---

## 2.4 Ponte H

A Ponte H será responsável pelo controle dos motores.

Dimensões aproximadas:

* Comprimento: 4,4 cm;
* Largura: 4,4 cm;
* Altura: 2,7 cm.

Será instalada próxima aos motores, facilitando a conexão elétrica e a organização dos cabos.

A fixação será feita por **parafusos e espaçadores**.

---

## 2.5 Bateria

Dimensões aproximadas:

* Comprimento: 1,8 cm;
* Largura: 1,8 cm;
* Altura: 6,56 cm.

A bateria ficará na região central/traseira do carrinho, próxima ao seu suporte.

---

## 2.6 Suporte da Bateria

Dimensões aproximadas:

* Comprimento: 7,6 cm;
* Largura: 2,1 cm;
* Altura: 2,0 cm.

O suporte será fixado ao chassi utilizando **parafusos** ou, alternativamente, **fita dupla-face/Velcro**.

---

## 2.7 Sensor

Dimensões aproximadas:

* Comprimento: 4,6 cm;
* Largura: 2,6 cm;
* Altura: 1,6 cm.

O sensor será instalado na **parte frontal e central do carrinho**, permitindo uma área de detecção adequada.

A fixação poderá ser feita utilizando **parafusos ou fita dupla-face**.

---

# 3. Posicionamento dos Componentes

A disposição inicial dos componentes será:

* **Sensor:** parte frontal e central;
* **Motor esquerdo:** lateral esquerda;
* **Motor direito:** lateral direita;
* **Arduino/ESP32:** região central;
* **Ponte H:** região central/traseira;
* **Bateria:** região traseira/central;
* **Suporte da bateria:** junto à bateria;
* **Carenagem:** cobertura superior removível.

A disposição busca manter o carrinho equilibrado, facilitar a manutenção e reduzir a quantidade de fios entre os componentes.

---

# 4. Croqui do Chassi

### Vista superior — proposta inicial

```text
                    FRENTE
                      ↑
        ┌─────────────────────────┐
        │                         │
        │        ┌─────────┐      │
        │        │ SENSOR  │      │
        │        └─────────┘      │
        │                         │
        │ ┌──────┐       ┌──────┐ │
        │ │MOTOR │       │MOTOR │ │
        │ │ ESQ. │       │ DIR. │ │
        │ └──────┘       └──────┘ │
        │                         │
        │     ┌─────────────┐     │
        │     │  ARDUINO /  │     │
        │     │    ESP32     │     │
        │     └─────────────┘     │
        │                         │
        │      ┌───────────┐      │
        │      │  PONTE H  │      │
        │      └───────────┘      │
        │                         │
        │    ┌───────────────┐    │
        │    │    BATERIA    │    │
        │    │  + SUPORTE    │    │
        │    └───────────────┘    │
        │                         │
        └─────────────────────────┘
             Largura: 15 cm
             
             Comprimento: 20 cm
```

### Vista lateral

```text
                 CARENAGEM
        ┌─────────────────────────┐
        │                         │
        │   Arduino / ESP32       │
        │                         │
        └─────────────────────────┘
        ┌─────────────────────────┐
        │         CHASSI          │
        └─────────────────────────┘
           O                   O
         RODA                RODA
```

---

# 5. Carenagem

O carrinho terá uma **carenagem superior removível**, responsável por proteger os componentes eletrônicos.

A cobertura deverá:

* proteger o Arduino/ESP32 e a Ponte H;
* evitar contato acidental com os componentes;
* permitir ventilação;
* permitir acesso para manutenção;
* possuir espaço para passagem dos cabos;
* não interferir no movimento das rodas e motores.

---

# 6. Lista de Componentes

| Quantidade | Componente         | Dimensões aproximadas |
| ---------: | ------------------ | --------------------- |
|          2 | Motores            | 7,1 × 3,7 × 2,3 cm    |
|          1 | Arduino/ESP32      | 5,2 × 2,9 × 1,2 cm    |
|          1 | Ponte H            | 4,4 × 4,4 × 2,7 cm    |
|          2 | Bateria            | 1,8 × 1,8 × 6,56 cm   |
|          2 | Suporte da bateria | 7,6 × 2,1 × 2,0 cm    |
|          1 | Sensor             | 4,6 × 2,6 × 1,6 cm    |

---

# 7. Requisitos Funcionais

1. O carrinho deverá possuir dois motores.
2. Os motores deverão ser controlados pela Ponte H.
3. O Arduino/ESP32 deverá controlar o sistema.
4. O sensor deverá ficar localizado na parte frontal.
5. A bateria deverá alimentar o sistema.
6. Os componentes deverão estar firmemente fixados ao chassi.
7. A carenagem deverá proteger os componentes eletrônicos.
8. A carenagem deverá ser removível para manutenção.
9. Os motores deverão permanecer alinhados.
10. A fiação deverá ser organizada e protegida contra partes móveis.

---

# 8. Métodos de Fixação

| Componente         | Método de fixação                   |
| ------------------ | ----------------------------------- |
| Motor esquerdo     | Parafusos em suporte/mancal         |
| Motor direito      | Parafusos em suporte/mancal         |
| Arduino/ESP32      | Parafusos + espaçadores             |
| Ponte H            | Parafusos + espaçadores             |
| Bateria            | Suporte da bateria                  |
| Suporte da bateria | Parafusos ou fita dupla-face/Velcro |
| Sensor             | Parafusos ou fita dupla-face        |
| Carenagem          | Parafusos removíveis                |

---


## 11. Integrantes

**Rui Amorim Siqueira — RM98436**
**Luigi Ferrara Sinno — RM98047**
**Davi Passanha de Sousa Guerra — RM551605**
**Cauã Gonçalves de Jesus — RM97648**
**Luan Silveira Macea — RM98290**
