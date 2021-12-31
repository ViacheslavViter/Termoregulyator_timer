// подключение энкодеров ===============================================================================
#include "GyverEncoder.h"         // библиотека энкодера
Encoder enc1(6, 5, 4);            // энкодер таймера, CLK пин 6, DT пин 5, SW пин 4
Encoder enc2(10, 8, 7);           // энкодер терморегулятора, CLK пин 10, DT пин 8, SW пин 7 
// подключение терморезистора ==========================================================================
#include <GyverNTC.h>             // библиотека термистора
GyverNTC therm(0, 100000, 3950, 25, 10000);  // пин A0, R терм, B терм, температура, R резистора
// подключение дисплеев ================================================================================
#include "GyverTM1637.h"          // библиотека дисплея
GyverTM1637 disp1(2, 3);          // дисплей таймера, CLK пин 2, DIO пин 3
GyverTM1637 disp2(11, 12);        // дисплей терморегулятора, CLK пин 11, DIO пин 12
// подключение энергонезависимой памяти ================================================================
#include <EEPROM.h>               // библиотека памяти
int e_temp = 0;                   // 0 - ячейка хранения значения настраиваемой температуры
int e_delta = 2;                  // 2 - ячейка хранения значения дельты
int e_power = 4;                  // 4 - ячейка хранения значения мощности
// стартовые настройки таймера и секундомера ===========================================================
int regime = 1;                   // счетчик режимов (таймер - секундомер)
int second=0;                     // секунды     
unsigned char minute=0;           // минуты                        
unsigned char hour=0;             // часы
unsigned char halfsecond=0;       // счетчик полсекунд
// настройки таймера ===================================================================================
int timerFlag=0;                  // флаг таймера
int vremyaTimer=30;               // счетчик времени
int spTimer=2;                    // счетчик режимов (старт - пауза)
unsigned long timeTimer=0;        // флаг времени  
unsigned long startTimer=0;       // флаг старта
#define buzPin 13                 // пин подключения бузера
// настройки секундомера ===============================================================================
int sekundomerFlag=0;             // флаг секундомера
int vremyaSekundomer=0;           // счетчик времени
int spSekundomer=2;               // счетчик режимов (старт - пауза)
unsigned long timeSekundomer=0;   // флаг времени 
unsigned long startSekundomer=0;  // флаг старта 
// настройки терморегулятора ===========================================================================
unsigned long timeDisplay;        // время вывода на дисплей
unsigned long ten_time;           // время включения/выключения нагрузки
unsigned long timeRegim;          // время возврата в основной режим 
byte porog;                       // порог уменьшения мощности (дельта) тэна
int pow_ten;                      // процент мощности нагрузки
int sett;                         // настраиваемая температура
int configTDP=1;                  // счетчик режимов (температура - дельта и мощность)
int regim = 1;                    // счетчик режимов (температура - установка)
int config_DP=1;                  // счетчик режимов (дельта - мощность)
#define tenPin A1                 // пин подключения нагрузки
// =====================================================================================================

void setup() {
  Serial.begin(9600);
  enc1.setTickMode(AUTO);         // выбор режима работы энкодера 1
  enc2.setTickMode(AUTO);         // выбор режима работы энкодера 2
  disp1.clear();                  // очистка дисплея
  disp1.brightness(7);            // яркость, 0 - 7
  disp2.clear();                  // очистка дисплея
  disp2.brightness(2);            // яркость, 0 - 7
  pinMode(buzPin, OUTPUT);        // пин подключения бузера
  pinMode(tenPin, OUTPUT);        // пин подключения нагрузки
}

void loop() {
  timer_sekundomer();             // функция таймера и секундомера
  termoregulyator();              // функция терморегулятора
}

