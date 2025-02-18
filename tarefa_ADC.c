//Tarefa U4C8 por Maria Eduarda Campos

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

#define LED_V 13
#define LED_A 12
#define LED_VD 11
#define JOYSTICK_X 27
#define JOYSTICK_Y 26
#define BOTAO_JOY 22
#define BOTAO_A 5

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

#define endereco 0x3C

const float DIVIDER_PWM = 256.f; 
const uint16_t PERIOD = 4096;

static volatile uint32_t ultimo_tempo = 0;
static volatile uint8_t estilo_borda = 0;
static volatile bool estado_leds = 1;

static void gpio_irq_handler(uint gpio, uint32_t events);

void inicia_pwm(uint pin, uint slice_num) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, DIVIDER_PWM); 
    pwm_init(slice_num, &config, true);
    pwm_set_wrap(slice_num, PERIOD);
}

uint16_t leitura_joystick(uint adc){
    adc_select_input(adc);
    return adc_read();
}

void alternar_estilo_borda(ssd1306_t *display) {
    if (estilo_borda == 0) {
        ssd1306_rect(display, 0, 0, 127, 63, true, false);
    } else {
        ssd1306_rect(display, 2, 2, 123, 59, true, false);
        ssd1306_rect(display, 5, 5, 123, 59, true, false);
    }
}

int main(){
    stdio_init_all();
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);
    adc_select_input(0);

    gpio_init(LED_V);
    gpio_set_dir(LED_V, GPIO_OUT);

    gpio_init(LED_A);
    gpio_set_dir(LED_A, GPIO_OUT);

    gpio_init(LED_VD);
    gpio_set_dir(LED_VD, GPIO_OUT);

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    
    gpio_init(BOTAO_JOY);
    gpio_set_dir(BOTAO_JOY, GPIO_IN);
    gpio_pull_up(BOTAO_JOY);
    
    gpio_set_irq_enabled_with_callback(BOTAO_JOY, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_t display;
    ssd1306_init(&display, 128, 64, false, endereco, I2C_PORT);
    ssd1306_config(&display); 
    ssd1306_fill(&display, false);
    ssd1306_send_data(&display); 

    uint slice_num_v = pwm_gpio_to_slice_num(LED_V);
    uint slice_num_a = pwm_gpio_to_slice_num(LED_A);

    inicia_pwm(LED_V, slice_num_v);
    inicia_pwm(LED_A, slice_num_a);

    bool cor = true;
    while (true) {
        uint16_t x_valor = leitura_joystick(0);
        uint16_t y_valor = leitura_joystick(1);

        if (estado_leds) {
            int16_t x_iluminacao = abs(x_valor - 1893) * 16 / 1893;
            int16_t y_iluminacao = abs(y_valor - 2099) * 16 / 2099;

            pwm_set_gpio_level(LED_V, x_iluminacao);
            pwm_set_gpio_level(LED_A, y_iluminacao);
        }

        ssd1306_fill(&display, false);
        uint8_t x_pos = x_valor * 57 / 4096;
        uint8_t y_pos = y_valor * 121 / 4096;

        ssd1306_rect(&display, x_pos, y_pos, 8, 8, cor, cor);
        alternar_estilo_borda(&display);
        ssd1306_send_data(&display);

        sleep_ms(100);
    }
    return 0;
}

void gpio_irq_handler(uint botao, uint32_t eventos){
    uint32_t tempo_real = to_us_since_boot(get_absolute_time());

    if (tempo_real - ultimo_tempo > 200000) {
        if (botao == BOTAO_A) {
            estado_leds = !estado_leds;
            if (!estado_leds) {
                pwm_set_gpio_level(LED_V, 0);
                pwm_set_gpio_level(LED_A, 0);
                pwm_set_gpio_level(LED_VD, 0);
            }
        } else if (botao == BOTAO_JOY) {
            gpio_put(LED_VD, !gpio_get(LED_VD));
            estilo_borda = !estilo_borda;
        }
        ultimo_tempo = tempo_real;
    }
}
