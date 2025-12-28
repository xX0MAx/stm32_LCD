#include <stdbool.h>
#include <stdint.h>

typedef struct {
    void (*vTaskDelay)(int);

    bool (*xSemaphoreTake)(void);
    void (*xSemaphoreGive)(void);

    void (*setCursor)(int, int);
    void (*lcd_print)(char*);
    void (*lcd_print_int)(int);
    void (*lcd_print_time)(void);
    void (*lcd_clear)(void);

    void (*set_hms)(int, int, int);
    void (*set_rtc_hms)(void);
} RAM_API;

void _start(uint32_t API_ADDRESS) {
    RAM_API* api = (RAM_API*)(API_ADDRESS);

    api->set_hms(12,0,0);
    api->set_rtc_hms();

    for(;;){
        if(api->xSemaphoreTake()){
            api->setCursor(0,0);
            api->lcd_print_time();
            api->xSemaphoreGive();
        }

        api->vTaskDelay(1000);
    }
    
}
