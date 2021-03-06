/* ===============================================
 * Project: XLCD, user commands
 * ===============================================
 * Autor:     Frank (xpix) Herrmann
 * Email:     xpixer@gmail.com
 * License:   all Free
 * Last edit: 30.08.2013
 */ 

void parse_command_line(char* line)
{
   if( line[1] == '?' ){
      // Display Menu from this device
      Serial.print(F("<XLCD "));
      Serial.print(VERSION);
      Serial.println(F(" MENU>"));

      Serial.println(F("<:b nr\tCall Button nr>"));
      #ifdef BUTTONS_A_ADC_PIN
      Serial.println(F("<:l\tLearn button values>"));
      #endif
      Serial.println(F("<:i ms\tSet or get interval in microseconds>"));

      #ifdef DEBUG
      Serial.println(F("<:s\tshow button values>"));
      #endif

      Serial.println(F("<:r\tReset device>"));
      return;
   }
   // Commands handle:
   #ifdef DEBUG
	if( line[1] == 's') return show_button();
	if( line[1] == 'f') return free_ram();
   #endif
   #ifdef BUTTONS_A_ADC_PIN
   if( line[1] == 'l') return learn_button();
   if( line[1] == 'b') return press_button(line);
   #endif
   if( line[1] == 'i') return setinterval(line);
   if( line[1] == 'r') return resetDevice(0);

   Serial.print(F("Cant parse this command: "));
   Serial.println(line);
}

#ifdef DEBUG

void show_button(){
	Serial.println(F("show buttons ... "));
	for (int i = 0; i < COUNT_OF(button_power); i++)
	{
		int value = get_set_button_power(i, 0);
		Serial.print("Button ");
		Serial.print(i);
		Serial.print(" ");
		Serial.println(value);
	}
}

void free_ram(){
	Serial.print(F("Free ram: "));
	Serial.println(freeRam());
}

#endif

#ifdef BUTTONS_A_ADC_PIN

void learn_button(){

   simpleThread_group_stop(group_one);

   Serial.println(F("learn buttons ... "));
   unsigned int buttonVoltage = 1000;
	for (int i = 0; i < COUNT_OF(button_power); i++)
   {
      unsigned int oldbuttonVoltage = buttonVoltage;
      Serial.print(F("<Please push button: "));
      Serial.print(i);
      Serial.println(F(" ...>"));
		boolean isB = false;
      for(;;){
			delay(200);
			buttonVoltage = analogRead( BUTTONS_A_ADC_PIN );
   		if(buttonVoltage < 1000 && buttonVoltage != oldbuttonVoltage) 
   			break;
		};
      #ifdef DEBUG
      Serial.print(F("<Set Button: "));
      Serial.print(i);
      Serial.print(F(" to: "));
      Serial.print(buttonVoltage);
      Serial.println(F(">"));
      #endif
		get_set_button_power(i, buttonVoltage);
   }
   Serial.println(F("<Button values saved!>"));

   simpleThread_group_restart(group_one);
}

void press_button(char* line){
   int button = atoi(split(line, " ", 1));
   if(button == 0){
      Serial.println(F("<No Button number given!>"));
      return;
   }      

   call_button(button);
}

#endif

void setinterval(char* line){
   int ms = atoi(split(line, " ", 1));
   if(ms > 0 || ms == -1){
      setinterval(ms);
   }
   else{
      Serial.print(F("<Interval: "));
      Serial.print(EEPROMReadInt(EEPROM_INTERVAL));
      Serial.println(F(">"));
   }
}

void setinterval(int ms){
   Serial.print(F("<Interval: "));
   Serial.print(ms);
   Serial.println(F(">"));

   EEPROMWriteInt(EEPROM_INTERVAL, ms);

   if(ms == -1){
      simpleThread_group_stop(group_one);
      return;
   }
   if(ms > 0){
      simpleThread_dynamic_setLoopTime(getPositions, ms);
      simpleThread_dynamic_setLoopTime(getStates, ms);

      simpleThread_group_restart(group_one);
	} 
}

void resetDevice(int n){
   Serial.println(F("Reset device ... "));
   grblSerial.write(0x18);
   delay(100);
   asm volatile ("jmp 0x0000");
}
