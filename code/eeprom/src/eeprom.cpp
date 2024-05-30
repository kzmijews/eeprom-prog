/*
    eeprom.cpp - Library for programming EEPROM memory.
    Created by Karol Zmijewski, May 30, 2024.
    Released into the public domain.
*/

#include "Arduino.h"
#include "eeprom.h"
#include<stdio.h>

// ARDUINO PINs layout
#define SER_PIN 2 // DATA SHIFT PIN
#define SCK_PIN 3 // CLK SHIFT PIN
#define RCK_PIN 4 // LATCH SHIFT PIN
#define EEPROM_D0_PIN 5
#define EEPROM_D7_PIN 12
#define WRITE_EN_PIN 13

// EEPROM settings
#define WRITE_EN_MASK 0x80      // 8th pin (0b00000001)
#define WRITE_OP_TIME_MS 5      // time needed to fnish write operation in miliseconds
#define WRITE_SIG_DURATION_NS 1 // write signal duration time in nanoseconds, depends on EEPROM spec
#define EEPROM_SIZE 2048        // 2KB EEPROM memory
#define SERIAL_OUT_FREQ 57600


EEPROM::EEPROM() {
}

void EEPROM::begin() {
    pinMode(SER_PIN, OUTPUT);
    pinMode(SCK_PIN, OUTPUT);
    pinMode(RCK_PIN, OUTPUT);
    digitalWrite(WRITE_EN_PIN, HIGH); // prevent from acidental write to the EEPROM
    pinMode(WRITE_EN_PIN, OUTPUT);
}

void EEPROM::setAddress(unsigned int address, bool rw_bit) {
    shiftOut(SER_PIN, SCK_PIN, MSBFIRST, (address >> 8) | (rw_bit ? 0x00 : WRITE_EN_MASK));
    shiftOut(SER_PIN, SCK_PIN, MSBFIRST, address);
    // toggle the pin - make sure that clock pin is at low state before rasing it,
    // regarding to doc, action will be triggeredon rising edge of the signal
    digitalWrite(RCK_PIN , LOW);
    digitalWrite(RCK_PIN , HIGH);
    digitalWrite(RCK_PIN , LOW);
}

byte EEPROM::readEEPROM(unsigned int address) {
    for(int pin_id = EEPROM_D0_PIN; pin_id <= EEPROM_D7_PIN; pin_id++) {
        // Arduino input, EEPROM output
        pinMode(pin_id, INPUT);
    }
    setAddress(address, true);
    byte data = 0xFF;
    for(int pin_id = EEPROM_D7_PIN; pin_id >= EEPROM_D0_PIN; pin_id--) {
        data = (data << 1) + digitalRead(pin_id);
    }
    return data;
}

void EEPROM::writeEEPROM(unsigned int address, byte data) {
    for(int pin_id = EEPROM_D0_PIN; pin_id <= EEPROM_D7_PIN; pin_id++) {
        // Arduino output, EEPROM input
        pinMode(pin_id, OUTPUT);
    }
    setAddress(address, false);
    for(int pin_id = EEPROM_D0_PIN; pin_id <= EEPROM_D7_PIN; pin_id++) {
        digitalWrite(pin_id, data & 1);
        data = data >> 1;
    }
    digitalWrite(WRITE_EN_PIN, LOW);
    delayMicroseconds(WRITE_SIG_DURATION_NS);
    digitalWrite(WRITE_EN_PIN, HIGH);
    delay(WRITE_OP_TIME_MS);
}

void EEPROM::printEEPROM(byte data_row_size, unsigned int data_total_size) {
    Serial.println("\n");
    if(data_total_size > EEPROM_SIZE) {
        char* err_msg = (char*) malloc(122);
        sprintf(err_msg, "Cannot read %u bytes from EEPROM, because the max size of memory is equal to %u. Whole memory will be read:\0", data_total_size, EEPROM_SIZE);
        Serial.println(err_msg);
        free(err_msg);
        data_total_size = EEPROM_SIZE;
    }
    const byte BASE_ADDR_CHARS_SIZE = 2 * sizeof(unsigned int);        // two characters needed to represent one byte
    const byte ADDITIONAL_CHARS_SIZE = 2 * sizeof(char);               // additional characters: ':' and '\0' (end of string)
    const byte DATA_ROW_CHARS_SIZE = 2 * data_row_size * sizeof(char); // two characters needed to represent each byte of data in row
    const byte WHITE_CHARS_SIZE = data_row_size * sizeof(char);        // additional white character after each byte of data
    const byte TOTAL_ROW_SIZE = BASE_ADDR_CHARS_SIZE + ADDITIONAL_CHARS_SIZE + DATA_ROW_CHARS_SIZE + WHITE_CHARS_SIZE + TOTAL_ROW_SIZE;
    // fulfill row string of data
    char* p_serial_out_buf = (char*) malloc(TOTAL_ROW_SIZE);
    for(unsigned int base_addr = 0x00; base_addr < data_total_size; base_addr += data_row_size) {
        int serial_out_buf_offset = sprintf(p_serial_out_buf, "%08x: ", base_addr);
        for(unsigned int offset_addr = 0; offset_addr < data_row_size; offset_addr++) {
            serial_out_buf_offset += sprintf(p_serial_out_buf + serial_out_buf_offset, "%02x ", readEEPROM(base_addr + offset_addr));
        }
        p_serial_out_buf[serial_out_buf_offset] = '\0';
        Serial.println(p_serial_out_buf);
    }
    free(p_serial_out_buf);
    Serial.println("\n");
}

void EEPROM::cleanEEPROM() {
    const byte EMPTY_BYTE = 0xFF;
    for(unsigned int address = 0x0; address < EEPROM_SIZE; address++) {
        byte curr_byte_val = readEEPROM(address);
        if(curr_byte_val != EMPTY_BYTE) {
            writeEEPROM(address, EMPTY_BYTE);
        }
    }
}
