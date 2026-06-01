/*!
 *  @file pixmob_cement.cpp
 *
 *  @mainpage driver for the PixMob "cement V1.1" crowd-pixel
 *
 *  @section intro_sec Introduction
 *
 * 	driver for the PixMob "cement V1.1" crowd-pixel
 * 
 *  @section dependencies Dependencies
 *
 *  This library depends on the SmartRC-CC1101-Driver-Lib
 *
 *  @section author Author
 *
 *  Sueppchen and Serge-45
 *
 * 	@section license License
 *
 * 	BSD (see license.txt)
 *
 * 	@section  HISTORY
 *
 *     v0.2 - first update
 *     v0.3 - XIAO ESP32-C6 support
 */

#include "Arduino.h"
#include <SPI.h>
#include <pixmob_cement.h>

Pixmob::Pixmob(){}

bool Pixmob::begin(int pin) {

  SPI.begin(D8, D9, D10, D4);
  ELECHOUSE_cc1101.setSpiPin(D8, D9, D10, D4);

  _pin = pin;

  confirmRed    = CONFIRM_RED;
  confirmGreen  = CONFIRM_GREEN;
  confirmBlue   = CONFIRM_BLUE;
  globalAttack  = DEFAULT_ATTACK;
  globalHold    = DEFAULT_HOLD;
  globalRelease = DEFAULT_RELEASE;
  globalRandom  = DEFAULT_RANDOM;

  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setGDO0(_pin);
  pinMode(_pin, OUTPUT);
  ELECHOUSE_cc1101.setModulation(2);
  ELECHOUSE_cc1101.setCCMode(0);
  ELECHOUSE_cc1101.setMHZ(TX_FREQ);
  ELECHOUSE_cc1101.setChannel(0);

  return 1;
}

uint8_t Pixmob::lineCode(uint8_t inByte){
    inByte &= 0x3f;
    return dictTable[inByte];
}

void Pixmob::transmitBit(bool txBit){
    digitalWrite(_pin,txBit);
    delayMicroseconds(BIT_TIME);
}

void Pixmob::transmitByte(uint8_t txByte){
    for(uint8_t bitCounter = 0; bitCounter < 8; bitCounter ++) {
        bool bit = (((txByte) >> (bitCounter)) & 1);
        transmitBit( bit );
      }
}

void Pixmob::refresh(){
    ELECHOUSE_cc1101.SetTx();
    delayMicroseconds(START_DELAY);
    
    transmitByte(PREAMBLE);
    transmitByte(PREAMBLE);
    
    transmitBit(SYNC1);
    transmitBit(SYNC2);
    
    for(uint8_t byteCounter = 0; byteCounter < 9; byteCounter ++ )
      transmitByte(TXbuffer[byteCounter] );
    
    ELECHOUSE_cc1101.SetRx();
    digitalWrite(_pin, 0);
}

void Pixmob::setCRC() {
    uint8_t *ptr = TXbuffer;
    *ptr++;
    uint16_t reg = INITR;

    for (uint8_t i = 0; i < 7; i ++) {
      reg ^= (*ptr++);
      for (uint8_t count_bits = 8; count_bits != 0; count_bits--) {
        if (reg & 0x0001) {
          reg >>= 1;
          reg ^= POLYR;
        } else {
          reg >>= 1;
        }
      }
    }

    TXbuffer[0] = lineCode( (reg & 0x3f) );
    TXbuffer[8] = lineCode( (reg >> 6) );
}

void Pixmob::generateTXbuffer(uint8_t * message){
    for( uint8_t i = 0; i < 7; i ++){
      TXbuffer[(i+1)] = lineCode( message[i] );
    }
    setCRC();
}

void Pixmob::setFXtiming(uint8_t attack, uint8_t hold, uint8_t release, uint8_t random){
    globalAttack  = attack  & 0x7;
    globalHold    = hold    & 0x7;
    globalRelease = release & 0x7;
    globalRandom  = random  & 0x7;
}

void Pixmob::setConfirmColor(uint8_t red, uint8_t green, uint8_t blue){
  confirmRed   = red;
  confirmGreen = green;
  confirmBlue  = blue;
}