void timer_sekundomer(){
  enc1.tick();                        // опрос энкодера таймера и секундомера
// переключатель секундомера и таймера =================================================================
  if (enc1.isHolded()){               // если удерживаем кнопку энкодера то...
    delay (50);                     
    regime ++;                        // ...добавляем 1 к счетчику режимов
    delay (50);
    second =0;}                       // при переходе в следующий режим обнуляем секунды
  if (regime ==3){                    // если счетчик режимов равен 3 то...
    regime=1;}                        // ...присваиваем счетчику режимом значение 1

// Режим 1. Таймер =====================================================================================
  if(regime==1){                      // если режим равен 1, то...
    startSekundomer=0;                // ...останавливаем секундомер,...
    spSekundomer=2;                   // ...устанавливаем режим пауза секундомера,...
    vremyaSekundomer=0;               // ...задаем начальное время секундомера
// настройка времени таймера ===========================================================================  
  if (enc1.isRight()){                // если был поворот энкодера вправо то...
    delay (50), vremyaTimer++;}       // ...добавляем к времени таймера значение 1
  if (enc1.isLeft()){                 // если был поворот энкодера влево то...
    delay (50), vremyaTimer--;        // ...отнимаем от времени таймера значение 1
    if(vremyaTimer==-1){              // если время таймера меньше 1 то...
      vremyaTimer=0;}}                // ...присваиваем времени таймера значение 0
      if (enc1.isRightH()){           // если было удержание + поворот вправо то...
        vremyaTimer+=60;              // ...добавляем к времени таймера значение 60 (1 час)
        if (vremyaTimer>=300)         // если время таймера равно 300 (5 часов) то...
        {vremyaTimer=300;}}           // ...присваиваем времени таймера значение 300 (5 часов)
 if (enc1.isLeftH()){                 // если было удержание + поворот влево то...
  vremyaTimer-=60;                    // ...отнимаем от времени таймера значение 60 (1 час)
  if (vremyaTimer<=60)                // если время таймера меньше 60 то...
  {vremyaTimer=0;}}                   // ...присваиваем времени таймера значение 0
     
// переключатель режимов старт и пауза таймера =========================================================
  if (enc1.isPress()){                        // если была нажата кнопка энкодера то...
    delay (50), spTimer ++;}                  // ...добавляем 1 к режиму
  if (spTimer ==3){                           // если режим равен 3, то...
    spTimer=1;}                               // ...присваиваем режиму значение 1
// старт таймера =======================================================================================   
  if (spTimer==2){                            // если включен режим старт то...
    timerFlag=1;                              // ...запускаем таймер (флаг =1),...
    startTimer=millis();                      // ...задаем точку отсчета,...
    halfsecond = 0;}                          // ...присваиваем счетчику полсекунд 0
  if (timerFlag==1){                          // если таймер включен (флаг =1), то...
    timeTimer = (millis()- startTimer);       // ...присваиваем счетчику времени таймера время пройденое от точки отсчета
    if (timeTimer>=500){                      // если время счетчика таймера пройденое от точки отсчета равно 500 (полсекунды), то...
      halfsecond ++;                          // ...к счетчику полсекунд добавляем 1, и...
      startTimer=millis();}                   // ...опять задаем точку отсчета
    if(halfsecond == 2){                      // если счетчик полсекунд равен 2 то...
      halfsecond = 0;                         // ...обнуляем его и...
      second-- ;}                             // ...уменьшаем значение секунд на 1
    if(second ==-1){                          // если значение секунд равно -1 то...
      second = 59;                            // ...присваиваем значению секунд 59 и...
      vremyaTimer--;}}                        // ...уменьшаем время таймера на 1
// когда время истекло ==================================================================================      
  if (timerFlag==1 && vremyaTimer==-1){       // если таймер запущен (флаг =1) и время таймера равно -1, то...
    second = 0;                               // ...присваиваем значению секунд 0...
    startTimer=0;                             // ...останавливаем таймер,...
    vremyaTimer=0;                            // ...останавливаем время таймера,...
    timeTimer=0;                              // ...обнуляем счетчик времени таймера,...
    timerFlag=0;                              // ...обнуляем флаг таймера,...
    disp1.point(POINT_OFF);                   // ...выключаем точки на дисплее,...
    disp1.displayByte(0x00, _E, _n, _d);      // ...выводим на дисплей End,...
    for (int i=0; i<3; i++) {                 // ...включаем счетчик повтора для бузера,...
      digitalWrite(buzPin, HIGH);             // ...включаем бузер,...
      delay(600);
      digitalWrite(buzPin, LOW);              // ...выключаем бузер
      delay(400);}}
// вывод информации на дисплей таймера =================================================================    
  hour=vremyaTimer/60;                    // вычисление значения часов из времени таймера
  minute=vremyaTimer-(hour*60);           // вычисление значения минут из времени таймера
  
  if (vremyaTimer>=60){                   // если время таймера больше 60, то на дисплей выводим...
    int8_t timeDisp[4];         
    timeDisp[0] = hour / 10;              // ... в сегмент 1 десятки часов...
    timeDisp[1] = hour % 10;              // ... в сегмент 2 единицы часов...
    timeDisp[2] = minute / 10;            // ... в сегмент 3 десятки минут...
    timeDisp[3] = minute % 10;            // ... в сегмент 4 единицы минут...
    disp1.display(timeDisp);           // все выводим на дисплей
    disp1.point(halfsecond ==1 ? POINT_OFF : POINT_ON);}  // включаем, выключаем точки
    
  if (vremyaTimer<60){                    // если время таймера меньше 60, то на дисплей выводим...
    int8_t timeDisp[4];          
    timeDisp[0] = minute / 10;            // ... в сегмент 1 десятки минут...
    timeDisp[1] = minute % 10;            // ... в сегмент 2 единицы минут...
    timeDisp[2] = second / 10;            // ... в сегмент 3 десятки секунд...
    timeDisp[3] = second % 10;            // ... в сегмент 4 единицы секунд...
    disp1.display(timeDisp);              // все выводим на дисплей
    disp1.point(halfsecond ==1 ? POINT_OFF : POINT_ON);}     // включаем, выключаем точки
} // -конец - Режим 1. Таймер --------------------------------------------------------------------------

// Режим 2. Секундомер =================================================================================
  if(regime==2){                          // если режим равен 2, то...
    startTimer=0;                         // ...останавливаем таймер,...
    spTimer=2;                            // ...устанавливаем режим пауза таймера,...
    vremyaTimer=30;                       // ...задаем начальное время таймеру 
// настройка времени секундомера =======================================================================  
  if (enc1.isRight()){                    // если был поворот энкодера вправо то...
    delay (50), vremyaSekundomer++;}      // ...добавляем к времени секундомера значение 1
  if (enc1.isLeft()){                     // если был поворот энкодера влево то...
    delay (50), vremyaSekundomer--;       // ...отнимаем от времени секундомера значение 1
    if(vremyaSekundomer==-1){             // если время секундомера равно -1 то...
      vremyaSekundomer=0;}}               // ...присваиваем времени секундомера значение 0
 // переключатель режима старт и режима пауза секундомера ==============================================
  if (enc1.isPress()){                    // если была нажата кнопка энкодера то...
    delay (50); spSekundomer ++;}         // ...добавляем 1 к счетчику режимов
  if (spSekundomer ==3){                  // если счетчик режимов равен 3 то...
    spSekundomer=1;}                      // ...присваиваем счетчику режимов значение 1
// старт секундомера ===================================================================================   
  if (spSekundomer==2){                             // если режим старт, то... 
    sekundomerFlag=1;                               // ...запускаем секундомер (флаг =1),...
    startSekundomer=millis();                       // ...задаем точку отсчета,...
    halfsecond = 0;}                                // ...присваиваем счетчику полсекунд 0
  if (sekundomerFlag==1){                           // если секундомер включен (флаг =1), то...
    timeSekundomer = (millis()- startSekundomer);   // ...присваиваем счетчику времени секундомера время пройденое от точки отсчета
    if (timeSekundomer>=500){                       // если счетчик времени таймера равен 500 (полсекунды), то...
      halfsecond ++;                                // ...к счетчику полсекунд добавляем 1, и...
      startSekundomer=millis();}                    // ...опять задаем точку отсчета
    if(halfsecond == 2){                            // если счетчик полсекунд равен 2, то...
      halfsecond = 0;                               // ...обнуляем его и...
      second++ ;}                                   // ...увеличиваем значение секунд на 1
    if(second ==60){                                // если значение секунд равно 60, то...
      minute ++;                                    // ...увеличиваем значение минут на 1,...
      second = 0;                                   // ...присваиваем значению секунд 0 и...
      vremyaSekundomer++;}}                         // ...увеличиваем время таймера на 1
// вывод информации на дисплей секундомера =============================================================    
  hour=vremyaSekundomer/60;               // вычисление значения часов из времени секундомера
  minute=vremyaSekundomer-(hour*60);      // вычисление значения минут из времени секундомера
  
  if (vremyaSekundomer>=60){              // если время секундомера больше 60, то на дисплей выводим...
    int8_t timeDisp[4];         
    timeDisp[0] = hour / 10;              // ... в сегмент 1 десятки часов...
    timeDisp[1] = hour % 10;              // ... в сегмент 2 единицы часов...
    timeDisp[2] = minute / 10;            // ... в сегмент 3 десятки минут...
    timeDisp[3] = minute % 10;            // ... в сегмент 4 единицы минут...
    disp1.display(timeDisp);              // все выводим на дисплей
    disp1.point(halfsecond ==1 ? POINT_OFF : POINT_ON);}  // включаем, выключаем точки
    
  if (vremyaSekundomer<60){               // если время секундомера меньше 60, то на дисплей выводим...
    int8_t timeDisp[4];          
    timeDisp[0] = minute / 10;            // ... в сегмент 1 десятки минут...
    timeDisp[1] = minute % 10;            // ... в сегмент 2 единицы минут...
    timeDisp[2] = second / 10;            // ... в сегмент 3 десятки секунд...
    timeDisp[3] = second % 10;            // ... в сегмент 4 единицы секунд...
    disp1.display(timeDisp);              // все выводим на дисплей
    disp1.point(halfsecond ==1 ? POINT_OFF : POINT_ON);}     // включаем, выключаем точки
} // конец --- Режим 2. Секундомер ---------------------------------------------------------------------
}
void termoregulyator() {
  int temp = (therm.getTempAverage());      // переменная хранения показаний температуры
  sett = EEPROM.read(e_temp);               // читаем значение настраиваемой температуры с EEPROM
  enc2.tick();                              // опрос энкодера терморегулятора
 
// установка мощности тэна при достижении порога от установленой температуры ============================
  float coef =(porog*0.01);                       // расчет коэфициента дельты
  if ((temp < sett) && (temp >(sett*coef)))       // если текущая температура больше порога от установленой ...
    {if ((millis()- ten_time > 0 && millis()- ten_time < pow_ten))   // ...на pow_ten милисекунд...
      {digitalWrite(tenPin, HIGH);}               // ...включаем нагрузку
  else                                            // иначе...
    {digitalWrite(tenPin, LOW);}                  // ...отключаем нагрузку
  if (millis() - ten_time > 100)                  // через 100 (в т.ч. pow_ten) милисекунд...
    {ten_time = millis();}}                       // ...обнуляем таймер
  else
// установка 100% мощности нагрузки меньше порога от установленой температуры ==========================  
    if (temp < (sett * coef))                   // если текущая температура меньше порога установленой...
      {digitalWrite(tenPin, HIGH);}             // ...включаем нагрузку
  else                                          // иначе...
    if (temp >= sett)                           // если текущая температура больше установленой...
      {digitalWrite(tenPin, LOW);}              // ...отключаем нагрузку
      
// переключатель режимов отображения, установки и режима установки дельты и мощности ===================
  if (enc2.isHolded()){                         // если удерживаем кнопку энкодера то...
    delay (50); configTDP ++;}                  // ...добавляем 1 к счетчику режимов
  if (configTDP ==3){                           // если счетчик режимов равен 3 то...
    configTDP=1;}                               // ...присваиваем счетчику режимом значение 1

// режимы отображения и установки температуры ==========================================================
  if (configTDP == 1) {
    
// переключатель режимов ===============================================================================
  if (enc2.isPress())  {              // если нажимаем кнопку энкодера то...
    delay (150); regim ++;}           // ...добавляем 1 к счетчику режимов
  if (regim ==3) {                    // если счетчик режимов равен 3 то...
    regim=1;}                         // ...присваиваем счетчику режимов значение 1
    
// Режим 1. Вывод температуры с датчика ================================================================   
  if(regim==1){                       
// вывод температуры с датчика на дисплей ============================================================== 
  if (millis()-timeDisplay>1000)            // через одну секунду... 
    {timeDisplay=millis();                  // ...обнуляем счетчик времени дисплея и....
  disp2.displayInt(temp*10);}               // ...выводим температуру
                    // *10 так как у меня дисплей трехсимвольный, а плата для четырёхсимвольного
}// -конец- Режим 1. Вывод температуры с датчика -------------------------------------------------------

// Режим 2. Установка температуры ======================================================================
  if(regim == 2){                     
    if (enc2.isRight())               // если крутим энкодер вправо то...
      {delay (50), sett+=5;           // ...добавляем к настраиваемой температуре 5 градусов и...
      EEPROM.write(e_temp, sett);}    // ...записываем значения температуры в EEPROM
    if (sett>=250){                   // если значение настраиваемой температуры больше или равно 250 то...
      sett=250;}                      // ...присваиваем значению настраиваемой температуры 250
    if (enc2.isLeft()){               // если крутим энкодер влево то...
      delay (50), sett-=5;            // ...отнимаем от настраиваемой температуры 5 градусов и...
      EEPROM.write(e_temp, sett);}    // ...записываем значения температуры в EEPROM
    if (sett<=50){                    // если значение настраиваемой температуры меньше или равно 50 то...
      sett=50;}                       // ...присваиваем значению настраиваемой температуры 50
// вывод настраиваемой температуры на дисплей ==========================================================  
  disp2.displayInt(sett*10);          // ...выводим температуру
                    // *10 так как у меня дисплей трехсимвольный, а плата для четырёхсимвольного
// автоматический переход в режим отображения температуры ==============================================
  if (millis()- timeRegim > 15000)      // через 15 секунд...
    {timeRegim = millis();              // ...обнуляем таймер и...
    regim = 1;}                         // ...переходим в режим 1
} // -конец- Режим 2. Установка температуры ------------------------------------------------------------
} // -конец- режимы отображения и установки температуры ------------------------------------------------

// Установка дельты и мощности =========================================================================
  if (configTDP == 2){
    porog= EEPROM.read(e_delta);            // читаем значение дельты с EEPROM
    pow_ten = EEPROM.read(e_power);         // читаем значение мощности с EEPROM
// переключатель режимов ===============================================================================
  if (enc2.isPress()){                      // если нажимаем кнопку энкодера то...
    delay (150); config_DP ++;}             // ...добавляем 1 к счетчику режимов
  if (config_DP ==3){                       // если счетчик режимов равен 3 то...
    config_DP=1;}                           // ...присваиваем счетчику режимов значение 1
// Режим 1. Установка дельты ===========================================================================
  if (config_DP==1){
    regim=1;                              // присваиваем... чтоб возвратится в режим отображения температуры
    if (enc2.isRight()){                  // если был поворот энкодера вправо то...
      delay (50), porog++;                // ...добавляем к значению дельты 1
      EEPROM.write(e_delta, porog);}      // ...записываем значения дельты в EEPROM
    if (porog>=99){                       // если значение дельты больше или равно 99 то...
      porog=99;}                          // ...присваиваем значению дельты 99
    if (enc2.isLeft()){                   // если был поворот энкодера влево то...
      delay (50), porog--;                // ...отнимаем от значения дельты 1
      EEPROM.write(e_delta, porog);}      // ...записываем значения дельты в EEPROM
    if (porog<=92){                       // если значение дельты меньше или равно 92 то...
      porog=92;}                          // ...присваиваем значению дельты 92
// вывод настраиваемой дельты на дисплей =============================================================== 
  disp2.displayInt(porog*10);             // ...выводим значения дельты 
                     // *10 так как у меня дисплей трехсимвольный, а плата для четырёхсимвольного
  disp2.displayByte(0,0b11011110);        // в первый сегмент выводим "d."
// автоматический переход в режим отображения температуры ==============================================
 if (millis()- timeRegim > 15000)         // через 10 секунд...
    {timeRegim = millis();                // ...обнуляем таймер и...
           configTDP = 1;}                // ..переходим в основной режим
} // -конец- Режим 1. Установка дельты -----------------------------------------------------------------

// Режим 2. Установка мощности =========================================================================
  if (config_DP==2){
    regim=1;                              // присваиваем... чтоб возвратится в режим отображения температуры
    if (enc2.isRight()){                  // если был поворот энкодера вправо то...
      delay (50), pow_ten++;              // ...добавляем к значению мощности 1
      EEPROM.write(e_power, pow_ten);}    // ...записываем значения мощности в EEPROM
    if (pow_ten>=99){                     // если значение мощности больше или равно 99 то...
      pow_ten=99;}                        // ...присваиваем значению мощности 99
    if (enc2.isLeft()){                   // если был поворот энкодера влево то...
      delay (50), pow_ten--;              // ...отнимаем от значения мощности 1
      EEPROM.write(e_power, pow_ten);}    // ...записываем значения мощности в EEPROM
    if (pow_ten<=50){                     // если значение мощности меньше или равно 50 то...
      pow_ten=50;}                        // ...присваиваем значению мощности 50
// вывод настраиваемой мощности на дисплей ============================================================= 
  disp2.displayInt(pow_ten*10);           // ...выводим значения мощности
                       // *10 так как у меня дисплей трехсимвольный, а плата для четырёхсимвольного
  disp2.displayByte(0, 0b11110011);       // в первый сегмент выводим "P."
// автоматический переход в режим отображения температуры ==============================================     
   if (millis()- timeRegim > 15000)       // через 15 секунд...
    {timeRegim = millis();                // ...обнуляем таймер и...
           configTDP = 1;}                // ..переходим в основной режим
} // -конец- Режим 2. Установка мощности ===============================================================
} // -конец- Установка дельты и мощности ===============================================================
} 

  
