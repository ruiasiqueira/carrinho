# EcoBot — Assistente de Uso Consciente de Energia Residencial

## 1. O que e este projeto

O arquivo `EcoBot_Energia_Final.ino` e a **versao integrada** do carrinho 4WD com ESP32, controle Bluetooth/Dabble, motor L298N, sensor ultrassonico, buzzer e display OLED. Foi acrescentado um sensor de luminosidade (LDR) e um classificador simples de aprendizado de maquina k-NN, implementado no proprio codigo sem dependencias externas de IA.

**Limite fisico importante:** o LDR observa a claridade ao seu redor. Nao e um medidor de energia, nao distingue luz solar de lampada e nao confirma se uma lampada esta ligada. O prototipo emite **alerta de iluminacao continua**, recomendando verificar uma possivel oportunidade de economia. Para monitorar o estado real de um circuito eletrico seria necessario hardware adicional seguro, com supervisao adequada.

## 2. Hardware necessario

- Carrinho pronto: ESP32, motores, ponte H, ultrassonico, buzzer e OLED (componentes do codigo original).
- **Um modulo LDR com saida analogica AO** e tres cabos jumper.
- Sem relé, lampada eletrica, LED adicional ou conexao a tomada.

**Ligacoes, somente para um modulo LDR 3,3 V com VCC/GND/AO:**

| Modulo | ESP32 |
|-------|-------|
| VCC | 3V3 |
| GND | GND |
| AO | GPIO 34 |
| DO | Deixar desconectado |

A **protoboard nao tem positivo/negativo fixos**; nunca confie na cor de um fio ou em uma marcacao de imagem ilustrativa para decidir qual linha tem 3V3/GND. Confira os rotulos reais do ESP32 e do sensor. Se seu LDR tiver pinos diferentes, pare e confirme o modelo. Desligue bateria e USB antes de mudar fios. Nunca alimente AO acima de 3,3 V.

## 3. O que acontece durante a operacao

- O LDR e lido a cada 250 milissegundos, com tolerancia a oscilacoes curtas (2 s de estabilidade) e histerese de 55% para entrar e 45% para sair.
- Quando ha claridade confirmada, o cronometro com `millis()` inicia (nao bloqueia controle Bluetooth).
- Depois de **5 minutos de claridade continua**, o ESP32 gera um aviso no Serial/OLED e o buzzer do proprio carrinho emite um bip curto a cada 5 s.
- Ao escurecer continuamente por cerca de 2 segundos, a contagem e zerada e o alerta cessa.
- O k-NN usa luminosidade e tempo, comparados com 18 exemplos sinteticos rotulados, para classificar NORMAL / ATENCAO / REVISAR USO. O tempo exato do alerta e uma regra independente da IA; isso garante que o limite de 5 minutos seja obedecido.
- O display alterna de forma periodica entre o resumo do EcoBot e as expressoes faciais originais.
- No piloto automatico, se o sensor ultrassonico perder o eco, o carro para por seguranca (melhoria em relacao ao original).

## 4. Primeiro uso: calibracao do LDR

1. Carregue o codigo no ESP32 e abra o Monitor Serial a **115200 baud**.
2. Cubra o sensor e envie a letra **`e`** (escuro).
3. Ilumine o sensor com uma lanterna e envie **`c`** (claro).
4. Envie **`s`** para ver a leitura, percentual, classificacao e contagem.
5. A calibracao fica gravada no ESP32 mesmo depois de desligar. Recalibre se trocar sensor ou mudar muito a configuracao.
6. Aponte o sensor a uma fonte de luz constante. Verifique que a contagem aumenta; cubra o sensor para zerar.

**Comandos:** `e` (calibrar escuro), `c` (claro), `s` (estado), `r` (zerar timer), `?` (ajuda). Evite deixar o LDR virado para fontes de luz variaveis durante a apresentacao.

## 5. Teste rapido (sem esperar 5 minutos)

No comeco do `.ino`, altere temporariamente:

```cpp
#define ECO_MODO_SIMULACAO 1  // permite simular claro/escuro sem o LDR
#define ECO_DEMO_RAPIDA 1     // tempo de alerta = 10 s
```

Recompile e carregue. No Monitor Serial, envie `a` para ambiente claro simulado. Apos ~12 segundos (2 s de estabilidade + 10 s de contagem), o alerta e o bip devem aparecer; envie `b` para escurecer e cessar o alerta. `s` mostra o estado.

**Antes da demonstracao final:** volte a `ECO_MODO_SIMULACAO 0` e `ECO_DEMO_RAPIDA 0`, carregue novamente e faca o teste com o sensor real por 5 min. A modalidade de 10 s deve ser explicada como aceleracao para testes, nao como o comportamento final.

## 6. Arduino IDE / alternativa pelo PowerShell

