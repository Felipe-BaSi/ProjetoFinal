#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "inc/font.h"
#include "inc/ssd1306.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pio_matrix.pio.h"

// Definição dos pinos utilizados
#define buzzer 10
#define mic 28
#define led_r 13
#define but_A 5
#define joystick_X 27
#define joystick_Y 26

// Configuração do I2C
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

// Configurações do buzzer
#define buzzer_max 70
#define buzzer_tempo_desl 1000000
#define buzzer_tempo_lig  200000

// Número de pixels na matriz de LEDs
#define NUM_PIXELS 25

// Pino da matriz de LEDs
#define matrix 7

// Arrays para armazenar os valores de cor (vermelho, verde, azul) para cada pixel
double r[NUM_PIXELS] = {0}, g[NUM_PIXELS] = {0}, b[NUM_PIXELS] = {0};

// Variáveis globais para o display OLED, percentual do microfone e buzzer
ssd1306_t ssd;
uint mic_percent, buzzer_percent = 5;
bool buzzer_on = false;
volatile bool A_pressed = false;

// Variáveis para controle de tempo e debounce do botão
volatile uint32_t last_time = 0, last_debounce_time = 0;

// Função para inicializar o PWM em um pino específico
uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);  
    return slice_num;
}

// Função para converter valores de cor (R, G, B) em um valor de 32 bits para a matriz de LEDs
uint32_t matrix_rgb(double b, double r, double g) {
    unsigned char R, G, B;
    R = r * 255;
    G = g * 255;
    B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

// Função para atualizar a matriz de LEDs com os valores de cor atuais
void update_led_matrix(double *r, double *g, double *b, PIO pio, uint sm) {
    uint32_t valor_led;
    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        valor_led = matrix_rgb(b[i], r[i], g[i]);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

// Função para tocar o buzzer com uma frequência e volume específicos
void buzzer_play(uint frequencia, uint volume_percent) {

    gpio_set_function(buzzer, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(buzzer);

    if(frequencia == 0 || volume_percent == 0){

        pwm_set_enabled(slice_num, false);

    }else{
        
        uint32_t freq_relogio = 125000000;
        uint16_t contador = freq_relogio / frequencia;
        pwm_set_wrap(slice_num, contador);
        
        uint16_t duty_cycle = (contador * volume_percent) / 100;
        pwm_set_chan_level(slice_num, PWM_CHAN_A, duty_cycle);
        pwm_set_enabled(slice_num, true);

    }
}

// Função para o modo automático, onde o volume do buzzer é controlado pelo microfone
void automatico(uint pwm_wrap){

    if (!buzzer_on){

        adc_select_input(2);
        int16_t mic_value = adc_read();
        
        int16_t supr_ruido = mic_value - 2000;

        mic_percent = (abs(supr_ruido) * 100) / 4095;
        int16_t pwm_value = ((mic_value - 2048) * pwm_wrap) / 4095;

        if(mic_value >= 2080){

            pwm_set_gpio_level(led_r, pwm_value);

        }else{

            pwm_set_gpio_level(led_r, 0);

        }

    }

    if(mic_percent > buzzer_percent){

        buzzer_percent = mic_percent + 15;

        if(buzzer_percent > buzzer_max){
            
            buzzer_percent = buzzer_max;

        }

    }
    else if(buzzer_percent >= mic_percent){

        buzzer_percent = mic_percent + 15;
    }

}

// Função para o modo manual, onde o volume do buzzer é controlado pelo joystick
void manual(){

    uint16_t adc_y;

    adc_select_input(0);
    adc_y = adc_read();

    if(adc_y > 2500 && buzzer_percent < 100){

        buzzer_percent++;

    }else if(adc_y < 1500 && buzzer_percent > 0){

        buzzer_percent--;
    }
    
}

// Função para atualizar a matriz de LEDs com base no volume do buzzer
void led_matrix_display(){

    int i;

    if(buzzer_percent <= 20 && buzzer_percent > 0){
        for(i = 0; i < 5; i++){

            r[i] = 0.0;
            g[i] = 0.1;
            b[i] = 0.0;

        }

    }else if(buzzer_percent > 20 && buzzer_percent <= 40){

        for(i = 0; i < 10; i++){

            r[i] = 0.0;
            g[i] = 0.1;
            b[i] = 0.0;
            
        }

    }else if(buzzer_percent > 40 && buzzer_percent <= 60){

        for(i = 0; i < 15; i++){

            r[i] = 0.1;
            g[i] = 0.1;
            b[i] = 0.0;
            
        }

    }else if(buzzer_percent > 60 && buzzer_percent <= 80){

        for(i = 0; i < 20; i++){

            r[i] = 0.1;
            g[i] = 0.1;
            b[i] = 0.0;
            
        }
    }else if(buzzer_percent > 80 && buzzer_percent <= 100){

        for(i = 0; i < 25; i++){

            r[i] = 0.1;
            g[i] = 0.0;
            b[i] = 0.0;
            
        }

    }

}

// Função de interrupção para o botão A
void gpio_irq_handler(uint gpio, uint32_t events){
    if(to_us_since_boot(get_absolute_time()) - last_debounce_time > 500000){

        A_pressed = !A_pressed;

        last_debounce_time = to_us_since_boot(get_absolute_time());

        printf("Interrupção ativada");

    }
}

// Função principal
int main(){

    char mic_text[20];
    char buzzer_text[20];
    PIO pio = pio0;

    stdio_init_all();

    // Inicialização dos pinos
    gpio_init(led_r);
    gpio_set_dir(led_r, GPIO_OUT);

    gpio_init(but_A);
    gpio_set_dir(but_A, GPIO_IN);
    gpio_pull_up(but_A);

    uint pwm_wrap = 4096;  
    pwm_init_gpio(led_r, pwm_wrap);

    // Inicialização do ADC e do microfone
    adc_init();
    adc_gpio_init(mic);

    // Inicialização do I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
        
    // Inicialização do display OLED
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicialização da matriz de LEDs
    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, matrix);

    // Configuração da interrupção do botão A
    gpio_set_irq_enabled_with_callback(but_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Loop principal
    while (true) {
        
        if(!A_pressed){

            // Modo automático: controle do buzzer pelo microfone
            automatico(pwm_wrap);

            snprintf(mic_text, sizeof(mic_text), "Mic: %d%%", mic_percent);
            snprintf(buzzer_text, sizeof(buzzer_text), "Buz: %d%%", buzzer_percent);
            
            ssd1306_fill(&ssd, false);
            ssd1306_draw_string(&ssd, mic_text, 10, 20);
            ssd1306_draw_string(&ssd, buzzer_text, 10, 40);
            ssd1306_rect(&ssd, 0, 0, 128, 64, true, false);
            ssd1306_send_data(&ssd);

        }else{

            // Modo manual: controle do buzzer pelo joystick
            manual();

            snprintf(buzzer_text, sizeof(buzzer_text), "Buz: %d%%", buzzer_percent);
            
            ssd1306_fill(&ssd, false);
            ssd1306_draw_string(&ssd, buzzer_text, 10, 30);
            ssd1306_rect(&ssd, 0, 0, 128, 64, true, false);
            ssd1306_rect(&ssd, 1, 1, 126, 62, true, false);
            ssd1306_send_data(&ssd);

        }

        uint32_t current_time = to_us_since_boot(get_absolute_time());

        // Controle do buzzer e da matriz de LEDs
        if(!buzzer_on && current_time - last_time > buzzer_tempo_desl){

            buzzer_play(1000, buzzer_percent);
            buzzer_on = true;
            last_time = current_time;

            led_matrix_display();
            update_led_matrix(r, g, b, pio, sm);

        }

        if(buzzer_on && current_time - last_time > buzzer_tempo_lig){

            buzzer_play(0, 0);
            buzzer_on = false;
            last_time = current_time;

            printf("Porcentagem do microfone: %d\n", mic_percent);
            printf("Porcentagem do buzzer: %d\n", buzzer_percent);

            for(int i = 0; i < NUM_PIXELS; i++){
                r[i] = 0.0;
                g[i] = 0.0;
                b[i] = 0.0;
            }
            update_led_matrix(r, g, b, pio, sm);

        }

    }
}