#include <SD.h>
#include <STM32FreeRTOS.h>
#include <LiquidCrystal_I2C.h>
#include <STM32RTC.h>

#define LED_PIN PC13
#define SD_PIN PA4

#define CODE_BUFFER_SIZE 1048

uint8_t codeTask1[CODE_BUFFER_SIZE] __attribute__((aligned(4)));
uint8_t codeTask2[CODE_BUFFER_SIZE] __attribute__((aligned(4)));

uint32_t fileSize = 0;

SemaphoreHandle_t xMutexLCD;

LiquidCrystal_I2C lcd(0x27, 16, 2);
STM32RTC& rtc = STM32RTC::getInstance();

static byte seconds;
static byte minutes;
static byte hours;

struct RAM_API {
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
};

RAM_API api __attribute__((aligned(4))) = {
  .vTaskDelay = [](int time) { vTaskDelay( time / portTICK_PERIOD_MS ); },

  .xSemaphoreTake = [] { return xSemaphoreTake(xMutexLCD, portMAX_DELAY) == pdTRUE; },
  .xSemaphoreGive = [] { xSemaphoreGive(xMutexLCD); },

  .setCursor = [](int col, int row) { lcd.setCursor(col, row); },
  .lcd_print = [](char* str) { lcd.print(str); },
  .lcd_print_int = [](int value) { lcd.print(value); },
  .lcd_print_time = []{ lcd.printf("%02d:%02d:%02d", rtc.getHours(), rtc.getMinutes(), rtc.getSeconds()); },
  .lcd_clear = [] { lcd.clear(); },

  .set_hms = [](int h, int m, int s){
    hours = h;
    minutes = m;
    seconds = s;
  },
  .set_rtc_hms = [] {
    rtc.setHours(hours);
    rtc.setMinutes(minutes);
    rtc.setSeconds(seconds);
  },
};

typedef uint32_t (*ram_function)(uint32_t API_ADDRESS);

void setup() {
  Serial1.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  rtc.begin();

  xMutexLCD = xSemaphoreCreateMutex();

  initSD_Thread();
}

void loop()
{
  if(!SD.begin(SD_PIN)){
    Serial1.println("Insert SD card...");

    digitalWrite(PC13, LOW);
    delay(100);
    digitalWrite(PC13, HIGH);
    delay(100);

    delay(5000);

    initSD_Thread();
  }
}

bool loadBinToRAM(const char* filename, uint8_t buffer[]) {
  File file = SD.open(filename);
  if (!file) {
    Serial1.print("No file: ");
    Serial1.println(filename);
    return false;
  }
  
  fileSize = file.size();
  if (fileSize > CODE_BUFFER_SIZE) {
    Serial1.print(filename);
    Serial1.println(" too big");
    return false;
  }
  
  memset(buffer, 0, CODE_BUFFER_SIZE);
  
  file.read(buffer, fileSize);
  file.close();
  
  return true;
}

void initSD_Thread(){
  if(SD.begin(SD_PIN)){
    delay(1000);
    Serial1.println("SD: 1");
    Serial1.print("Adress api: 0x");
    Serial1.println((uint32_t)&api, HEX);

    if(loadBinToRAM("task1.bin", codeTask1)){
      Serial1.print("task1 code: 0x");
      Serial1.println((uint32_t)codeTask1, HEX);

      delay(1000);
      
      xTaskCreate(
        task1
        ,  NULL
        ,  512
        ,  NULL
        ,  1
        ,  NULL );

    }

    if(loadBinToRAM("task2.bin", codeTask2)){
      Serial1.print("task2 code: 0x");
      Serial1.println((uint32_t)codeTask2, HEX);

      delay(1000);
      
      xTaskCreate(
        task2
        ,  NULL
        ,  512
        ,  NULL
        ,  2
        ,  NULL );

    }

    vTaskStartScheduler();
  }

  else{
    Serial1.println("SD: 0");
  }
}

void task1(void* pvParameters) {
  ram_function func1 = (ram_function)((uint32_t)codeTask1 | 1);
  func1((uint32_t)&api);
}

void task2(void* pvParameters) {
  ram_function func2 = (ram_function)((uint32_t)codeTask2 | 1);
  func2((uint32_t)&api);
}
