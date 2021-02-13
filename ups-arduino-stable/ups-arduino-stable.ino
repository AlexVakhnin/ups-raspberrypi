//
// SMART UPS FOR RRASPBERRY PI
//
// версия 1.2 (стабильная) / av899@ukr.net /Вахнин Александр/ =Cherkasy 2020= /
//
// программа написана в стиле "Программный автомат" на сегодняшний день оттестирована и работает стабильно...
//

#include <avr/io.h>
#include <avr/interrupt.h>
#define LEDPIN_12 12
#define LEDPIN_11 11
#define SETPIN_10 10
#define SCANPIN_9 9

#define KEYPIN_2  2    // кнопка на выводе D2
#define KEYPIN_3  3    // кнопка на выводе D3

#define H_BAT  730     //верхний порог гистерезиса 11,5 вольт(767)
#define L_BAT  681     //нижний порог гистерезиса 10 вольт


volatile byte buttonState = 0;         // текущее состояние кнопки
volatile byte lastButtonState = 1;     // предыдущее состояние кнопки (1- кнопка отжата)
volatile byte buttonCounter = 0;       //счетчик времени на отжатие всех кнопок (антидребезг)
volatile static byte keyCode = 0;             // код нажатой клавиши, (0-ожидание нажатия)


volatile int seconds;  //глобальная переменная счетчик
volatile int seconds1;  //глобальная переменная счетчик 1
volatile int sound = 0;  //управление звуковыми сигналами

char s_buff [10] ; //буфер для ввода команды
byte pos=0;  //указатель на символ конца команды

int sensorPin = A0;    // номер аналогового входа
volatile int sensorValue = 0;  // переменная зля хранения значения АЦП
volatile int sensorValue1 = 0;  // округленное значение АЦП
volatile int wdt_link_pc = 0;  //таймер - связь с компьютером (0-нет связи)
volatile int shutdown_counter = 0; //время отведенное на shutdown компьютера
volatile int state_value = 0;  //состояние контроллера - ожидание заряда=10/ожидание разряда=20/время реакции=30/время shutdown=40/пауза=50
volatile byte flag_z = 0;  // 1- компьютер получил от контроллера команду shutdown
volatile byte flag_pause = 0;  // 1- ручной режим управления питанием

// the setup function runs once when you press reset or power the board
void setup() {

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);     //D13 звуковой сигнал
  pinMode(LEDPIN_12, OUTPUT);       //D12 светодиод контроля батареи
  pinMode(LEDPIN_11, OUTPUT);       //D11 светодиод контроля связи с ПК
  pinMode(SETPIN_10, OUTPUT);       //D10 управление реле
  pinMode(SCANPIN_9, INPUT);         //D9 сканирование напряжения сети 220в. 
  pinMode(KEYPIN_2,  INPUT_PULLUP);  //клавиатурный вход1 , резистор 20к на +5в. встроенный
  pinMode(KEYPIN_3,  INPUT_PULLUP);  //клавиатурный вход2 , резистор 20к на +5в. встроенный
  
  //инициализвция COM-порта
  Serial.begin(9600);
  for (pos =0;pos<10;pos++) s_buff[pos]=0x0; //обнуляем буфер ввода команды
  pos=0;  //указатель в начало

  // инициализация Timer1
  cli(); // отключить глобальные прерывания
  TCCR1A = 0; // установить регистры в 0
  TCCR1B = 0; 

  OCR1A = 15624; // установка регистра совпадения - определяет 1 секунду
  TCCR1B |= (1 << WGM12); // включение в CTC режим

  // Установка битов (CS10=1, CS12=1) на коэффициент деления 1024
  TCCR1B |= (1 << CS10);
  TCCR1B |= (0 << CS12);
  TIMSK1 |= (1 << OCIE1A);  // включение прерываний по совпадению

  // Чтение АЦП после RESET и проверка условия подачи питания PC
  sensorValue = analogRead(sensorPin);delay(1); // чтение АЦП
  sensorValue = analogRead(sensorPin);//контрольный
  if (sensorValue1 >= H_BAT){ //больше 12 вольт - возможно только при технологических работах...
                            //сброс контроллера обычно происходит только после разрядя батарреи
    digitalWrite(SETPIN_10, HIGH); //включить питание PC
    state_value = 20; //ожидаем разряд
  } else {
    state_value = 10; //состояние ожидания заряда батарреи
  }

  seconds = 0;//счетчики
  seconds1 = 0;

  sei(); // включить глобальные прерывания (задача с state_value)
}


