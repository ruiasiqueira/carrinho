# EcoBot — Assistente de Uso Consciente de Iluminação Residencial

> **FIAP | Global Solution 2026.2 | Project-Based Maker Lab | CitizenTech**  

**Integrantes:**
 Rui Amorim Siqueira
 Luigi Ferara Sinno
 Cauã Gonçalves de Jesus
 Davi Passanha
 Luan Macea
**Turma:** 4ESPW 
**Repositório:** (https://github.com/ruiasiqueira/carrinho)  
**Vídeo demonstrativo (máximo de 6 minutos):** https://youtu.be/KTf69Ze1XEM?si=1_dIUUs6-AuNHhLE


## 1. Resumo executivo

O **EcoBot** reutiliza um carrinho robótico 4WD equipado com **ESP32** e adiciona o monitoramento de **claridade ambiental** por um sensor LDR. Quando a leitura analógica permanece **acima de 100 unidades ADC durante 10 segundos consecutivos**, o dispositivo recomenda verificar a iluminação, apresenta o estado no display OLED e no Monitor Serial, emite bipes repetitivos e tenta enviar um aviso ao aplicativo Dabble conectado por Bluetooth.

A solução mantém as funções de locomoção, controle remoto por Dabble e desvio de obstáculos já existentes no robô. Além disso, executa localmente um classificador de **Machine Learning k-NN (k=3)**, com **18 exemplos rotulados e sintéticos**, para apoiar a interpretação do contexto em três classes: `NORMAL`, `ATENCAO` e `REVISAR USO`.

**Precisão conceitual:** o EcoBot **não mede consumo elétrico, potência ou kWh**. O LDR não identifica se a luz vem do sol ou de uma lâmpada, nem se há pessoas no cômodo. O resultado é um **alerta preventivo de iluminação prolongada**, e não uma detecção comprovada de desperdício.

## 2. Problema, motivação e CitizenTech

Luzes mantidas acesas por períodos desnecessários podem representar oportunidades de uso mais consciente de energia. O desafio do projeto é tornar essa situação perceptível para o usuário por meio de uma interação simples entre sensores, robótica, processamento local e aplicativo móvel.

O projeto se relaciona com **CitizenTech** ao explorar tecnologia acessível para informar e orientar decisões cotidianas, buscando incentivar hábitos responsáveis. O protótipo prioriza transparência: não toma uma medição de claridade como evidência de gasto de energia, mas oferece uma recomendação de verificação.

**Objetivo geral:** adaptar um robô ESP32 preexistente para monitorar claridade de modo autônomo, classificar a situação por k-NN e emitir avisos locais e por Bluetooth.

**Objetivos específicos:** (a) integrar um novo sensor LDR; (b) preservar a operação do carrinho; (c) implementar uma regra temporal de alerta; (d) classificar as leituras por k-NN; (e) mostrar resultados no Serial e OLED; (f) disponibilizar uma forma de reconhecer o alerta; (g) documentar montagem, limitações e testes reproduzíveis.

## 3. O que o protótipo faz

| Recurso | Implementação prevista no código |
|---|---|
| Sensor de luz | Leitura analógica `analogRead(GPIO27)`, a cada ~250 ms |
| Início da contagem | Primeira leitura `ADC > 100` |
| Alerta | Quando `ADC > 100` persiste por 10.000 ms |
| Buzzer | GPIO 5; tom de ~1400 Hz, bip de ~260 ms a cada ~800 ms |
| Encerramento automático | Uma leitura `ADC <= 100` zera o contador e encerra o alerta |
| Reconhecimento | `SELECT` no GamePad, ou `k` no Monitor Serial/Terminal Dabble |
| Aviso no celular | Pedido de notificação via `Notification.notifyPhone(...)` quando Dabble está conectado; mensagem também enviada ao Terminal |
| IA | k-NN com 3 vizinhos mais próximos e 18 amostras sintéticas |
| Robótica original | Controle Bluetooth/GamePad, motores e desvio de obstáculos por ultrassom |

**Importante sobre o botão OK:** a implementação atual **não recebe do Dabble a confirmação do toque em “OK” na notificação do sistema**. O reconhecimento funcional ocorre por `SELECT` ou `k`. A chegada da notificação ao sistema do celular depende de versão, conexão e permissões do aplicativo, e ainda exige validação real.

## 4. Arquitetura da solução

```text
  Claridade ambiente
         |
         v
  LDR avulso (GPIO27)
         |
         v
  ESP32: analogRead() -------------------+
         |                               |
         v                               v
  Regra ADC > 100 / 10 s       Normalização didática dos dados
         |                               |
         v                               v
  Alarme e temporizador           k-NN (k = 3)
         |                               |
         +--------------+----------------+
                        |
             OLED / Monitor Serial
                        |
                Dabble via Bluetooth
                        |
       Notificação / Terminal / GamePad

  Subsistema do carrinho: GamePad -> ESP32 -> ponte H -> motores
  Subsistema de navegação: ultrassom -> ESP32 -> manobras
```

As decisões de alertar e de classificar são **separadas**: o k-NN não é quem liga diretamente o buzzer. Essa separação torna o alarme demonstrativo previsível e a análise da IA observável.


## 5. Lista de Componentes

| Quantidade | Componente | Dimensões aproximadas |
| ---: | --- | --- |
| 4 | Motores DC | **7,1 × 3,7 × 2,3 cm** |
| 4 | Rodas de borracha | 
| 1 | ESP32 | **5,2 × 2,9 × 1,2 cm** |
| 1 | Ponte H L298N | **4,4 × 4,4 × 2,7 cm** |
| 1 | Suporte para 4 pilhas AA | **7,6 × 2,1 × 2,0 cm** |
| 4 | Pilhas AA | **1,8 × 1,8 × 6,56 cm** |
| 1 | HC-SR04 | **4,6 × 2,6 × 1,6 cm** |
| 1 | Suporte para HC-SR04 |
| 4 | Discos de encoder | 
| 1 | Buzzer ativo 5 V |
| 1 | Mini protoboard 
| 1 | Cabo USB-C | — |
| — | Jumpers | — |
| — | Fios | — |
| — | Parafusos | — |
| — | Porcas | — |
| — | Espaçadores | — |
| 1 | LDR
| 1 | Sensor Ultrasonico 

### Limitações e segurança da montagem do LDR

O protótipo atual conecta o **LDR entre GPIO27 e GND** e utiliza `pinMode(PINO_LDR, INPUT_PULLUP)` para tentar obter a leitura analógica sem resistor externo. **Essa é uma montagem experimental, não um circuito de medição recomendado.** Ela produziu variação de leituras em teste pontual, mas pode ter baixa repetibilidade, depender do hardware e falhar sob outras condições. O limite `100` é **local e empírico**, não uma medida de lux.

## 6. Lógica do alerta e controle de estados

1. O ESP32 lê o sensor aproximadamente quatro vezes por segundo.
2. Se `ADC > 100`, inicia a contagem de iluminação contínua, caso ainda não esteja ativa.
3. Ao atingir **10 segundos consecutivos** acima do limiar, ativa o alerta, emite bipes repetidos e tenta enviar o aviso ao Dabble conectado.
4. Se o usuário enviar **`k`** no Terminal Dabble ou no Monitor Serial, ou pressionar **`SELECT`** no GamePad, os bipes param. A confirmação permanece até escurecer.
5. Se o valor cair para `100` ou menos, o sistema interrompe o alerta, zera a contagem e fica pronto para um novo ciclo.
6. Enquanto o alerta estiver ativo e não confirmado, o código emite **lembretes textuais** aproximadamente a cada 20 segundos; a notificação no celular não é necessariamente repetida nesses lembretes.

O tempo de 10 segundos foi escolhido **para demonstração**. Não equivale a um limite validado de desperdício residencial. Para alterar, editar a constante `ECO_TEMPO_ALERTA_MS` e compilar/carregar uma nova versão.

## 7. Inteligência Artificial: k-NN no ESP32

**Abordagem:** aprendizado supervisionado por exemplos rotulados, implementado por k-Nearest Neighbors com **`k = 3`**. O algoritmo consulta uma base pequena armazenada no próprio firmware, sem exigir internet ou execução de Python no robô.

**Entradas do classificador:**

- **Índice relativo de luz:** `min(100, floor(ADC * 100 / 300))`, em escala 0–100. O valor `300` é referência didática fixa, **não calibração fotométrica**.
- **Tempo normalizado:** `min(100, floor(tempo_ms * 60 / 10000))`. Dessa forma, 10 segundos correspondem ao índice 60 e aproximadamente 16,7 segundos ao índice 100.

A base `ECO_TREINO[]` tem **18 registros sintéticos**, cada um com índice de luz, tempo normalizado e rótulo de classe (`0`, `1`, `2`). O algoritmo calcula a **distância euclidiana ao quadrado** em duas dimensões, seleciona os três menores valores e retorna a classe majoritária:

| Classe | Rótulo | Interpretação |
|---|---|---|
| 0 | `NORMAL` | Condição didática de baixo risco |
| 1 | `ATENCAO` | Condição que merece acompanhamento |
| 2 | `REVISAR USO` | Sugestão de verificar o uso da iluminação |

A planilha **`EcoBot_Treinamento_IA_kNN.xlsx`** contém as 18 amostras e um simulador do classificador para fins didáticos. Ela documenta o raciocínio, mas **não é lida pelo ESP32**: as amostras já estão no vetor `ECO_TREINO[]` dentro do `gs.ino`.

**Limitações metodológicas:** os 18 registros não foram coletados em residências reais; não há, até o momento, validação com conjunto de teste independente, métricas de precisão, nem estimativa de consumo de energia. O k-NN é uma técnica de ML genuína, mas este protótipo é uma **prova de conceito**. Sua classificação não comprova desperdício e **não governa o acionamento do buzzer**.


## 8. Pré-requisitos e instalação

- Placa compatível com o firmware: **ESP32 Dev Module** (confirmar modelo físico).
- **Arduino IDE 2.x** ou **Arduino CLI** e pacote de placas `esp32 by Espressif Systems`.
- Biblioteca `DabbleESP32` (no computador utilizado: 1.6.1).
- Bibliotecas `Adafruit GFX Library`, `Adafruit SSD1306` e dependências (`Adafruit BusIO`), além de `Wire`, fornecida pelo core/ambiente.
- Aplicativo **Dabble** para controle via Bluetooth, com permissões de notificação se disponíveis.
- Alimentação adequada para motores e para ESP32; rodas suspensas nos primeiros testes.

### Compilar e carregar no Windows

1. Baixar ou clonar o projeto, mantendo o arquivo Arduino em `gs/gs.ino`.
2. Selecionar a placa **ESP32 Dev Module** e a porta serial correta no Arduino IDE.
3. Compilar usando o IDE ou a CLI. **Substituir `COM3` caso a porta mude.**

4. Abrir o **Monitor Serial a 115200 baud**. Conectar o aplicativo Dabble a **`My Bluetooth Car`** para usar GamePad/Terminal e tentar as notificações.
5. **Não é preciso calibrar**: esta versão usa o limiar fixo de 100 ADC. Comandos `e`, `c` e `t` pertencem a versões antigas e **não estão ativos** nesta versão.

### Comandos de diagnóstico

| Comando | Função |
|---|---|
| `s` | Imprime ADC, limiar, estado, tempo, classe IA, alarme e confirmação |
| `p` | Faz um bip de teste pelo buzzer (GPIO 5) |
| `n` | Solicita uma notificação de teste ao Dabble conectado |
| `k` ou `K` | Reconhece o alerta ativo e silencia os bipes até escurecer |
| `r` | Reinicia a contagem do sensor |
| `?` | Mostra instruções no Monitor Serial |

O comando pode ser enviado pelo Monitor Serial; o **Terminal do Dabble** também encaminha caracteres ao mesmo processador de comandos. **Não confundir o clique em OK da notificação com os comandos `k`/`SELECT`.**


## 9. Avaliação de autonomia, inovação e limitações

**Autonomia:** após ligado, o EcoBot lê o sensor, conta o tempo, classifica as condições e gera avisos **sem intervenção humana**. O reconhecimento do alerta e a locomoção manual continuam sendo interações do usuário. O modo de navegação automática aproveita a rotina original de ultrassom/desvio.

**Integração:** o mesmo microcontrolador combina aquisição de sinal, classificação k-NN, temporização, acionamento do buzzer, visualização no OLED e comunicação Bluetooth, sem remover o controle original do robô.

**Criatividade:** adapta um carrinho educacional móvel como interface demonstrativa para conscientização de iluminação residencial, aproveitando hardware já existente.

**Limitações principais:** (1) LDR sem divisor resistivo externo; (2) limiar ADC empírico; (3) falsa detecção por luz solar; (4) ausência de medidor de consumo/ocupação; (5) notificações móveis dependentes do Dabble; (6) tempo de 10 s usado apenas para demonstração.
