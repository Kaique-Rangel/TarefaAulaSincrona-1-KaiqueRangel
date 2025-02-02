# EmbarcaTech   
### TAREFA aula síncrona 27/01/2025 
#### Grupo 4 
##### Autor:

* Kaique Rangel Da Silva

#### Vídeo de funcionamento
* 

#### Instruções de compilação
Certifique-se de que você tem o ambiente configurado conforme abaixo:
* Pico SDK instalado e configurado.
* VSCode com todas as extensões configuradas, CMake e Make instalados.
* Clone o repositório e abra a pasta do projeto, a extensão Pi Pico criará a pasta build
* Clique em Compile na barra inferior, do lado direito (ao lado esquerdo de RUN | PICO SDK)
* Abra o diagram.json
* Clique em Play

#### Descrição:
Este projeto implementa a exibição de números de 0 a 9 em uma matriz de LEDs WS2812 (5x5) usando um Raspberry Pi Pico. A mudança de números é controlada por dois botões:

Botão A (GPIO 5): Incrementa o número exibido na matriz.

Botão B (GPIO 6): Retorna ao número anterior.

A matriz de LEDs está conectada ao GPIO 7, e um LED RGB é usado para sinalizações visuais nos GPIOs 11, 12 e 13.
#### Funcionalidades Principais:
✅ Exibição de números na matriz WS2812
✅ Uso de interrupções (IRQ) para capturar eventos dos botões
✅ Implementação de debouncing para evitar leituras erradas
✅ Pilha para armazenar números anteriores e possibilitar a navegação reversa
✅ LED vermelho piscando continuamente

### Interrupções (IRQ)
* As interrupções são utilizadas para evitar a necessidade de um loop contínuo que verifica o estado dos botões.
* GPIO_IRQ_EDGE_FALL: A interrupção ocorre quando o botão é pressionado (sinal vai de HIGH para LOW).

* &button_callback: Define a função que será chamada quando a interrupção ocorrer.

#### Debouncing
* Foi usado uma verificação de tempo (delay via software) para ignorar acionamentos repetidos.
* Explicação:
current_time = to_ms_since_boot(get_absolute_time()); pega o tempo atual.

last_button_A_time armazena o tempo da última pressão do botão.

Se o botão for pressionado antes de 300ms, o evento é ignorado.

Isso impede leituras falsas, garantindo que apenas um acionamento real seja registrado.