// the loop function runs over and over again forever
void loop() {
  
  for (pos=0;pos<8;pos++)  //проход по массиву 
  {
     while ( Serial.available() <= 0 ) { //цикл ожидания символа
          //только в этом цикле вся фоновая обработка может быть !!!
          sensorValue = analogRead(sensorPin);delay(1); // чтение АЦП
          //обработка кнопки
          if (keyCode==1){  //была нажата кнопка с кодом "1"
            key_tasks();  // выполнение действий по нажатию кнопки.
            keyCode=0;  //признак - нажатие кнопки обработано...
          }
     }
     char s = Serial.read();  //принимаем очередной символ
     if ((s=='\n')||(s=='\r')) { //если очередной символ - конец строки, то в буфере может быть какая то команда.
      
        //анализ буфера с информацией от serial !
        if (pos>1 && pos<4) { //команды могут иметь 2 или 3 символа только (установка ограничения)
           //Serial.print(s_buff);Serial.print("\n");
           if (strcmp(s_buff,"at")==0) { Serial.print("ok");Serial.print("\n");wdt_link_pc=10;  } //пустая команда
           else if (strcmp(s_buff,"at1")==0){  Serial.print(sensorValue1,DEC);Serial.print("\n") ; wdt_link_pc=10; } //чтение округленного значения АЦП (отладка)
           else if (strcmp(s_buff,"at0")==0){  Serial.print(sensorValue,DEC);Serial.print("\n") ; wdt_link_pc=10; }  //чтение исходного значения АЦП (отладка)
           else if (strcmp(s_buff,"at2")==0){  Serial.print(digitalRead(SCANPIN_9),DEC);Serial.print("\n") ; wdt_link_pc=10; } //наличие питания 220в
           /*else if (strcmp(s_buff,"at5")==0){  Serial.print("ok");Serial.print("\n"); sound=3; state_value = 10; wdt_link_pc=10; }*/
           else if (strcmp(s_buff,"atb")==0){  Serial.print(digitalRead(LEDPIN_12),DEC);Serial.print("\n") ; wdt_link_pc=10; } //если 0->shutdown (команда для PC !!!)
           else if (strcmp(s_buff,"ats")==0){  Serial.print("ok");Serial.print("\n"); digitalWrite(SETPIN_10, HIGH);state_value = 20; wdt_link_pc=10; } //вкл. питание (отладка)
           else if (strcmp(s_buff,"atr")==0){  Serial.print("ok");Serial.print("\n"); digitalWrite(SETPIN_10, LOW);state_value = 10; wdt_link_pc=10; } //выкл. питание (отладка)
           else if (strcmp(s_buff,"atz")==0){  Serial.print("ok");Serial.print("\n");flag_z=1; wdt_link_pc=10; }  //подтверждение shutdown (команда от PC !!!)
           else if (strcmp(s_buff,"ati")==0){  Serial.print("St=");Serial.print(state_value,DEC); //информация о батарейке (отладка)
                    Serial.print(" Vbat=");Serial.print(sensorValue1*0.0145,2);Serial.print("(");Serial.print(H_BAT*0.0145,2);Serial.print("/");
                    Serial.print(L_BAT*0.0145,2);Serial.print(")");
                    Serial.print(" pause=");Serial.print(flag_pause,DEC);
                    Serial.print(" Fbat=");Serial.print(digitalRead(LEDPIN_12),DEC);Serial.print(" Fz=");Serial.print(flag_z,DEC);
                    Serial.print(" F220=");Serial.print(digitalRead(SCANPIN_9),DEC);
                    Serial.print(" WDT=");Serial.print(shutdown_counter,DEC);Serial.print("\n") ; wdt_link_pc=10; }
        }
        break;
     } else {
        //Serial.print(s,DEC);
        s_buff[pos]=s;
     }
  }  //цикл по массиву
  
  for (pos =0;pos<10;pos++) s_buff[pos]=0x0; //обнуляем буфер ввода, начнем сначала

}  //бесконечный цикл


