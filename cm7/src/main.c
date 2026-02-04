#include "FreeRTOS.h"
#include "common.h"
#include "stm32h755.h"
#include "task.h"

/*-----------------------------------------------------------*/

static void vBlinkTask(void *argument)
{
    (void)argument;

    for (;;) {
        /* Placeholder task */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/*-----------------------------------------------------------*/

int main(void)
{
    /* SystemInit() is called from startup.s */
    delay(1000);
    RCC_GCR |= RCC_GCR_BOOT_C4;

    xTaskCreate(vBlinkTask, "blink", 256, /* stack words, not bytes */
                NULL, tskIDLE_PRIORITY + 1, NULL);

    vTaskStartScheduler();

    /* Should never reach here */
    for (;;)
        ;
}
