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

    int counter = 0;
    bool anim = 0;

    char buf[] = {'T', 'E', 'S', 'T'};
    int size = 4;

    for(;;){
        if(api->xSemaphoreTake()){
            api->lcd_clear();
            api->setCursor(counter,1);
            api->lcd_print(buf);
            api->xSemaphoreGive();
        }

        if(counter == (16-size)){
            anim = true;
        }
        if(!anim){
            counter++;
        }
        else{
            counter--;
        }
        if(counter == 0){
            anim = false;
        }
        
        api->vTaskDelay(1000);
    }
    
}