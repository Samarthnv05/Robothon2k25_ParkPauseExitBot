#include <TM1637Display.h>

// TM1637 Pins
#define CLK 6
#define DIO 2
TM1637Display display(CLK, DIO);

// IR Sensors
const int irLeft = A0;
const int irCenter = A1;
const int irRight = A2;

// Motor Pins
const int motorL1 = 9;
const int motorL2 = 10;
const int motorR1 = 11;
const int motorR2 = 12;
const int ena = 5; // ENA for left motor
const int enb = 3; // ENB for right motor

// LED
const int greenLED = 7;

int slotNumber;
bool lineDetected = false;
bool parked = false;
bool exited = false;
unsigned long startTime = 0;

const int speedThreshold = 150; // Adjust motor speed (0–255)

void setup() {
  Serial.begin(9600);

  // Motor setup
  pinMode(motorL1, OUTPUT);
  pinMode(motorL2, OUTPUT);
  pinMode(motorR1, OUTPUT);
  pinMode(motorR2, OUTPUT);
  pinMode(ena, OUTPUT);
  pinMode(enb, OUTPUT);

  // IR setup
  pinMode(irLeft, INPUT);
  pinMode(irCenter, INPUT);
  pinMode(irRight, INPUT);

  pinMode(greenLED, OUTPUT);
  digitalWrite(greenLED, LOW);  // Initially OFF

  display.setBrightness(5);  // Display brightness

  randomSeed(analogRead(A5));  // Seed for slot generation
}

void loop() {
  if (!lineDetected) {
    // Wait for black line detection (black = LOW)
    if (digitalRead(irLeft) == 0 || digitalRead(irCenter) == 0 || digitalRead(irRight) == 0) {
      digitalWrite(greenLED, HIGH); // Turn ON green LED
      slotNumber = random(1, 13);   // Generate random slot
      Serial.print("Slot: "); Serial.println(slotNumber);
      display.showNumberDec(slotNumber);
      delay(1000);
      lineDetected = true;
    }
  }
  else if (!parked) {
    followLine();

    if (millis() - startTime > 10000) {  // After 10 seconds of line following
      stopMotors();
      reverseIntoSlot();  // Reverse parking
      parkAndCountdown();
      parked = true;
    }
  }
  else if (!exited) {
    exitToGate();  // Move forward back to start
    exited = true;
    digitalWrite(greenLED, LOW); // Turn OFF green LED
  }
}

// Follow Line Logic
void followLine() {
  if (startTime == 0) startTime = millis();

  int left = digitalRead(irLeft);
  int center = digitalRead(irCenter);
  int right = digitalRead(irRight);

  if (center == 0) moveForward();
  else if (left == 0) turnLeft();
  else if (right == 0) turnRight();
  else stopMotors();
}

// Reverse into parking slot
void reverseIntoSlot() {
  Serial.println("Parking in reverse...");
  analogWrite(ena, speedThreshold);
  analogWrite(enb, speedThreshold);
  digitalWrite(motorL1, LOW);
  digitalWrite(motorL2, HIGH);
  digitalWrite(motorR1, LOW);
  digitalWrite(motorR2, HIGH);
  delay(1500);  // Time to reverse
  stopMotors();
}

// Parking Countdown
void parkAndCountdown() {
  Serial.println("Waiting in parking...");
  for (int i = 10; i >= 0; i--) {
    display.showNumberDec(i, true);
    delay(1000);
  }
  stopMotors();
}

// Exit the slot (go back to gate)
void exitToGate() {
  Serial.println("Returning to start...");
  moveForward();
  delay(3000);
  stopMotors();
}

// Motor Control
void moveForward() {
  analogWrite(ena, speedThreshold);
  analogWrite(enb, speedThreshold);
  digitalWrite(motorL1, HIGH);
  digitalWrite(motorL2, LOW);
  digitalWrite(motorR1, HIGH);
  digitalWrite(motorR2, LOW);
}

void turnLeft() {
  analogWrite(ena, 0);
  analogWrite(enb, speedThreshold);
  digitalWrite(motorL1, LOW);
  digitalWrite(motorL2, LOW);
  digitalWrite(motorR1, HIGH);
  digitalWrite(motorR2, LOW);
}

void turnRight() {
  analogWrite(ena, speedThreshold);
  analogWrite(enb, 0);
  digitalWrite(motorL1, HIGH);
  digitalWrite(motorL2, LOW);
  digitalWrite(motorR1, LOW);
  digitalWrite(motorR2, LOW);
}

void stopMotors() {
  analogWrite(ena, 0);
  analogWrite(enb, 0);
  digitalWrite(motorL1, LOW);
  digitalWrite(motorL2, LOW);
  digitalWrite(motorR1, LOW);
  digitalWrite(motorR2, LOW);
}
