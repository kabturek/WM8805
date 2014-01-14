// vim:ft=arduino
/*
  WM8805.cpp - Wolfson WM8805 library for Arduino
  Copyright (c) 2011 Marcin Domanski.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <Wire.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include "Arduino.h"
#include "WM8805.h"

namespace arduino{
  namespace libraries{
    WM8805::WM8805(
        const uint8_t address, 
        const bool prescale,
        const uint8_t pll_n,
        const uint8_t pll_n192,
        const uint8_t pll1 ,
        const uint8_t pll1_192 ,
        const uint8_t pll2 ,
        const uint8_t pll2_192 ,
        const uint8_t pll3 ,
        const uint8_t pll3_192 
        ) :
      _address(address),
      pll1(pll1),
      pll1_192(pll1_192),
      pll2(pll2),
      pll2_192(pll2_192),
      pll3(pll3),
      pll3_192(pll3_192)
    {
      this->pwrdn = B00000111;
      this->spdstat = B00000000;
      this->pll6 = B00011000;
      this->pll4_192 = pll_n192;
      this->pll4 = pll_n;
      if(prescale){
        this->pll4_192 |= (1 << 4);
        this->pll4 |= (1 << 4);
      } else {
        this->pll4_192 &= ~(1 << 4);
        this->pll4 &= ~(1 << 4);
      }
      this->current_mode = 2;
    }
    void WM8805::init( uint8_t selected_input){
      //configure the system for s/pdif receiver startup
      //1. Write appropriate calculated values (relative to oscillator frequency) to PRESCALE, PLL_N and PLL_K registers for 32/44.1/48/88.2/96kHz (modes 2/3/4) S/PDIF receiver sample rate operation.
      //2. Enable PLL by clearing PLLPD bit.
      //3. Enable S/PDIF receiver by clearing SPDIFRXPD bit.
      select_input(selected_input);
      power();
      //4. Read S/PDIF Status Register REC_FREQ[1:0] bits to identify recovered S/PDIF sample frequency and clocking mode.
      //5. If indicated sample rate is 192kHz, then the user must know what the sampling frequency is (176.4kHz or 192kHz) since these cannot be distinguished. The user should then write appropriate calculated values (relative to oscillator frequency) to PRESCALE, PLL_N and PLL_K for 176.4/192kHz (mode 1) S/PDIF receiver sample rate operation.
      check_stream();
    }
    uint8_t WM8805::read(uint8_t register_address){
      uint8_t value = 0;
      Wire.beginTransmission(_address);
      Wire.write((byte)register_address);
      // stop transmitting
      Wire.endTransmission();
      // Now do a transfer reading one byte from the LIS3L*
      // This data will be the contents of register 0x28
      Wire.requestFrom(_address, (uint8_t) 1);
      if(Wire.available()) // loop through all but the last      
      {
        value = Wire.read(); // print the character       
      }
      return value;
    }
    uint8_t WM8805::write(uint8_t register_address, uint8_t data){
      Wire.beginTransmission(_address);
      Wire.write((byte)register_address);
      Wire.write((byte)data);
      // stop transmitting
      uint8_t ret = Wire.endTransmission();
      return ret;
    }
    bool WM8805::check_stream(){
      //get spdif status
      uint8_t new_spdstat = read(WM8805::SPDSTAT_BIT);
      if(new_spdstat == this->spdstat){
        return false;
      }
      this->spdstat = new_spdstat;
      if(this->current_mode == 2 && is_192()){
        set_pll_mode(1);
      }
      if(this->current_mode == 1 && !is_192()){
        set_pll_mode(2);
      }
      return true;
    }
    bool WM8805::set_pll_mode(uint8_t mode){
      if(mode == 1){
        this->current_mode = 1;
        write(WM8805::PLL1_BIT, this->pll1_192);
        write(WM8805::PLL2_BIT, this->pll2_192);
        write(WM8805::PLL3_BIT, this->pll3_192);
        write(WM8805::PLL4_BIT, this->pll4_192);
      } else {
        this->current_mode = 2;
        write(WM8805::PLL1_BIT, this->pll1);
        write(WM8805::PLL2_BIT, this->pll2);
        write(WM8805::PLL3_BIT, this->pll3);
        write(WM8805::PLL4_BIT, this->pll4);
      }
      return true;
    }
    bool WM8805::is_192(){
      return (this->frequency() == 192);
    }
    uint8_t WM8805::power(bool pll, bool spdif_reciver, bool spdif_transmitter, bool oscillator, bool aif, bool tri){
      //OPTIMIZE: messy!
      if(tri){
        this->pwrdn &= ~(1 << 5);
      } else {
        this->pwrdn |= (1 << 5);
      }
      if(aif){
        this->pwrdn &= ~(1 << 4);
      } else {
        this->pwrdn |= (1 << 4);
      }
      if(oscillator){
        this->pwrdn &= ~(1 << 3);
      } else {
        this->pwrdn |= (1 << 3);
      }
      if(spdif_transmitter){
        this->pwrdn &= ~(1 << 2);
      } else {
        this->pwrdn |= (1 << 2);
      }
      if(spdif_reciver){
        this->pwrdn &= ~(1 << 1);
      } else {
        this->pwrdn |= (1 << 1);
      }
      if(pll){
        this->pwrdn &= ~1;
      } else {
        this->pwrdn |= 1;
      }
      return write(WM8805::PWRDN_BIT, this->pwrdn);
    }
    bool WM8805::is_connected(){
      // 0x88 is the second half of the device id
      return (read(WM8805::DEVID2_BIT) == 136 );
    }
    bool WM8805::select_input(uint8_t input_number){
      uint8_t writed =  write(WM8805::PLL6_BIT, this->pll6 + input_number);
      if(writed == 0){
        this->selected_input = input_number;
        return true;
      }
      return false;
    }
    bool WM8805::is_audio(){
      return !(this->spdstat & B00000001);
    }
    bool WM8805::is_deemph(){
      return !(this->spdstat & B00000100);
    }
    uint8_t WM8805::frequency(){
      if(!is_locked()){
        return 0;
      }
      //frequency is the 5:4 bits of pll6, we use the mask 48
      uint8_t freq_bits = (this->spdstat & 48) >> 4;
      if(freq_bits == 0){
        return 192;
      } else if( freq_bits == 1){
        return 96;
      } else if( freq_bits == 2){
        return 44;
      }
      return 32;
    }
    bool WM8805::is_locked(){
      return !(this->spdstat & B01000000);
    }
    uint8_t WM8805::indicated_frequency(){
      if(!is_locked()){
        return 0;
      }
      uint8_t rxchan4 = read(RXCHAN4_BIT);
      //get the last 4 bytes that indicate the original sampling frequency
      rxchan4 &= B00001111;
      switch(rxchan4){
        case 0:
          return 44;
        case 1:
          return 0;
        case 2:
          return 48;
        case 3:
          return 32;
        case 4:
          return 22;
        case 6:
          return 24;
        case 8:
          return 88;
        case 10:
          return 96;
        case 12:
          return 176;
        case 14:
          return 192;
        default:
          return 0;
      }
    }
  }
}
