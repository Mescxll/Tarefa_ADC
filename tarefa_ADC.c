//Tarefa U4C8 por Maria Eduarda Campos

//Bibliotecas
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

#define LED_V 13 //LED vermelho
#define LED_A 12 //LED azul
#define LED_VD 11 //LED verde
#define JOYSTICK_X 27 //Joystick eixo X
#define JOYSTICK_Y 26 //Joystick eixo Y
#define BOTAO_JOY 22 //Botão do Joystick
#define BOTAO_A 5 //Botão A

#define I2C_PORT i2c1 //Porta do I2C
#define I2C_SDA 14 //Display SDA
#define I2C_SCL 15 //Display SCL

#define endereco 0x3C 

//Váriaveis PWM
const float DIVIDER_PWM = 256.f; 
const uint16_t PERIOD = 4096;

static volatile uint32_t ultimo_tempo = 0; //Armazena o último tempo absoluto 
static volatile uint8_t estilo_borda = 0; //Indica qual o estilo de borda atual
static volatile bool estado_leds = 1; //Indica o estado dos LEDs

//Protótipo da função callback
static void gpio_irq_handler(uint gpio, uint32_t events);

//Inicializa e configura o PWP
void inicia_pwm(uint pin, uint slice_num) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, DIVIDER_PWM); 
    pwm_init(slice_num, &config, true);
    pwm_set_wrap(slice_num, PERIOD);
}

//Lê a posição do Joystick
uint16_t leitura_joystick(uint adc){
    adc_select_input(adc);
    return adc_read();
}

//Muda a borda conforme a váriavel booleana
void alternar_estilo_borda(ssd1306_t *display) {
    if (estilo_borda == 0) {
        ssd1306_rect(display, 0, 0, 127, 63, true, false);
    } else {
        ssd1306_rect(display, 2, 2, 123, 59, true, false);
        ssd1306_rect(display, 5, 5, 123, 59, true, false);
    }
}

//Função principal
int main(){
    stdio_init_all();

    //Inicializa o Joystick
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);
    adc_select_input(0);

    //Inicializa o LED vermelho e define como saída
    gpio_init(LED_V);
    gpio_set_dir(LED_V, GPIO_OUT);

    //Inicializa o LED azul e define como saída
    gpio_init(LED_A);
    gpio_set_dir(LED_A, GPIO_OUT);

    //Inicializa o LED verde e define como saída
    gpio_init(LED_VD);
    gpio_set_dir(LED_VD, GPIO_OUT);

    //Inicializa o Botão A, define como entrada e põe em nível alto enquanto não pressionado
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    
    //Inicializa o Botão do Joystick, define como entrada e põe em nível alto enquanto não pressionado
    gpio_init(BOTAO_JOY);
    gpio_set_dir(BOTAO_JOY, GPIO_IN);
    gpio_pull_up(BOTAO_JOY);
    
    //Chama a função de callback
    gpio_set_irq_enabled_with_callback(BOTAO_JOY, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    //Inicializa o I2C na porta i2c1 em 400Hz
    i2c_init(I2C_PORT, 400 * 1000);

    //Configura o I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_t display; //Declara o Dysplay
    ssd1306_init(&display, 128, 64, false, endereco, I2C_PORT); //Inicializa
    ssd1306_config(&display); //Configura
    ssd1306_fill(&display, false); //Limpa o display
    ssd1306_send_data(&display); //Envia os dados do display

    //Declara os slices dos LEDs azul e vermelho
    uint slice_num_v = pwm_gpio_to_slice_num(LED_V);
    uint slice_num_a = pwm_gpio_to_slice_num(LED_A);

    //Inicializa os slices nos LEDs correspondentes
    inicia_pwm(LED_V, slice_num_v);
    inicia_pwm(LED_A, slice_num_a);

    //Váriavel para indicar o ativamento de um pixel
    bool cor = true;

    //Loop Infinito
    while (true) {
        //Lê os valores dos eixos X e Y
        uint16_t x_valor = leitura_joystick(0);
        uint16_t y_valor = leitura_joystick(1);

        //Liga os LEDs vemelho e azul com luminosidade conforme a movimentação do Joystick
        if (estado_leds) {
            int16_t x_iluminacao = abs(x_valor - 1893) * 16 / 1893;
            int16_t y_iluminacao = abs(y_valor - 2099) * 16 / 2099;

            pwm_set_gpio_level(LED_V, x_iluminacao);
            pwm_set_gpio_level(LED_A, y_iluminacao);
        }

        ssd1306_fill(&display, false); //Limpa o Display
        
        //Configura a posição do quadrado
        uint8_t x_pos = x_valor * 57 / 4096;
        uint8_t y_pos = y_valor * 121 / 4096;

        //Posiciona um quadrado 8x8 nas posições configuradas conforme a movimentação do Joystick 
        ssd1306_rect(&display, x_pos, y_pos, 8, 8, cor, cor);      
        alternar_estilo_borda(&display); //Chama a função que altera a borda
        ssd1306_send_data(&display); //Envia os dados

        sleep_ms(100); //Delay
    }
    return 0;
}

//Função de Callback
void gpio_irq_handler(uint botao, uint32_t eventos){
    //Armazena o tempo absoluto do sistema
    uint32_t tempo_real = to_us_since_boot(get_absolute_time());

    //Debouncing
    if (tempo_real - ultimo_tempo > 200000) { //Verifica se passou 200ms desde o último evento
        if (botao == BOTAO_A) { //Caso botão A
            estado_leds = !estado_leds; //Muda a váriavel de estado dos LEDs
            if (!estado_leds) { //Desliga os LEDs
                pwm_set_gpio_level(LED_V, 0);
                pwm_set_gpio_level(LED_A, 0);
                pwm_set_gpio_level(LED_VD, 0);
            }
        } else if (botao == BOTAO_JOY) { //Caso botão do joystick
            gpio_put(LED_VD, !gpio_get(LED_VD)); //Muda o estado do LED verde
            estilo_borda = !estilo_borda; //Muda a borda
        }
        ultimo_tempo = tempo_real; //Atualiza o tempo
    }
}