ISR(TIMER1_COMPA_vect) //это обработка прерываия от таймера по совпадению с регистром
{
     
//digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    seconds++;
    if(seconds == 32) //период опроса клавиатуры (1024 для отладки)
    {
        seconds = 0;
        seconds1++;
          if(seconds1 == 40){   //делитель относительно частоты опроса клавиатуры
            seconds1 = 0;
            link_pc (); //контроль связи с компьютером
            charge_state(); // обработка состояний заряда батарреи (период 1 сек.)
          }
        
        //Звук
        sound_control(); //вызов с частотой опроса клавиатуры
        
        //обработка клавиатуры
        //digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));//индикация частоты опроса клавиатуры
        buttonState = digitalRead(KEYPIN_2); //читаем вход
        
        if ( (buttonState != lastButtonState)&&(buttonState==LOW)) {  //событие нажатия кнопки  --\__
                    if ((buttonCounter==0) && (keyCode==0)){ //если таймаут на отпускание выдержан и предыдущий код обработан
                           keyCode=1; //выставляем код нажатия клавиши
                           
                    }
                    //Serial.println("FALL DOWN");
        }
        else if ((buttonState != lastButtonState)&&(buttonState==HIGH)) { //событие отпускания кнопки  __/--
                    buttonCounter = 10;  //выставляем таймаут на отпускание (флаг запрета исключает дребезг контактов)
                    //Serial.println("RISE UP");
        }
        else if ((buttonState == lastButtonState)&&(buttonState==HIGH)) { //кнопка в ненажатом состоянии
                    if (buttonCounter > 0 ) {buttonCounter--;} //счетчик циклов на отпускание клавиши (антидребезг) 
                    //Serial.print(buttonCounter,DEC); Serial.print("/"); Serial.println(keyCode,DEC); 
        }
        else if ((buttonState == lastButtonState)&&(buttonState==LOW)) { //кнопка в нажатом состоянии
                    //Serial.println("LOW");
        }
        lastButtonState=buttonState;  // все события обработали, можно обновить предыдущее состояние
      }
}

//контроль связи с компьютером (вызов через 1 сек)
void link_pc ()
{
    if (wdt_link_pc == 0)
         {   digitalWrite(LEDPIN_11, !digitalRead(LEDPIN_11));}//wdt разряжен - моргаем
    else
         {   digitalWrite(LEDPIN_11, HIGH);}                     //wdt заряжен -  светим

  if (wdt_link_pc > 0){wdt_link_pc--;}//уменьшаем с каждым циклом , пока не 0
}

