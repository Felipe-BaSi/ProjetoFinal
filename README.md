# Projeto de Controle de Volume Automático

Este projeto implementa um sistema de controle de volume automático utilizando a placa **BitDogLab**. O sistema ajusta o volume do buzzer com base no som ambiente captado pelo microfone e permite ajustes manuais via joystick. O display OLED exibe o nível de volume, e a matriz de LEDs fornece feedback visual.

---

## Funcionalidades

- **Modo Automático**: O volume do buzzer é ajustado automaticamente com base no som ambiente captado pelo microfone.
- **Modo Manual**: O volume do buzzer pode ser ajustado manualmente usando o joystick.
- **Feedback Visual**:
  - O display OLED exibe o nível de volume atual.
  - A matriz de LEDs muda de cor e brilho conforme o nível de volume.
- **Alternância de Modos**: Um botão permite alternar entre os modos automático e manual.

---

## Hardware Utilizado

- **Placa BitDogLab** (com Raspberry Pi Pico W).
- **Microfone**: Capta o som ambiente.
- **Joystick**: Permite ajustes manuais do volume.
- **Buzzer**: Emite sons com volume ajustável.
- **Display OLED**: Exibe informações sobre o nível de volume.
- **Matriz de LEDs**: Fornece feedback visual do nível de volume.
- **Botão**: Alterna entre os modos de operação.

---

## Configuração do Ambiente

1. **Instalação do SDK do Raspberry Pi Pico**:
   - Siga as instruções oficiais para configurar o [Pico SDK](https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html).

2. **Configuração do Projeto**:
   - Clone este repositório ou copie o código para o seu ambiente de desenvolvimento.
   - Certifique-se de que as bibliotecas necessárias (`pico/stdlib.h`, `hardware/adc.h`, `hardware/pwm.h`, etc.) estão instaladas.

3. **Compilação**:
   - Utilize o CMake para configurar e compilar o projeto:
     ```bash
     mkdir build
     cd build
     cmake ..
     make
     ```

4. **Upload para a Placa**:
   - Conecte a placa BitDogLab ao computador via USB.
   - Copie o arquivo `.uf2` gerado para a placa.

---

## Execução

Após carregar o código na placa, o sistema iniciará automaticamente. O display OLED mostrará o nível de volume atual, e a matriz de LEDs refletirá o nível de volume com cores e brilho variáveis.

- **Modo Automático**: O volume do buzzer será ajustado com base no som ambiente.
- **Modo Manual**: Use o joystick para ajustar o volume manualmente.
- **Alternância de Modos**: Pressione o botão para alternar entre os modos automático e manual.

---

## Estrutura do Código

- **`main.c`**: Contém a lógica principal do projeto, incluindo a leitura do microfone, o controle do buzzer, a exibição no display OLED e a atualização da matriz de LEDs.
- **Funções Principais**:
  - `automatico()`: Implementa o modo automático de controle de volume.
  - `manual()`: Implementa o modo manual de controle de volume.
  - `buzzer_play()`: Controla o volume do buzzer via PWM.
  - `led_matrix_display()`: Atualiza a matriz de LEDs com base no nível de volume.
  - `gpio_irq_handler()`: Gerencia a interrupção do botão para alternar entre modos.

---

## Testes e Validação

O sistema foi testado em ambientes controlados, com níveis de ruído moderados. O microfone da BitDogLab mostrou-se funcional, mas pode não ser ideal para ambientes com grandes ruídos. O joystick, o display OLED e a matriz de LEDs funcionaram conforme o esperado.

---

## Melhorias Futuras

- Substituir o microfone por um modelo mais sensível para ambientes com grandes ruídos.
- Adicionar filtros digitais para melhorar a precisão da leitura do microfone.
- Implementar integração com dispositivos IoT para monitoramento remoto.