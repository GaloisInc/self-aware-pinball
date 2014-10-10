/*
 * Pinball Switch Matrix Reader/Broadcaster
 * Daniel M. Zimmerman, Brian Huffman
 * Galois, Inc.
 */

/* The number of rows in the switch matrix, maximum 8 */
#define NUM_ROWS 8

/* The number of columns in the switch matrix, maximum 8 */
#define NUM_COLS 8

/* The Arduino register for the row inputs */
#define ROW_REGISTER PINF

/* The Arduino register for the column inputs */
#define COL_REGISTER PINK

/* The number of times to fail to detect a column sync before
   reporting a problem */
#define PROBLEM_DETECT 1000000

/* Flag for printing serial communication debug information */
#define COMMS_DEBUG 0

/* Flag for printing matrix state */
#define SERIAL_OUTPUT 1

// pin names
int led = 13;
int rows[] = { 54, 55, 56, 57, 58, 59, 60, 61 }; 
int cols[] = { 62, 63, 64, 65, 66, 67, 68, 69 }; 

// switch and LED state tracking
boolean lamp = false;
boolean state[NUM_COLS][NUM_ROWS] = { false };
int count[NUM_COLS][NUM_ROWS] = { 0 };

// setup routine
void setup() {                
  // initialize pins
  pinMode(led, OUTPUT);   
  for (int i = 0; i < NUM_ROWS; i++) { pinMode(rows[i], INPUT); }
  for (int i = 0; i < NUM_COLS; i++) { pinMode(cols[i], INPUT); }
  
  // initialize serial ports
  Serial.begin(115200);
  Serial1.begin(115200);
  if (SERIAL_OUTPUT) {  
    Serial.println("Initialized!");
  }
}

// the loop routine runs over and over again forever:
void loop() {
  boolean changed = false;
  int registers[NUM_COLS] = { 255 };
  
  int col = 0;
  while (col < NUM_COLS) {
    unsigned long wait = 0;
    while (!activeColumn(col)) { 
      /* busy wait */ 
      wait += 1;
      if (wait > PROBLEM_DETECT) {
        reportProblem();
        wait = 0;
      }
    }
    unsigned long time = micros();
    wait = 0;
    int temp = ROW_REGISTER;
    while (activeColumn(col)) {
      /* busy wait */ 
      wait += 1;
      if (wait > PROBLEM_DETECT) {
        reportProblem();
        wait = 0;
      }
    }
    if (micros() - time > 10) { 
      registers[col] = temp;
      col++; 
    } else {
      col = 0;
    }
  }

  // now we've read all the rows, let's update the switch state
  
  for (int col = 0; col < NUM_COLS; col++) {
    int current = registers[col];
    for (int row = 0; row < NUM_ROWS; row++) {
      boolean r = current & 1;
      current = current >> 1;
      if (!r) {
        if (count[col][row] == 0) {
          changed = true;
          state[col][row] = true; 
        }
        count[col][row] = 5;
      } else if (count[col][row] > 0) {
        count[col][row] -= 1;
        if (count[col][row] == 0) {
          changed = true;
          state[col][row] = false;
        }
      }
    }
  }  
  
  if (lamp) {
    digitalWrite(led, HIGH);
  } else {
    digitalWrite(led, LOW);
  }
  
  if (changed) {
    sendSwitchMatrix();
    if (SERIAL_OUTPUT) {
      printSwitchMatrix();
    }
    toggleLamp();
  } 
}

boolean activeColumn(int col) {
  return !((COL_REGISTER >> col) & 1);
}

void toggleLamp() {
  if (lamp) {
    digitalWrite(led, LOW);
  } else {
    digitalWrite(led, HIGH);
  }
  lamp ^= true;
}

void reportProblem() {
   if (SERIAL_OUTPUT) {
     Serial.println("Switch matrix sync problem!");
   }
   for (int i = 0; i < 3; i++) {
     digitalWrite(led, HIGH);
     delay(250);
     digitalWrite(led, LOW);
     delay(250);
   }
}

void sendSwitchMatrix() {
  Serial1.write(255);
  for (int row = 0; row < 8; row++) {
    if (row >= NUM_ROWS) {
      Serial1.write(0);
    } else {
      byte b = 0;
      for (int col = 8; col >= 0; col--) {
        b = (b << 1) | state[col][row];
      }
      Serial1.write(b);
      if (COMMS_DEBUG) {
        Serial.print("Sent row ");
        Serial.print(row);
        Serial.print(" as ");
        Serial.print(b);
        Serial.println(" over serial 1");
      }
    }
  }
}

void printSwitchMatrix() {
  for (int r = 0; r < NUM_ROWS; r++) {
    for (int c = 0; c < NUM_COLS; c++) {
      if (state[c][r]) {
        Serial.print("X ");
      } else {
        Serial.print("- ");
      }
    } 
    Serial.println();
  }
  Serial.println(micros());
  Serial.println();
}

