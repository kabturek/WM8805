// vim:ft=arduino
#ifndef WM8805_H
#define WM8805_H

#include "WProgram.h"

namespace arduino{
  namespace libraries{
    class WM8805 {
      public:
        //indicates whether were in mode 1 (192khz) or mode 2/3/4
        uint8_t current_mode;
        uint8_t selected_input;
        static const uint8_t DEVID1_BIT = 0x00;
        static const uint8_t DEVID2_BIT = 0x01;
        static const uint8_t PLL1_BIT = 0x03;
        static const uint8_t PLL2_BIT = 0x04;
        static const uint8_t PLL3_BIT = 0x05;
        static const uint8_t PLL4_BIT = 0x06;
        static const uint8_t PLL6_BIT = 0x08;
        static const uint8_t RXCHAN4_BIT = 0x10;
        static const uint8_t RXCHAN5_BIT = 0x11;
        static const uint8_t SPDSTAT_BIT = 0x0C;
        static const uint8_t PWRDN_BIT = 0x1E;
        // PWRDN bit 0x1E
        uint8_t pwrdn;
        // 0x03
        uint8_t pll1;
        uint8_t pll1_192;
        // 0x04
        uint8_t pll2;
        uint8_t pll2_192;
        // 0x05
        uint8_t pll3;
        uint8_t pll3_192;
        // 0x05
        uint8_t pll4;
        uint8_t pll4_192;
        //SPDSTAT 0x0c
        uint8_t spdstat;
        // PLL6 
        uint8_t pll6;
        WM8805(
            const uint8_t address = 0x3A, 
            const uint8_t x = 5,
            const bool prescale = 0,
            const uint8_t pll_n = 7,
            const uint8_t pll_n192 = 8,
            const uint8_t pll1 = B00100001,
            const uint8_t pll1_192 = B10111010,
            const uint8_t pll2 = B11111101,
            const uint8_t pll2_192 = B01001001,
            const uint8_t pll3 = B00110110,
            const uint8_t pll3_192 = B11000100
            );
        void init(uint8_t selected_input = 1);
        uint8_t read(uint8_t register_address);
        uint8_t write(uint8_t register_address, uint8_t data);
        bool is_connected();
        // PWRDN
        uint8_t power(
            bool pll = true, 
            bool spdif_reciver = true, 
            bool spdif_transmitter = true, 
            bool oscillator = true, 
            bool aif = true, 
            bool tri = true
            );
        //PLL6
        bool select_input(uint8_t input_number);
        //SPDTX4
        bool select_source(bool source);
        //TXSRC
        bool txsrc(bool source = 1);
        //set mode (1 - 192/176.4, 2,3,4 - 32/44/48/88/96)
        bool set_pll_mode(uint8_t mode = 2);
        bool is_192();
        bool is_audio();
        bool is_deemph();
        uint8_t frequency();
        uint8_t indicated_frequency();
        bool is_locked();
        //checks if the stream is 192 and sets pll, checks if its a non-audio
        //stream
        bool check_stream();
        bool stream_changed();

      private:
        const uint8_t _address;
        const uint8_t _x;
    };
  }
}

#endif
