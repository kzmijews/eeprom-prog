/*
    eeprom.h - Library for programming EEPROM memory.
    Created by Karol Zmijewski, May 30, 2024.
    Released into the public domain.
*/

#ifndef EEPROM_H
#define EEPROM_H

#include "Android.h"
#include<stdio.h>

class EEPROM {
    public:
        EEPROM();
        /**
         * Method to configure Arduino pins
         */
        void begin();
        /**
         * Method to set memory address, after setting the address read or write transaction can be triggered
         * M - Mandatory / O - Optional
         * @param address [unsigned int][M] - memory physical address
         * @param rw_bit [bool][M] - read/write, the last bit of the address that indicates transaction
         *     direction: 1 (true) - write / 0 (false) - read
         */
        void setAddress(unsigned int address, bool rw_bit);
        /**
         * Method to trigger memory read transaction
         * M - mandatory / O - Optional
         * @param address [unsigned int][M] - memory physical address
         * @return: byte of data read from memory
         */
        byte readEEPROM(unsigned int address);
        /**
         * Method to trigger memory write transaction
         * M - mandatory / O - Optional
         * @param address [unsigned int][M] - memory physical address
         * @param data [byte][M] - byte of data to write at given address
         */
        void writeEEPROM(unsigned int address, byte data);
        /**
         * Method to print on serial console (COM port) content of the EEPROM memory
         * M - mandatory / O - Optional
         * @param data_row_size [byte][M] - number of bytes printed in single line
         * @param data_total_size [unsigned int][M] - memory size to read (total number of bytes to read)
         */
        void printEEPROM(byte data_row_size, unsigned int data_total_size);
        /**
         * Function used to clean whole EEPROM memory data
         */
        void cleanEEPROM();
}

#endif