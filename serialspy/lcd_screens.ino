/* ===============================================
 * Project: XLCD, LCD Screens
 * ===============================================
 * Autor:     Frank (xpix) Herrmann
 * Email:     xpixer@gmail.com
 * Editor:    Luca Zimmermann (http://arduino-hannover.de)
 * License:   all Free
 * Last edit: 07.05.2015
 */

boolean showWorkingPos = false; //toggle to show either machine or working positions
boolean externalLCDAccess = false; //toggle to stop display access by thread.

unsigned long lastCall = 0;
char GRBLxPos[10];
char GRBLyPos[10];
char GRBLzPos[10];
char GRBLstate[10];
boolean externalLCDAccessWas = false; //just for clearing the display again, if mode unset
int lenXPos = 0, lenYPos = 0, lenZPos = 0, lenState = 0;

void parse_status_line(char* line) {
  // <Idle,MPos:5.529,0.560,7.000,WPos:1.529,-5.440,-0.000>

  // Display on LCD ...
  // lcd screen
  // |--------------|
  // State  Z:555.529
  // 000.000  000.000
  
  if (externalLCDAccessWas != externalLCDAccess) {
    if (externalLCDAccessWas) {
      myLCD.clear();
    }
    externalLCDAccessWas = externalLCDAccess;
  }
  if (externalLCDAccess) {
    return;
  }

  char* temp;
  int len;
  char delim[] = "<,:>";
  // State ..
  temp = split(line, delim, 0);
  //if (strcmp(temp, GRBLstate) != 0) {
  myLCD.setCursor(0, 0);
  switch (state(temp)) {
    case IDLE:
      myLCD.write(IDLEICON); break;
    case QUEUE:
      //this status is no longer used by GRBL
      myLCD.write(104); break; //random chinese char which looks like a list.
    case HOLD:
      myLCD.write(HOLDICON); break;
    case RUN:
      myLCD.write(RUNICON); break;
    case HOME:
      myLCD.write(HOMEICON); break;
    case CHECK:
      myLCD.write(CHECKICON); break;
    default:
      myLCD.write(ALARMICON); break;
  }
  
  if (showWorkingPos) {
    myLCD.print(" Rel");
  } else {
    myLCD.print(" Abs");
  }

  // Z Machine position ...
  temp = split(line, delim, 4 + showWorkingPos * 4);
  if (strcmp(temp, GRBLzPos) != 0) {
    len = strlen(temp);
    for (int i = 0; i < len; i++) {
      GRBLzPos[i] = *temp;
      *temp++;
    }
    GRBLzPos[len] = '\0';
    myLCD.setCursor(LCD_cols - max(len, lenZPos), 0);
    for (int i = 0; i < lenZPos - len; i++) {
      myLCD.print(" ");
    }
    lenZPos = len;
    myLCD.print(GRBLzPos);
  }

  // X Machine Position ..
  temp = split(line, delim, 2 + showWorkingPos * 4);
  if (strcmp(temp, GRBLxPos) != 0) {
    len = strlen(temp);
    for (int i = 0; i < len; i++) {
      GRBLxPos[i] = *temp;
      *temp++;
    }
    GRBLxPos[len] = '\0';
    //strcpy(GRBLxPos, temp);
    myLCD.setCursor(0, 1);
    myLCD.print(GRBLxPos);
    for (int i = 0; i < lenXPos - len; i++) {
      myLCD.print(" ");
    }
    lenXPos = len;
  }

  // Y Machine position ...
  temp = split(line, delim, 3 + showWorkingPos * 4);
  if (strcmp(temp, GRBLyPos) != 0) {
    len = strlen(temp);
    for (int i = 0; i < len; i++) {
      GRBLyPos[i] = *temp;
      *temp++;
    }
    GRBLyPos[len] = '\0';
    //strcpy(GRBLyPos, temp);
    myLCD.setCursor(LCD_cols - max(len, lenYPos), 1);
    for (int i = 0; i < lenYPos - len; i++) {
      myLCD.print(" ");
    }
    lenYPos = len;
    myLCD.print(GRBLyPos);
  }

  lastCall = millis();
}

