/*
 * Por: Wilton Lacerda Silva
 *    Ohmímetro utilizando o ADC da BitDogLab
 *
 * 
 * Neste exemplo, utilizamos o ADC do RP2040 para medir a resistência de um resistor
 * desconhecido, utilizando um divisor de tensão com dois resistores.
 * O resistor conhecido é de 10k ohm e o desconhecido é o que queremos medir.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_PIN 28 // GPIO para o voltímetro
#define Botao_A 5  // GPIO para botão A

int R_conhecido = 10000;   // Resistor de 10k ohm
float R_x = 0.0;           // Resistor desconhecido
float ADC_VREF = 3.31;     // Tensão de referência do ADC
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
  reset_usb_boot(0, 0);
}
void Codigo_cores(float Res, ssd1306_t c);//protótipo de função que imprime código de cores no display 
int main()
{
  // Para ser utilizado o modo BOOTSEL com botão B
  gpio_init(botaoB);
  gpio_set_dir(botaoB, GPIO_IN);
  gpio_pull_up(botaoB);
  gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  // Aqui termina o trecho para modo BOOTSEL com botão B

  gpio_init(Botao_A);
  gpio_set_dir(Botao_A, GPIO_IN);
  gpio_pull_up(Botao_A);

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA);                                        // Pull up the data line
  gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
  ssd1306_t ssd;                                                // Inicializa a estrutura do display
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd);                                         // Configura o display
  ssd1306_send_data(&ssd);                                      // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  adc_init();
  adc_gpio_init(ADC_PIN); // GPIO 28 como entrada analógica

  float tensao;
  char str_x[5]; // Buffer para armazenar a string
  char str_y[5]; // Buffer para armazenar a string

  bool cor = true;
  while (true)
  {
    adc_select_input(2); // Seleciona o ADC para eixo X. O pino 28 como entrada analógica

    float soma = 0.0f;
    for (int i = 0; i < 500; i++)
    {
      soma += adc_read();
      sleep_ms(1);
    }
    float media = soma / 500.0f;

      // Fórmula simplificada: R_x = R_conhecido * ADC_encontrado /(ADC_RESOLUTION - adc_encontrado)
      R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);

    sprintf(str_x, "%1.0f", media); // Converte o inteiro em string
    sprintf(str_y, "%1.0f", R_x);   // Converte o float em string

    // cor = !cor;
    //  Atualiza o conteúdo do display com animações
    ssd1306_fill(&ssd, !cor);                          // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
    ssd1306_draw_string(&ssd, "Ohmimetro", 25, 5); // Desenha uma string
    ssd1306_line(&ssd, 3, 13, 123, 13, cor);           // Desenha uma linha
    //ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);  // Desenha uma string
    //ssd1306_draw_string(&ssd, "  Ohmimetro", 10, 28);  // Desenha uma string
    ssd1306_draw_string(&ssd, "ADC", 13, 44);          // Desenha uma string
    ssd1306_draw_string(&ssd, "Resisten.", 50, 44);    // Desenha uma string
    ssd1306_draw_string(&ssd, "Fx 1.", 5, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&ssd, "Fx 2.", 5, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&ssd, "Mul .", 5, 34);    // Desenha uma string (multiplicador)
    ssd1306_line(&ssd, 3, 42, 123, 42, cor);           // Desenha uma linha
    ssd1306_line(&ssd, 3, 24, 123, 24, cor);           // Desenha uma linha
    ssd1306_line(&ssd, 3, 33, 123, 33, cor);           // Desenha uma linha
    ssd1306_line(&ssd, 44, 13, 44, 60, cor);           // Desenha uma linha vertical
    ssd1306_draw_string(&ssd, str_x, 8, 52);           // Desenha uma string
    ssd1306_draw_string(&ssd, str_y, 59, 52);          // Desenha uma string
    Codigo_cores(R_x, ssd);//Calcula as cores correspondentes da resistência Considerando a série E24
    ssd1306_send_data(&ssd);                           // Atualiza o display
    sleep_ms(700);
  }
}
void Codigo_cores(float Res, ssd1306_t c){//Função para saber qual resistor corrspondente e24
  float erro;//considerar erro de 5% para Série E24
  erro=Res*0.05;
  if((Res>(510-erro)) && (Res<(510+erro))){
    ssd1306_draw_string(&c, "Verde", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Marron", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Marron", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(560-erro)) && (Res<(560+erro))){
    ssd1306_draw_string(&c, "Verde", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Azul", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Marron", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(620-erro)) && (Res<(620+erro))){
    ssd1306_draw_string(&c, "Azul", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Vermelho", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Marron", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(680-erro)) && (Res<(680+erro))){
    ssd1306_draw_string(&c, "Azul", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Cinza", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Marron", 47, 34);    // Desenha uma string (multiplicador)  
  }else if((Res>(750-erro)) && (Res<(750+erro))){
    ssd1306_draw_string(&c, "Violeta", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Verde", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Marron", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(820-erro)) && (Res<(820+erro))){
    ssd1306_draw_string(&c, "Cinza", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Vermelho", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Marron", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(910-erro)) && (Res<(910+erro))){
    ssd1306_draw_string(&c, "Branco", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Marron", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Marron", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(1000-erro)) && (Res<(1000+erro))){
    ssd1306_draw_string(&c, "Marron", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Preto", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(1100-erro)) && (Res<(1100+erro))){
    ssd1306_draw_string(&c, "Marron", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Marron", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(1200-erro)) && (Res<(1200+erro))){
    ssd1306_draw_string(&c, "Marron", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Vermelho", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(1300-erro)) && (Res<(1300+erro))){
    ssd1306_draw_string(&c, "Marron", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Laranja", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(1500-erro)) && (Res<(1500+erro))){
    ssd1306_draw_string(&c, "Marron", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Verde", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(1600-erro)) && (Res<(1600+erro))){
    ssd1306_draw_string(&c, "Marron", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Azul", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(1800-erro)) && (Res<(1800+erro))){
    ssd1306_draw_string(&c, "Marron", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Cinza", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(2000-erro)) && (Res<(2000+erro))){
    ssd1306_draw_string(&c, "Vermelho", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Preto", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(2200-erro)) && (Res<(2200+erro))){
    ssd1306_draw_string(&c, "Vermelho", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Vermelho", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(2400-erro)) && (Res<(2400+erro))){
    ssd1306_draw_string(&c, "Vermelho", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Amarelo", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(2700-erro)) && (Res<(2700+erro))){
    ssd1306_draw_string(&c, "Vermelho", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Violeta", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(3000-erro)) && (Res<(3000+erro))){
    ssd1306_draw_string(&c, "Laranja", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Preto", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(3300-erro)) && (Res<(3300+erro))){
    ssd1306_draw_string(&c, "laranja", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Laranja", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(3600-erro)) && (Res<(3600+erro))){
    ssd1306_draw_string(&c, "Laranja", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Azul", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(3900-erro)) && (Res<(3900+erro))){
    ssd1306_draw_string(&c, "Laranja", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Branco", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(4300-erro)) && (Res<(4300+erro))){
    ssd1306_draw_string(&c, "Amarelo", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Laranja", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(4700-erro)) && (Res<(4700+erro))){
    ssd1306_draw_string(&c, "Amarelo", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Violeta", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(5100-erro)) && (Res<(5100+erro))){
    ssd1306_draw_string(&c, "Verde", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Marron", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(5600-erro)) && (Res<(5600+erro))){
    ssd1306_draw_string(&c, "Verde", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Azul", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(6200-erro)) && (Res<(6200+erro))){
    ssd1306_draw_string(&c, "Azul", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Vemelho", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(6800-erro)) && (Res<(6800+erro))){
    ssd1306_draw_string(&c, "Azul", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Cinza", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(7500-erro)) && (Res<(7500+erro))){
    ssd1306_draw_string(&c, "Violeta", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Verde", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(8200-erro)) && (Res<(8200+erro))){
    ssd1306_draw_string(&c, "Cinza", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Vemelho", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(9100-erro)) && (Res<(9100+erro))){
    ssd1306_draw_string(&c, "Branco", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Marron", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Vermelho", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(10000-erro)) && (Res<(10000+erro))){
    ssd1306_draw_string(&c, "Marron", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Preto", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(11000-erro)) && (Res<(11000+erro))){
    ssd1306_draw_string(&c, "Marron", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Marron", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(12000-erro)) && (Res<(12000+erro))){
    ssd1306_draw_string(&c, "Marron", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Vermelho", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(13000-erro)) && (Res<(13000+erro))){
    ssd1306_draw_string(&c, "Marron", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Laranja", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(15000-erro)) && (Res<(15000+erro))){
    ssd1306_draw_string(&c, "Marron", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Verde", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(16000-erro)) && (Res<(16000+erro))){
    ssd1306_draw_string(&c, "Marron", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Azul", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(18000-erro)) && (Res<(18000+erro))){
    ssd1306_draw_string(&c, "Marron", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Cinza", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(20000-erro)) && (Res<(20000+erro))){
    ssd1306_draw_string(&c, "Vermelho", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Preto", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(22000-erro)) && (Res<(22000+erro))){
    ssd1306_draw_string(&c, "Vermelho", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Vermelho", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(24000-erro)) && (Res<(24000+erro))){
    ssd1306_draw_string(&c, "Vermelho", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Amarelo", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(27000-erro)) && (Res<(27000+erro))){
    ssd1306_draw_string(&c, "Vermelho", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Violeta", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(30000-erro)) && (Res<(30000+erro))){
    ssd1306_draw_string(&c, "Laranja", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Preto", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(33000-erro)) && (Res<(33000+erro))){
    ssd1306_draw_string(&c, "Laranja", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Laranja", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(36000-erro)) && (Res<(36000+erro))){
    ssd1306_draw_string(&c, "Laranja", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Azul", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(39000-erro)) && (Res<(39000+erro))){
    ssd1306_draw_string(&c, "Laranja", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Branco", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(43000-erro)) && (Res<(43000+erro))){
    ssd1306_draw_string(&c, "Amarelo", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Laranja", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(47000-erro)) && (Res<(47000+erro))){
    ssd1306_draw_string(&c, "Amarelo", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Violeta", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(51000-erro)) && (Res<(51000+erro))){
    ssd1306_draw_string(&c, "Verde", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Marron", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(56000-erro)) && (Res<(56000+erro))){
    ssd1306_draw_string(&c, "Verde", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Azul", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(62000-erro)) && (Res<(62000+erro))){
    ssd1306_draw_string(&c, "Azul", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Vermelho", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(68000-erro)) && (Res<(68000+erro))){
    ssd1306_draw_string(&c, "Azul", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Cinza", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(75000-erro)) && (Res<(75000+erro))){
    ssd1306_draw_string(&c, "Violeta", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Verde", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(82000-erro)) && (Res<(82000+erro))){
    ssd1306_draw_string(&c, "Cinza", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Vermelho", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(91000-erro)) && (Res<(91000+erro))){
    ssd1306_draw_string(&c, "Branco", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Marron", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Laranja", 47, 34);    // Desenha uma string (multiplicador)
  }else if((Res>(100000-erro)) && (Res<(100000+erro))){
    ssd1306_draw_string(&c, "Marron", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Preto", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Amarelo", 47, 34);    // Desenha uma string (multiplicador)
  }else{
    ssd1306_draw_string(&c, "Fora E24", 47, 16);    // Desenha uma string (faixa 1)
    ssd1306_draw_string(&c, "Fora E24", 47, 25);    // Desenha uma string (faixa 2)
    ssd1306_draw_string(&c, "Fora E24", 47, 34);    // Desenha uma string (multiplicador)
  }
}