#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "hardware/gpio.h"

#define NUM_PIXELS 25
#define WS2812_PIN 7
#define BUTTON_A 5
#define BUTTON_B 6
#define LED_R 13
#define LED_G 12
#define LED_B 11

#define IS_RGBW false
#define STACK_SIZE 10

PIO pio = pio0;
uint sm = 0;

int number_stack[STACK_SIZE];
int stack_top = -1;

uint64_t last_button_A_time = 0;  // Variável para armazenar o tempo do último pressionamento do botão A
uint64_t last_button_B_time = 0;  // Variável para armazenar o tempo do último pressionamento do botão B

bool led_red_on = false;
int current_number = 0;

void push_number(int number) {
    if (stack_top < STACK_SIZE - 1) {
        number_stack[++stack_top] = number;
    }
}

int pop_number() {
    if (stack_top >= 0) {
        return number_stack[stack_top--];
    }
    return 0;
}

void set_pixel(uint index, uint32_t color) {
    pio_sm_put_blocking(pio, sm, color << 8u);
}

void invert_matrix_180(uint8_t *number) {
    uint8_t inverted[NUM_PIXELS];
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            inverted[(4 - row) * 5 + col] = number[row * 5 + col];
        }
    }
    for (int i = 0; i < NUM_PIXELS; i++) {
        number[i] = inverted[i];
    }
}

const uint8_t numbers[10][NUM_PIXELS] = {
    // Aqui ficam as representações dos números de 0 a 9
    // (Cada número é uma matriz 5x5 representando os LEDs acesos ou apagados)
   // Número 0
    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 1
    {0, 0, 1, 0, 0,
     0, 1, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 1, 1, 1, 0},
    // Número 2
    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0},
    // Número 3
    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 4
    {0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 0, 0, 0},
    // Número 5
    {0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 6
    {0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 7
    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0},
    // Número 8
    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 9
    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 0, 0, 0}
};

void display_number(int number) {
    uint8_t temp[NUM_PIXELS];
    for (int i = 0; i < NUM_PIXELS; i++) {
        temp[i] = numbers[number][i];
    }
    invert_matrix_180(temp);

    float brightness_factor = 0.3;

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            int index = row * 5 + col;
            if (temp[index] == 1) {
                uint32_t blue_color = 0x0000FF;
                uint8_t r = (blue_color >> 16) & 0xFF;
                uint8_t g = (blue_color >> 8) & 0xFF;
                uint8_t b = blue_color & 0xFF;

                r = (uint8_t)(r * brightness_factor);
                g = (uint8_t)(g * brightness_factor);
                b = (uint8_t)(b * brightness_factor);

                uint32_t dimmed_color = (r << 16) | (g << 8) | b;
                set_pixel(index, dimmed_color);
            } else {
                set_pixel(index, 0x000000);
            }
        }
    }
}

void set_rgb_led(bool r, bool g, bool b) {
    gpio_put(LED_R, r);
    gpio_put(LED_G, g);
    gpio_put(LED_B, b);
}

// Debouncing e Interrupção - Função chamada quando um botão é pressionado
void button_callback(uint gpio, uint32_t events) {
    uint64_t current_time = to_ms_since_boot(get_absolute_time());

    // Início do Debouncing
    if (gpio == BUTTON_A && (current_time - last_button_A_time) > 300) {  
        // Verifica se foi o botão A e aplica debouncing (evita leituras falsas)
        last_button_A_time = current_time;  // Atualiza o tempo do último pressionamento
        printf("Botão A pressionado\n");
        push_number(current_number);  // Salva o número atual no "stack"
        current_number = (current_number + 1) % 10;  // Incrementa o número mostrado
        display_number(current_number);  // Exibe o novo número
    } 
    else if (gpio == BUTTON_B && (current_time - last_button_B_time) > 300) {  
        // Verifica se foi o botão B e aplica debouncing
        last_button_B_time = current_time;  // Atualiza o tempo do último pressionamento
        printf("Botão B pressionado\n");
        if (stack_top >= 0) {  
            current_number = pop_number();  // Recupera o último número salvo
            display_number(current_number);  // Exibe o número recuperado
        }
    }
    // Fim do Debouncing
}

int main() {
    stdio_init_all();

    gpio_init(LED_R);
    gpio_init(LED_G);
    gpio_init(LED_B);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_set_dir(LED_B, GPIO_OUT);

    gpio_init(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_A);  // Ativa pull-up interno para evitar leituras flutuantes
    gpio_pull_up(BUTTON_B);

    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    display_number(current_number);

    // Início das Interrupções para os botões
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_callback);  
    // Habilita interrupção na borda de descida para o botão A

    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &button_callback);  
    // Habilita interrupção na borda de descida para o botão B
    // Fim das Interrupções

    while (1) {
        uint64_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time % 200 < 100) {  
            gpio_put(LED_R, true);  // Liga LED vermelho a cada 100ms
        } else {
            gpio_put(LED_R, false);
        }
    }
    return 0;
}
