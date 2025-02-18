# Tarefa U4C8: Controle de LEDs RGB e Display OLED com Conversores A/D no RP2040
Nesse projeto utilizamos o ADC (Conversor Analógico Digital) e o PWM (Pulse Width Modulation), presentes no RP2040, para controlar LEDs RGB e Display OLED por um Joystick e dois botões, todos presentes na placa BitDogLab utilizada. Tarefa desenvolvida para a Capacitação EmbarcaTech por Maria Eduarda Campos.

### Video de demonstração:
https://youtu.be/nFNyoBln278?si=fMPep_nyfHX7AhOw

### Funcionalidades:

1. O joystick fornece valores analógicos dos eixos X e Y que controlam a intensidade dos LEDs RGB via PWM.
  - LED Azul: Brilho ajustado conforme o eixo Y (Valor central = apagado, 0 e 4095 = aceso em uma luminosidade máxima).
  - LED Vermelho: Brilho ajustado conforme o eixo X (Valor central = apagado, 0 e 4095 = aceso em uma luminosidade máxima).

2. Exibe um quadrado de 8x8 pixels no display SSD1306, movendo-o conforme os valores do joystick.

3. Botão do joystick: alterna o estado do LED Verde e modifica a borda do display (alternando estilos) a cada acionamento.

4. Botão A: ativa ou desativa os LEDs controlados por PWM a cada acionamento.

### Instruções:
1. Clone este repositório:
   ```sh
   git clone https://github.com/Mescxll/Tarefa_ADC.git
   ```
2. Compile e carregue o código no Raspberry Pi Pico W utilizando o VS Code e o Pico SDK.
3. Utilize a placa BitDogLab e os seus componentes para testar o funcionamento do display e dos LEDs.