char GRBLplane[4];
char GRBLtool[4];
char GRBLfeed[5];
int lenFeed = 0;

// send every second the command $G
void parse_state_line(char* myBuffer) {
  // Display on LCD ...
  // |--------------|
  // S1 T1 F1000
  // MM LIN XY M1

  if (externalLCDAccess) {
    return;
  }
  
  char delim[] = "[ ]";

  //             mm                   TNr Feed
  // G0 G54 G17 G21 G90 G94 M0 M5 M9 T0 F500.000

  myLCD.setCursor(0, 2); //third row
  char *temp = split(myBuffer, delim, 7);
  if (strcmp(temp, "M5") == 0) {
    //myLCD.print("Off   ");
    myLCD.write(HOLDICON);
    myLCD.print("  ");
  } else if (strcmp(temp, "M4") == 0) {
    //myLCD.print("On ");
    myLCD.write(CCWARROW);
    myLCD.print("  ");
  } else {
    //myLCD.print("On ");
    myLCD.write(CWARROW);
    myLCD.print("  ");
  }

  // Move
  temp = split(myBuffer, delim, 0);
  if (strcmp(temp, "G") == 0 || strcmp(temp, "G0") == 0) {
    myLCD.write(243); //infinite = rapid
  } else if (strcmp(temp, "G1") == 0) {
    myLCD.write('-'); //linear
  } else if (strcmp(temp, "G2") == 0) {
    myLCD.write(CWARROW);
  } else if (strcmp(temp, "G3") == 0) {
    myLCD.write(CCWARROW);
  }

  // Feed
  temp = split(split(myBuffer, delim, 10), ".", 0);
  int tmp = strlen(temp) - 1;
  *temp++;  //skip 'F'
  for (int i = 0; i < tmp; i++) {
    GRBLfeed[i] = *temp;
    *temp++;
  }
  GRBLfeed[tmp] = '\0';
  myLCD.setCursor((LCD_cols - max(lenFeed, tmp) - 4), 2);
  for (int i = 0; i < lenFeed - tmp; i++) {
    myLCD.print(" ");
  }
  myLCD.print(strcat(GRBLfeed, "mm/s"));

  // next line
  myLCD.setCursor(0, 3);

  // mm or inch
  temp = split(myBuffer, delim, 3);
  if (strcmp(temp, "G21") == 0) {
    myLCD.print("mm ");
  } else {
    myLCD.print("in ");
  }

  // Plane
  temp = split(myBuffer, delim, 2);
  if (strcmp(temp, "G17") == 0)      myLCD.print(F("XY "));
  else if (strcmp(temp, "G18") == 0) myLCD.print(F("ZX "));
  else if (strcmp(temp, "G19") == 0) myLCD.print(F("YZ "));

  // Tool
  temp = split(myBuffer, delim, 9);
  tmp = strlen(temp);
  for (int i = 0; i < tmp; i++) {
    GRBLtool[i] = *temp;
    *temp++;
  }
  GRBLtool[tmp] = '\0';
  myLCD.print(GRBLtool);
  for (int i = 0; i < 5 - tmp; i++) {
    myLCD.print(" ");
  }

  // Flow (Pause, Stop ...)
  myLCD.setCursor((LCD_cols - 5), 3);  
  temp = split(myBuffer, delim, 6);
  if (strcmp(temp, "M0") == 0)        myLCD.print(F("Pause"));
  else if (strcmp(temp, "M2") == 0)   myLCD.print(F("  End"));
  else if (strcmp(temp, "M30") == 0)  myLCD.print(F("  End"));
  else                                myLCD.print(F(" Done")); //...?
}