void Pixmob::rxSend(uint8_t mode, uint8_t red, uint8_t green, uint8_t blue, uint8_t group){
    uint8_t message[7];
    message[0] = mode;
    message[1] = green >> 2;
    message[2] = red   >> 2;
    message[3] = blue  >> 2;
    message[4] = ((globalAttack) << 3) + (globalRandom);
    message[5] = ((globalRelease) << 3) + (globalHold);
    message[6] = group & 0x1f;
    generateTXbuffer(message);
    refresh();
}

void Pixmob::sendColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t group){
  uint8_t mode = MODE_RX;
  rxSend( mode, red, green, blue, group);
}

void Pixmob::sendColor(uint8_t red, uint8_t green, uint8_t blue){
  uint8_t mode = MODE_RX;
  rxSend( mode, red, green, blue, 0);
}

void Pixmob::sendColorOnce(uint8_t red, uint8_t green, uint8_t blue, uint8_t group){
  uint8_t mode = MODE_RX | (1 << MODE_ONESHOT);
  rxSend( mode, red, green, blue, group);
}

void Pixmob::sendColorOnce(uint8_t red, uint8_t green, uint8_t blue){
  uint8_t mode = MODE_RX | (1 << MODE_ONESHOT);
  rxSend( mode, red, green, blue, 0);
}

void Pixmob::sendColorForever(uint8_t red, uint8_t green, uint8_t blue, uint8_t group){
  uint8_t mode = MODE_RX | (1 << MODE_ONESHOT)| (1 << MODE_FOREVER);
  rxSend( mode, red, green, blue, group);
}

void Pixmob::sendColorForever(uint8_t red, uint8_t green, uint8_t blue){
  uint8_t mode = MODE_RX | (1 << MODE_ONESHOT)| (1 << MODE_FOREVER);
  rxSend( mode, red, green, blue, 0);
}

void Pixmob::batchWrite(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t submode, uint8_t group){
    uint8_t message[7];
    message[0] = MODE_WRITE;
    message[1] = b1 & 0x3f;
    message[2] = b2 & 0x3f;
    message[3] = b3 & 0x3f;
    message[4] = b4 & 0x3f;
    message[5] = submode & 0x3f;
    message[6] = group & 0x1f;
    generateTXbuffer(message);
    refresh();
}

void Pixmob::setBackground(uint8_t red, uint8_t green, uint8_t blue){
    batchWrite( green >> 2, red >> 2, blue >> 2, SET_BG, SUBMODE_EEWRITE, 0);
}

void Pixmob::setBackground(uint8_t red, uint8_t green, uint8_t blue, uint8_t group){
    batchWrite( green >> 2, red >> 2, blue >> 2, SET_BG, SUBMODE_EEWRITE, group);
}

void Pixmob::setBackgroundSilent(uint8_t red, uint8_t green, uint8_t blue){
    batchWrite( green >> 2, red >> 2, blue >> 2, SET_BG_SILENT, SUBMODE_EEWRITE, 0);
}

void Pixmob::setBackgroundSilent(uint8_t red, uint8_t green, uint8_t blue, uint8_t group){
    batchWrite( green >> 2, red >> 2, blue >> 2, SET_BG_SILENT, SUBMODE_EEWRITE, group);
}

void Pixmob::storeColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t mem){
    uint8_t location = STORE_COLOR | (mem & 0xf);
    batchWrite( green >> 2, red >> 2, blue >> 2, location, SUBMODE_EEWRITE, 0);
}

void Pixmob::storeColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t mem, uint8_t group){
    uint8_t location = STORE_COLOR | (mem & 0xf);
    batchWrite( green >> 2, red >> 2, blue >> 2, location, SUBMODE_EEWRITE, group);
}

void Pixmob::storeColorSilent(uint8_t red, uint8_t green, uint8_t blue, uint8_t mem){
    uint8_t location = STORE_COLOR_SILENT | (mem & 0xf);
    batchWrite( green >> 2, red >> 2, blue >> 2, location, SUBMODE_EEWRITE, 0);
}

void Pixmob::storeColorSilent(uint8_t red, uint8_t green, uint8_t blue, uint8_t mem, uint8_t group){
    uint8_t location = STORE_COLOR_SILENT | (mem & 0xf);
    batchWrite( green >> 2, red >> 2, blue >> 2, location, SUBMODE_EEWRITE, group);
}

