/**
 * Smart Parking Management System - RFID Card Initialization Module
 * 
 * This Arduino sketch handles the initialization of RFID cards for the parking system.
 * It writes the vehicle's license plate number and initial balance to the RFID card.
 * 
 * Hardware Requirements:
 * - Arduino Uno/Nano
 * - MFRC522 RFID Reader
 * - SPI interface
 * 
 * Data Structure on RFID Card:
 * - Block 2: Vehicle License Plate (7 characters)
 * - Block 4: Initial Balance
 * 
 * @author tony
 * @version 1.0
 */

#include <SPI.h>
#include <MFRC522.h>

// Pin definitions for RFID reader
#define RST_PIN  9 
#define SS_PIN   10  

// Initialize RFID reader and required variables
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode card_status;

/**
 * Setup function - Initializes the system
 * Configures serial communication and RFID reader
 */
void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println(F("==== CARD REGISTRATION ===="));
  Serial.println(F("Place your RFID card near the reader..."));
  Serial.println();
}

/**
 * Main loop - Handles card initialization process
 * 1. Waits for card detection
 * 2. Prompts for license plate
 * 3. Prompts for initial balance
 * 4. Writes data to card
 */
void loop() {
  // Initialize default key (0xFF 0xFF 0xFF 0xFF 0xFF 0xFF)
  for(byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Wait for card detection
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println(F("Card detected!"));

  // Buffers for storing data to be written
  byte carPlateBuff[16];
  byte balanceBuff[16];

  // Request and validate license plate number
  while (true) {
    Serial.println(F("Enter car plate number (must be exactly 7 characters, e.g., RAG234H), end with #:"));
    Serial.setTimeout(20000L); // 20 second timeout for input
    byte len = Serial.readBytesUntil('#', (char *)carPlateBuff, 16);

    if (len == 7) {
      padBuffer(carPlateBuff, len);
      break; // Valid input received
    } else {
      Serial.print(F("Invalid input length (got "));
      Serial.print(len);
      Serial.println(F(" characters). Try again.\n"));
      flushSerial();
    }
  }

  // Request and validate initial balance
  while (true) {
    Serial.println(F("Enter balance (max 16 characters), end with #:"));
    Serial.setTimeout(20000L); // 20 second timeout for input
    byte len = Serial.readBytesUntil('#', (char *)balanceBuff, 16);

    if (len > 0 && len <= 16) {
      padBuffer(balanceBuff, len);
      break; // Valid input received
    } else {
      Serial.println(F("Invalid balance input. Try again.\n"));
      flushSerial();
    }
  }

  // Define RFID data blocks for writing
  byte carPlateBlock = 2;  // Block for storing license plate
  byte balanceBlock = 4;   // Block for storing balance

  // Write data to RFID card
  writeBytesToBlock(carPlateBlock, carPlateBuff);
  writeBytesToBlock(balanceBlock, balanceBuff);

  // Display confirmation messages
  Serial.println();
  Serial.print(F("Car Plate written to block "));
  Serial.print(carPlateBlock);
  Serial.print(F(": "));
  Serial.println((char*)carPlateBuff);

  Serial.print(F("Balance written to block "));
  Serial.print(balanceBlock);
  Serial.print(F(": "));
  Serial.println((char*)balanceBuff);

  Serial.println(F("Please remove the card to write again."));
  Serial.println(F("--------------------------\n"));

  // Cleanup and prepare for next card
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(2000);
}

/**
 * Pads the buffer with spaces to ensure 16-byte length
 * @param buffer The buffer to pad
 * @param len Current length of the buffer
 */
void padBuffer(byte* buffer, byte len) {
  for(byte i = len; i < 16; i++) {
    buffer[i] = ' ';
  }
}

/**
 * Clears the Serial input buffer
 * Used to prevent processing of old input data
 */
void flushSerial() {
  while (Serial.available()) {
    Serial.read();
  }
}

/**
 * Authenticates and writes data to a specific block on the RFID card
 * @param block The block number to write to
 * @param buff The buffer containing data to write
 */
void writeBytesToBlock(byte block, byte buff[]) {
  // Authenticate with the card
  card_status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));

  if (card_status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(card_status));
    return;
  } else {
    Serial.println(F("Authentication success."));
  }

  // Write data to the card
  card_status = mfrc522.MIFARE_Write(block, buff, 16);
  if (card_status != MFRC522::STATUS_OK) {
    Serial.print(F("Write failed: "));
    Serial.println(mfrc522.GetStatusCodeName(card_status));
  } else {
    Serial.println(F("Data written successfully."));
  }
}