//обработка состояний автоматики
void charge_state()
{
  //выполняем округление АЦП
  if (abs(sensorValue1-sensorValue)>=3){sensorValue1=sensorValue;}
  //Отрабатываем гистерезис
  if (state_value == 10){ //ожидаем верхний уровень (ЗАРЯД)
      //устанавливаем порты
      if (sensorValue1 >= H_BAT){ //индикация состояния батареи - 1
        digitalWrite(LEDPIN_12, HIGH);  //информация для PC, что заряд еще есть
        if (digitalRead(SETPIN_10)==LOW){digitalWrite(SETPIN_10, HIGH);}  //включаем питание, если оно небыло включено
        flag_z=0; //сброс флага команды shutdown
        sound=4;
        state_value = 20; //переход на ожидание разряда батарреи
      }
  }
  if (state_value == 20){ //ожидаем нижний уровень (РАЗРЯД)
      //устанавливаем порты
      if (sensorValue1 <= L_BAT){ //(681)индикация состояния батареи - 0
        flag_z=0; //для контроля получения команды shutdown компьютером
        digitalWrite(LEDPIN_12, LOW);  //информация для PC-батарея разряжена, необходим shutdown !!!
        shutdown_counter = 6; //время отведенное на реакцию компьютера на команду shutdown
        sound=4;
        state_value = 30; //переход на отработку shutdown
      }
  }
  if (state_value == 30){  //ожидаем подтверждения команды shutdown от компьютера
      if (flag_z==0){sound=2;} //бикать, если еще нет "z"
      if (shutdown_counter ==0){
        if (flag_z==0){sound=2;}// продолжать бикать, если что то не так...
        if ((wdt_link_pc == 0)&&(flag_z==1)){ //проверка что PC уже не работает
            flag_z=0; //сбросим его от греха подальше...
            //sound=8; //команда принятя компьютером
            shutdown_counter = 24; //время отведенное на выполнение shutdown компьютером
            state_value = 40; //переход на выключение питания
        } /*else {state_value = 10;} //переход на ожидание зарядки батарреи - PC не подключен...*/
      }    
      if (shutdown_counter > 0){shutdown_counter--;}//уменьшаем с каждым циклом , пока не 0
  }
  if (state_value == 40){  //выключение питания компьютера
      if (shutdown_counter ==0){
            digitalWrite(SETPIN_10, LOW); // выключакм питание компьютера
            sound=4;
            if (flag_pause==1) {  //инициатор shutdown - кнопка
                flag_pause==0;
                state_value = 50; //переход на состояние "пауза" (ручной режим работы)
            } else {  //штатный автоматический режим работы
                state_value = 10; //переход на ожидание зарядки батарреи
            }
      }    
      if (shutdown_counter > 0){shutdown_counter--;}//уменьшаем с каждым циклом , пока не 0
  }
  if (state_value == 50){  //питание выключено а режиме "пауза" , ожидаем включение питания кнопкой
    //состояние необходимо для того, чтобы автоматика сама не включала питпние PC, если условия заряда батареи позволяют
    //индикация пауза
  }
  
 }

 void sound_control (){ //издаем звковой сигнал

  //digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  if (sound >0){digitalWrite(LED_BUILTIN, HIGH);}
  else {digitalWrite(LED_BUILTIN, LOW);}
  if (sound > 0){sound--;}//уменьшаем с каждым циклом , пока не 0
 }

 void key_tasks(){  //все задачи, которые выполняем по нажатии кнопки

  if (state_value == 10) {  //форсируем включение питание компьютера
    if ((sensorValue1 > L_BAT)&&(digitalRead(SCANPIN_9)==1)) { //батарея выше нижнего порога и есть 220в
        digitalWrite(LEDPIN_12, HIGH);  //информация для PC, что заряд еще есть
        if (digitalRead(SETPIN_10)==LOW){digitalWrite(SETPIN_10, HIGH);} //включаем питание, если оно небыло включено
        flag_z=0; //сброс флага команды shutdown
        sound=4;
        state_value = 20; //переход на ожидание разряда батарреи
    } else {
        sound = 2;delay(100);sound = 2; //двойной писк - типа пропускаем нажатие кнопки
    }
    
  } else if (state_value == 20) {  //вызываем выполнение процесса shutdown - перевод "пауза"
        flag_z=0; //для контроля получения команды shutdown компьютером
        digitalWrite(LEDPIN_12, LOW);  //информация для PC-батарея разряжена, необходим shutdown !!!
        shutdown_counter = 6; //время отведенное на реакцию компьютера на команду shutdown
        sound=4;
        flag_pause=1; //перевод в ручной режим управления питанием
        state_value = 30; //переход на отработку shutdown (ожидание подтверждения PC)

  } else if (state_value == 30) { //контроллер ожидает подтверждения "z" и пищит 
    if (digitalRead(SCANPIN_9)==0) { //220в отсутствует
      flag_z=1;  //продолжаем shutdown
      sound = 4;
    } else {  //включено питание 220в
      if (flag_pause==0) {  //инициатор начала shutdown - автомат
                            //попытка работы в автоматическом режиме без USB кабеля... )))
        if (sensorValue1 > L_BAT) { //уровень батареи выше нижнего порога
          digitalWrite(LEDPIN_12, HIGH);  //информация для PC, что заряд еще есть
          flag_z=0; //сброс флага команды shutdown
          sound=4;
          state_value = 20; //переход на ожидание разряда батарреи
                
        } else {  //уровень батареи ниже нижнего порога
          flag_z=1;  //продолжаем shutdown
          sound = 4;
        }
      } else {  //инициатор начала shutdown - кнопка
         flag_z=1;  //продолжаем shutdown
         sound = 4;          
      }
    }
  } else if (state_value == 50) {  //контроллер в режиме "пауза"
      if (((sensorValue1 > L_BAT)&&(digitalRead(SCANPIN_9)==1))||(sensorValue1 > H_BAT)) { //батарея выше нижнего порога и есть 220в или выше верхнего
        digitalWrite(LEDPIN_12, HIGH);  //информация для PC, что заряд еще есть
        if (digitalRead(SETPIN_10)==LOW){digitalWrite(SETPIN_10, HIGH);} //включаем питание, если оно небыло включено
        flag_z=0; //сброс флага команды shutdown
        flag_pause=0; //выходим из паузы
        sound=4;
        state_value = 20; //переход на ожидание разряда батарреи        
      } else {sound = 2;delay(100);sound = 2;} 
  } else {
    sound = 2;delay(100);sound = 2; //двойной писк - типа пропускаем нажатие кнопки
  }
 }