void Pixmob::storeGroup(uint8_t location, uint8_t value, uint8_t group){
    uint8_t b1 = (confirmGreen >> 4) | (((confirmRed >> 4) & 0x3) << 4);
    uint8_t b2 = ((confirmBlue >> 4) << 2) | (confirmRed >> 6);
    batchWrite( b1, b2, (location & 0x7), (value & 0x1f), SUBMODE_GR, group);
}

void Pixmob::setMasterGroup(uint8_t masterGroup, uint8_t group){
    uint8_t b1 = (confirmGreen >> 4) | (((confirmRed >> 4) & 0x3) << 4);
    uint8_t b2 = ((confirmBlue >> 4) << 2) | (confirmRed >> 6);
    batchWrite( b1, b2, (masterGroup & 0x7), 0, SUBMODE_MGR, group);
}

void Pixmob::playSend(uint8_t mode, uint8_t from, uint8_t to, uint8_t mRandom, uint8_t group){
    uint8_t message[7];
    message[0] = mode;
    message[1] = (from & 0xf) | ((to & 0x3) << 4);
    message[2] = (globalAttack << 3) | (mRandom & 0x4) | ((to & 0xc) >> 2);
    message[3] = ((globalRelease) << 3) + (globalHold);
    message[4] = (globalRandom);
    message[5] = 0;
    message[6] = (group & 0x1f) | ((mRandom & 0x1) << 5);
    generateTXbuffer(message);
    refresh();
}

void Pixmob::playMem(uint8_t from, uint8_t to, uint8_t mRandom){
  uint8_t mode = (1 << MODE_MEM);
  playSend( mode, from, to, mRandom, 0);
}

void Pixmob::playMem(uint8_t from, uint8_t to, uint8_t mRandom, uint8_t group){
  uint8_t mode = (1 << MODE_MEM);
  playSend( mode, from, to, mRandom, group);
}

void Pixmob::playMemOnce(uint8_t from, uint8_t to, uint8_t mRandom){
  uint8_t mode = (1 << MODE_MEM) | (1 << MODE_ONESHOT);
  playSend( mode, from, to, mRandom, 0);
}

void Pixmob::playMemOnce(uint8_t from, uint8_t to, uint8_t mRandom, uint8_t group){
  uint8_t mode = (1 << MODE_MEM) | (1 << MODE_ONESHOT);
  playSend( mode, from, to, mRandom, group);
}

void Pixmob::playMemForever(uint8_t from, uint8_t to, uint8_t mRandom){
  uint8_t mode = (1 << MODE_MEM) | (1 << MODE_ONESHOT) | (1 << MODE_FOREVER);
  playSend( mode, from, to, mRandom, 0);
}

void Pixmob::playMemForever(uint8_t from, uint8_t to, uint8_t mRandom, uint8_t group){
  uint8_t mode = (1 << MODE_MEM) | (1 << MODE_ONESHOT) | (1 << MODE_FOREVER);
  playSend( mode, from, to, mRandom, group);
}

void Pixmob::dualSend(uint8_t mode, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2){
    uint8_t message[7];
    message[0] = mode;
    message[1] = g1 >> 2;
    message[2] = r1 >> 2;
    message[3] = b1 >> 2;
    message[4] = g2 >> 2;
    message[5] = r2 >> 2;
    message[6] = b2 >> 2;
    generateTXbuffer(message);
    refresh();
}

void Pixmob::flashDual(uint8_t red1, uint8_t green1, uint8_t blue1, uint8_t red2, uint8_t green2, uint8_t blue2){
    uint8_t mode = MODE_RX | (1 << MODE_DUAL);
    dualSend(mode, red1, green1, blue1, red2, green2, blue2);
}

void Pixmob::flashDualLong(uint8_t red1, uint8_t green1, uint8_t blue1, uint8_t red2, uint8_t green2, uint8_t blue2){
    uint8_t mode = MODE_RX | (1 << MODE_DUAL) | (1 << MODE_ONESHOT);
    dualSend(mode, red1, green1, blue1, red2, green2, blue2);
}
