#include <Wire.h>
#define BLYNK_TEMPLATE_ID "TMPL3vdGSbgif"
#define BLYNK_TEMPLATE_NAME "FAL DETECTION SYSTEM"
#define BLYNK_AUTH_TOKEN "bMWhIe3uYSNeNbpjXakIBpvo0400Ik-T"
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
const int MPU_addr = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
float ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
boolean fall = false; //stores if a fall has occurred
boolean trigger1 = false; //stores if first trigger (lower threshold) has occurred
boolean trigger2 = false; //stores if second trigger (upper threshold) has occurred
boolean trigger3 = false; //stores if third trigger (orientation change) has occurred
byte trigger1count = 0; //stores the counts past since trigger 1 was set true
byte trigger2count = 0; //stores the counts past since trigger 2 was set true
byte trigger3count = 0; //stores the counts past since trigger 3 was set true
int angleChange = 0;

 // WiFi network info.
char auth[] = "bMWhIe3uYSNeNbpjXakIBpvo0400Ik-T"; //Auth code sent via Email
const char *ssid = "AndroidAP_3901"; // Enter your Wi-Fi Name
const char *pass = "yashnagj?19"; // Enter your Wi-Fi Password
void setup() {
  delay(1000); // 1-second delay
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial.println("Wrote to IMU");
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
 {
   delay(500);
   Serial.print(".");              // print … till not connected
  }
  Serial.println("");
  Serial.println("WiFi connected");
 }
 void loop() {
  Blynk.run();
  mpu_read(); // Read sensor data
  ax = (AcX - 2050) / 16384.00;
  ay = (AcY - 77) / 16384.00;
  az = (AcZ - 1947) / 16384.00;
  gx = (GyX + 270) / 131.07;
  gy = (GyY - 351) / 131.07;
  gz = (GyZ + 136) / 131.07;
   // calculating Amplitute vector for 3 axis
   float rawAmp = sqrt(ax * ax + ay * ay + az * az);
   int Amp = rawAmp * 10; // Scaled
 
   Serial.print("Amp: "); Serial.println(Amp);
 
   // Trigger 1: Low movement (free fall)
   if (Amp <= 2 && !trigger2) {
     trigger1 = true;
     Serial.println("TRIGGER 1 ACTIVATED");
   }
 
   // Trigger 2: Sudden high impact
   if (trigger1) {
     trigger1count++;
     if (Amp >= 9) { // TUNED threshold from 12 to 9
       trigger2 = true;
       trigger1 = false;
       trigger1count = 0;
       Serial.println("TRIGGER 2 ACTIVATED");
     }
   }
 
   // Trigger 3: Orientation change after fall
   float angleChange = sqrt(gx * gx + gy * gy + gz * gz);
   if (trigger2) {
     trigger2count++;
     Serial.print("Angle Change: "); Serial.println(angleChange);
 
     if (angleChange >= 30 && angleChange <= 400) {
       trigger3 = true;
       trigger2 = false;
       trigger2count = 0;
       Serial.println("TRIGGER 3 ACTIVATED");
     }
   }
 
   // Confirmed fall
   if (trigger3) {
     trigger3count++;
     if (trigger3count >= 10) {
       angleChange = sqrt(gx * gx + gy * gy + gz * gz);
       if (angleChange >= 0 && angleChange <= 10) {
         fall = true;
         trigger3 = false;
         trigger3count = 0;
         Serial.println("FALL DETECTED");
 
         // 🔔 Trigger Blynk alert
         Blynk.logEvent("fall_detected", "ALERT: Fall Detected! Take action immediately.");
       } else {
         trigger3 = false;
         trigger3count = 0;
         Serial.println("TRIGGER 3 DEACTIVATED");
       }
     }
   }
 
   // Reset triggers if timing out
   if (trigger1count >= 6) {
     trigger1 = false; trigger1count = 0;
     Serial.println("TRIGGER 1 DEACTIVATED");
   }
   if (trigger2count >= 6) {
     trigger2 = false; trigger2count = 0;
     Serial.println("TRIGGER 2 DEACTIVATED");
   }
 
   delay(100);
 }
void mpu_read() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false); // Communication open for nxt operation
  Wire.requestFrom((uint8_t)MPU_addr, (uint8_t)14, (uint8_t)true); // request a total of 14 registers
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
 }