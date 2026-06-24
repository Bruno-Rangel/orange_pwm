#include <zephyr/kernel.h>             // Funções básicas do Zephyr (ex: k_msleep, k_thread, etc.)
#include <zephyr/device.h>             // API para obter e utilizar dispositivos do sistema
#include <zephyr/drivers/gpio.h>       // API para controle de pinos de entrada/saída (GPIO)
#include <zephyr/console/console.h>
#include <pwm_z42.h>                // Biblioteca personalizada com funções de controle do TPM (Timer/PWM Module)
#include <stdio.h>

// Define o valor do registrador MOD do TPM para configurar o período do PWM
#define TPM_MODULE 1000         // Define a frequência do PWM fpwm = (TPM_CLK / (TPM_MODULE * PS))
// Valores de duty cycle correspondentes a diferentes larguras de pulso
uint16_t duty_50  = TPM_MODULE/2;       // 50% de duty cycle (meio brilho)

int main(void)
{
    // Inicializa o módulo TPM com:
    // - base do TPMx
    // - fonte de clock PLL/FLL (TPM_CLK)
    // - valor do registrador MOD
    // - tipo de clock (TPM_CLK)
    // - prescaler de 1 a 128 (PS)
    // - modo de operação EDGE_PWM
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

    pwm_tpm_Init(TPM0, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

    // Inicializa o canal 0 do TPM2 para gerar sinal PWM na porta GPIOB_18
    // - modo TPM_PWM_H (nível alto durante o pulso)
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18);

    // Inicializa o canal 1 do TPM2 para gerar sinal PWM na porta GPIOB_19
    // - modo TPM_PWM_H (nível alto durante o pulso)
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 19);

    // Inicializa o canal 1 do TPM0 para gerar sinal PWM na porta GPIOD_1
    // - modo TPM_PWM_H (nível alto durante o pulso)
    pwm_tpm_Ch_Init(TPM0, 1, TPM_PWM_H, GPIOD, 1);
 
    pwm_tpm_CnV(TPM2, 0, 0);
    pwm_tpm_CnV(TPM2, 1, TPM_MODULE*0.6);
    pwm_tpm_CnV(TPM0, 1, TPM_MODULE);

    printk("\nDigite o brilho desejado (0 a 100%%):\n");

    int porcentagem = 0;

    // Loop infinito
    for (;;)
    {
        // O programa poderia alterar o duty cycle dinamicamente aqui se desejado

        // Captura a linha do terminal (O Zephyr inicializa isso internamente no primeiro chamado)
        if (scanf("%d", &porcentagem) == 1) {
            if (porcentagem < 0 || porcentagem > 100) {
                printk("\n[ERRO] Valor invalido. Apenas numeros entre 0 e 100.\n");
                continue;
            }
            printk("\npassei aqui");
            // k representa a parcela de "tempo apagado" do LED Vermelho (Lógica Invertida)
            uint16_t k_vermelho = TPM_MODULE - ((porcentagem * TPM_MODULE) / 100);
            
            // Aplica o novo Duty Cycle mantendo a proporção do Laranja:
            // O Vermelho varia de 0 a 100% da intensidade escolhida
            pwm_tpm_CnV(TPM2, 0, k_vermelho);
            
            // O Verde precisa manter a proporção de ser mais fraco que o vermelho para não virar amarelo.
            // O cálculo abaixo escala linearmente a intensidade do Verde baseada na porcentagem do usuário.
            uint16_t k_verde = TPM_MODULE - (((porcentagem * 40) * TPM_MODULE) / 10000);
            pwm_tpm_CnV(TPM2, 1, k_verde);
            
            // O azul permanece totalmente apagado
            pwm_tpm_CnV(TPM0, 1, TPM_MODULE);
        } else {
            getchar();
        }
        k_msleep(200);
    }
    return 0;
}