Como voce teve problemas com a verificacao da IDE, o Arduino CLI com `--jobs 1` funcionou em uma tentativa anterior. Para compilar esta **nova** versao, salve o `.ino` dentro de uma pasta com o mesmo nome:

```
Desktop/EcoBot_Energia_Final/EcoBot_Energia_Final.ino
```

Exemplo em PowerShell, ajustando o caminho se necessario:

```powershell
$cli = Get-ChildItem -Path @(
  "$env:LOCALAPPDATA\Programs\Arduino IDE",
  "$env:ProgramFiles\Arduino IDE"
) -Filter "arduino-cli.exe" -File -Recurse -ErrorAction SilentlyContinue |
  Select-Object -First 1 -ExpandProperty FullName

& $cli compile --fqbn esp32:esp32:esp32 --jobs 1 "$env:USERPROFILE\Desktop\EcoBot_Energia_Final"
```

Para enviar ao ESP32, feche o Monitor Serial, conecte a placa e confira a porta COM correta (por exemplo, COM3) antes de adicionar `--port COM3 --upload` ao comando. Mantenha as rodas suspensas no primeiro teste.

## 7. Testes minimos para documentar

| Teste | Passos | Resultado esperado |
|---|---|---|
| Calibracao | `e` cobrindo; `c` iluminando | Valores diferentes por >=200 pontos ADC |
| Escuro | Cobrir LDR | Tempo volta a 00:00, sem bip |
| Iluminacao breve | Iluminar por 1 minuto | Conta o tempo, sem bip |
| Alerta real | Manter iluminado >5 minutos | Buzzer apita a cada ~5 s, OLED/Serial alertam |
| Restaurar | Cobrir por ~2 s | Alerta para, timer zera |
| IA | Comparar luz intensa breve e prolongada | Classe observada no Serial/OLED |
| Controle | Acionar motores e botoes no Dabble | Funcoes originais mantidas |
| Obstaculo | Aproximar objeto no piloto automatico | Para/desvia; sem eco, para |

**ATENCAO:** o programa passou por verificacao sintatica de C++ com interfaces simuladas, mas nao foi compilado aqui com as bibliotecas reais do seu ESP32 nem executado no seu carrinho. A integracao so sera comprovada apos os testes fisicos.

## 8. Matriz de criterios da FIAP (slides da Global Solution 2026.2)

- **Sensor adicional:** modulo LDR fisico conectado ao ESP32, validado em teste real.
- **IA:** k-NN (3 vizinhos) com 18 amostras **sinteticas didaticas**; documentar fontes dos dados de demonstracao, por que estas features foram escolhidas, limitacoes e exemplos de predicao. Para maior qualidade, coletar amostras reais e avaliar acerto.
- **Integracao hardware/software:** LDR -> ADC do ESP32 -> temporizador e IA -> OLED, Monitor Serial e buzzer.
- **Autonomia:** sistema monitorando e decidindo quando alertar sem precisar de comando; carro possui piloto automatico ja existente.
- **CitizenTech:** incentivo ao uso consciente de iluminacao residencial, com alerta acessivel.
- **Acabamento e robustez:** fixar sensor e fios, evitar curto, testar sem eco e interferencia dos botões.
- **Testes:** gravar os cenarios acima e resultados observados, inclusive falhas e ajustes.
- **Criatividade:** contextualizar para uma residencia e mostrar como o projeto poderia evoluir para sensores de consumo real.

Nao e possivel garantir uma nota maxima apenas com codigo. A demonstracao real, a justificativa da solucao e a documentacao contam.

## 9. Entrega e video

- Envie apenas o **link do repositorio GitHub**, conforme os slides da atividade.
- No README do GitHub, inclua: contexto CitizenTech, arquitetura, componentes, ligacoes, passo a passo de instalacao, bibliotecas, calibracao, como testar, detalhes do modelo k-NN, seus limites e fotos/video.
- Coloque todos os **integrantes do grupo** no README; sem isso, os slides dizem que a nota nao sera atribuida aos ausentes.
- Video de **ate 6 minutos** (a descricao da atividade cita video demonstrativo e um teto de 6 minutos): apresente problema, ligacoes, leitura LDR, mudanca no OLED, aviso do buzzer, papel da IA e funcionamento geral.
- Vincule o video no proprio README ou em um link publico ao YouTube, conforme os slides.

## 10. Seguranca

Nao ligue o prototipo em tomadas nem corte cabos de energia residencial. Nao trate um sensor ultrassonico como detector confiavel de pessoas. Nao ligue a placa pela USB enquanto a fiação estiver sendo alterada. Antes do primeiro teste, confira com a serigrafia da placa cada 3V3/GND/AO/GPIO34. Durante testes com motor, levante as rodas do chao.
