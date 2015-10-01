/* 
 This file is part of eLeReS (by Michal Maciakowski - <http://cyberdrones.com> ). 
 Adapted for MultiWii by Mis (Romuald Bialy)
 */
#ifdef ELERES_RX

#define BIN_OFF_VALUE	1150
#define BIN_ON_VALUE	1850

static uint8_t rcChannel[RC_CHANS] = {CHANNELS_ORDER};
static uint32_t last_pack_time, next_pack_time;
static uint32_t localizer_time = 1000L*LOCALIZER_DELAY;
static uint8_t DataReady, ch_hopping_time, first_run = 1, loc_force=0;
static uint8_t hop_list[16];
static uint16_t good_frames;
static uint8_t finder_time = 0;
static volatile uint8_t RF_Mode;
static uint8_t RF_Tx_Buffer[10]; 
static uint8_t RF_Rx_Buffer[22]; 
static uint16_t rcData4Values[RC_CHANS][2];
uint8_t QUALITY;
#if defined(ELERES_BIND)
  #define RF_Header  global_conf.RF_Header
#else
  static uint8_t RF_Header[4];
#endif

#if defined(TELEMETRY_ENABLE) && defined(LOCALIZER_ENABLE)
  uint8_t BkgLoc_enable = 0;
  uint8_t BkgLoc_chlist;  
  uint8_t BkgLoc_buf[3][10];
  uint8_t BkgLoc_cnt;
#else
  #define BkgLoc_enable 0
#endif

#if defined(PROMINI)
  // For 328p Boards ( UNTESTED! )
  #define  nSEL_on   PORTD |= 0x10          // pin12  ROLL
  #define  nSEL_off  PORTD &= ~0x10
  #define  SCK_on    PORTD |= 0x20          // pin13  PITCH
  #define  SCK_off   PORTD &= ~0x20
  #define  SDI_on    PORTD |= 0x40          // pin14  YAW
  #define  SDI_off   PORTD &= ~0x40
  #define  SDO_1    (PIND & 0x80) == 0x80   // pin15  AUX1
  #define  SDO_0    (PIND & 0x80) == 0x00
  #define  IRQ_0    (PIND & 0x04) == 0      // pin10  THROTTLE
  #define  IRQ_PIN_MASK0  0b00001111;
  #define  IRQ_PIN_MASK1  0b00000100;
  #define  RED_LED_ON   LEDPIN_ON
  #define  GREEN_LED_ON LEDPIN_ON
#endif

#if defined(MEGA)
  // For MEGA Boards, IRQ at A8
  #define  nSEL_on   PORTK |= 0x02          // A9   ROLL
  #define  nSEL_off  PORTK &= ~0x02
  #define  SCK_on    PORTK |= 0x04          // A10  PITCH
  #define  SCK_off   PORTK &= ~0x04
  #define  SDI_on    PORTK |= 0x08          // A11  YAW
  #define  SDI_off   PORTK &= ~0x08
  #define  SDO_1    (PINK & 0x10) == 0x10   // A12  AUX1
  #define  SDO_0    (PINK & 0x10) == 0x00
  #define  IRQ_0    (PINK & 0x01) == 0      // A8   THROTTLE
  #define  IRQ_PIN_MASK0  0b11100001;
  #define  IRQ_PIN_MASK1  0b00000001;
  #define  RED_LED_ON   PORTB |= (1<<7)        // Czerwony led na AIO
  #define  GREEN_LED_ON PORTC |= (1<<7);       // Zielony led na AIO
#endif

//***************************************************************************** 

void configureReceiver() {
  uint8_t i;
 #if defined(PROMINI)
  PORTD &= ~0b00011111; 
  PORTD |=  0b00010100;   // nSEL = 1, IRQ pullup 
  DDRD  &= ~0b01111011; 
  DDRD  |=  0b01110000; 
 #endif
 #if defined(MEGA)
  PORTK &= ~0b11100011; 
  PORTK |=  0b00000011;   // nSEL = 1, IRQ pullup
  DDRK  &= ~0b11100000; 
  DDRK  |=  0b00001110; 
 #endif
  PCINT_RX_MASK &= IRQ_PIN_MASK0; 
  PCINT_RX_MASK |= IRQ_PIN_MASK1; 
  PCICR |= PCIR_PORT_BIT;
  #if !defined(ELERES_BIND)
    RF_Header[0] = (BIND_SIGNATURE >> 24) & 0xFF;
    RF_Header[1] = (BIND_SIGNATURE >> 16) & 0xFF;
    RF_Header[2] = (BIND_SIGNATURE >> 8 ) & 0xFF;
    RF_Header[3] = (BIND_SIGNATURE >> 0 ) & 0xFF;
  #endif
  RF22B_init_parameter();
  Bind_Channels(RF_Header,hop_list);
  ch_hopping_time = 33;
  to_rx_mode();
  ChannelHopping(1);
  RF_Mode = 4;
  for(i=0; i<12; i++){ 
    rcData[i] = MIDRC;
    rcData4Values[i][0] = MIDRC;
    rcData4Values[i][1] = MIDRC;
  }
}

uint16_t readRawRC(uint8_t chan) {
  if (chan < RC_CHANS) return rcData[chan];
  else return MIDRC;
}

void eLeReS_control() {
  static uint8_t rc4ValuesIndex = 0;
  static uint32_t qtest_time, guard_time;
  static uint8_t rx_frames,loc_cnt;
  uint8_t i,n;
  uint16_t temp_int;
  uint32_t cr_time = millis();
  if((DataReady & 2) == 0) {
    if(cr_time > next_pack_time+2) {
      if ((cr_time-last_pack_time > 1500) || first_run) {
        RED_LED_ON;
        analog.rssi = 0x1012;
        RF22B_init_parameter();
        to_rx_mode();
        ChannelHopping(15);
        next_pack_time += 17L*ch_hopping_time;
      #ifdef TELEMETRY_ENABLE
        telemetry_RX();
        to_tx_mode(9);
      #endif
      } 
      else {
        if(cr_time-last_pack_time > 3*ch_hopping_time) {
          RED_LED_ON;
        }
        to_rx_mode();
        ChannelHopping(1);
        next_pack_time += ch_hopping_time;
      }
    }
    if(cr_time > qtest_time) {
      qtest_time = cr_time + 500;
      QUALITY = good_frames * 100 / (500/ch_hopping_time);
      if(QUALITY > 100) QUALITY = 100;
      good_frames = 0;
    }
  }
  if((DataReady & 3) == 1) {
    if((DataReady & 4)==0) {
      rc4ValuesIndex = !rc4ValuesIndex;
      cli();
      uint8_t channel_count = RF_Rx_Buffer[20] >> 4;
      if(channel_count < 4)  channel_count = 4;
      if(channel_count > 12) channel_count = 12;
      for(i = 0; i<channel_count; i++) {                                                          
        temp_int = RF_Rx_Buffer[i+1];
        if(i%2 == 0)
          temp_int |= ((uint16_t)RF_Rx_Buffer[i/2 + 13] << 4) & 0x0F00;
        else								 
          temp_int |= ((uint16_t)RF_Rx_Buffer[i/2 + 13] << 8) & 0x0F00;
        if ((temp_int>799) && (temp_int<2201)) rcData4Values[i][rc4ValuesIndex] = temp_int;
      }
      n = RF_Rx_Buffer[19];
      sei();
      for(i=channel_count; i < channel_count+5; i++) {
        if(i > 11) break;
        if(n & 0x01) temp_int = BIN_ON_VALUE; 
        else temp_int = BIN_OFF_VALUE;
        rcData4Values[i][rc4ValuesIndex] = temp_int; 
        n >>= 1;
      }
      for(; i<12; i++) rcData4Values[i][rc4ValuesIndex]=MIDRC;
  #ifdef FAILSAFE
    if(!rcOptions[BOXFAILSAFE]) failsafeCnt >>= 1;
  #endif
    localizer_time = cr_time + (1000L*LOCALIZER_DELAY);
   #if defined(TELEMETRY_ENABLE) && defined(LOCALIZER_ENABLE)
    if(BkgLoc_enable == 0) BkgLoc_cnt=0;
    if(BkgLoc_cnt) BkgLoc_cnt--;
    if(BkgLoc_cnt<128)
      telemetry_RX();
    else
      memcpy(RF_Tx_Buffer, BkgLoc_buf[BkgLoc_cnt%3], 9);  
   #else
     #if defined(TELEMETRY_ENABLE)
       telemetry_RX();
     #endif
   #endif  
    }
    DataReady &= 0xFC;
  }
 #ifdef LOCALIZER_ENABLE
  if((DataReady & 3)==3 && RF_Rx_Buffer[19]<128) {
    if(rx_frames == 0)	guard_time = last_pack_time;
    if(rx_frames < 250)	{rx_frames++; GREEN_LED_ON;}
    if(rx_frames > 20 && cr_time-guard_time > (loc_force?5000:20000)) {
      DataReady = 0;
      localizer_time = cr_time + (1000L*LOCALIZER_DELAY);
      RF22B_init_parameter();
      ChannelHopping(1);
      rx_frames = 0;
      #ifdef TELEMETRY_ENABLE
      if(loc_force) {
        BkgLoc_enable = 1;
        finder_time = 64;
        temp_int = 0;
        for(i=0;i<16;i++) {
          uint16_t mult = hop_list[i] * hop_list[(i+1)%16];
          if(mult > temp_int) {temp_int = mult; BkgLoc_chlist = i;}
        }
      }
      #endif  
    }
  }
  if(cr_time-last_pack_time > 8000) {
    rx_frames = 0;
  }
  if(!f.ARMED && cr_time>localizer_time) {
    if((DataReady & 2)==0) {
      RF22B_init_parameter();
     #if defined(LOCALIZER_POWER)
       _spi_write(0x6d, LOCALIZER_POWER | 0x18);
     #endif
    }
    DataReady &= 0xFB;
    DataReady |= 2;
    localizer_time = cr_time+35;
    _spi_write(0x79, 0);
    RED_LED_ON;
    #ifdef TELEMETRY_ENABLE
    BkgLoc_enable = 0;
    if(!(++loc_cnt & 1)) {
        telemetry_RX();
        to_tx_mode(9);
    } else
    #endif
    {
      RF_Tx_Buffer[0] = 0x48; 
      RF_Tx_Buffer[1] = 0x45; 
      RF_Tx_Buffer[2] = 0x4c; 
      RF_Tx_Buffer[3] = 0x50; 
      RF_Tx_Buffer[4] = 0x21; 
      to_tx_mode(5);
    }
  }
  if((f.ARMED || first_run) && (DataReady & 2)==0)
    localizer_time = cr_time + (1000L*LOCALIZER_DELAY);
    
  #ifdef TELEMETRY_ENABLE 
    if(DataReady & 4) {
      if(RF_Rx_Buffer[0]=='H') BkgLoc_buf[0][0]='T';
      if(RF_Rx_Buffer[0]=='T') {BkgLoc_buf[0][0]='T'; memcpy(BkgLoc_buf[0]+2, RF_Rx_Buffer+2, 7); }
      if(RF_Rx_Buffer[0]=='P') memcpy(BkgLoc_buf[1], RF_Rx_Buffer, 9);    
      if(RF_Rx_Buffer[0]=='G') memcpy(BkgLoc_buf[2], RF_Rx_Buffer, 9);
      DataReady = 0;
    }
    if(!f.ARMED && BkgLoc_enable == 2) GREEN_LED_ON;
  #endif  
 #endif  
}

void computeRC(void) {
  uint8_t i;
  uint16_t temp_int;
  for(i=0; i<12; i++) {
    temp_int = (rcData4Values[rcChannel[i]][0] + rcData4Values[rcChannel[i]][1] + 1)/2;
    if (temp_int < rcData[i] -3)  rcData[i] = temp_int+2;
    if (temp_int > rcData[i] +3)  rcData[i] = temp_int-2;
  }
}

#if defined(ELERES_BIND)
void eLeReS_Bind() {
  static uint8_t RF_Header_old[4];
  static uint8_t RF_Header_OK_count = 0;
  uint8_t i;
  RF_Header[0] = 0x42;
  RF_Header[1] = 0x49;
  RF_Header[2] = 0x4e;
  RF_Header[3] = 0x44;
  RF22B_init_parameter();
  Bind_Channels(RF_Header,hop_list);
  ch_hopping_time = 33;
  STABLEPIN_OFF;
  while(1) {
    eLeReS_control();
    if (RF_Rx_Buffer[0]==0x42)
    {	
      for(i=0; i<4; i++) {
        if (RF_Rx_Buffer[i+1]==RF_Header_old[i]) 
          RF_Header_OK_count++; 
        else 
          RF_Header_OK_count = 0;
      }
      for(i=0; i<4; i++) RF_Header_old[i] = RF_Rx_Buffer[i+1];
      if (RF_Header_OK_count>200) {
        for(i=0; i<4; i++) RF_Header[i] = RF_Header_old[i];
        writeGlobalSet(1);
        LEDPIN_ON;
        STABLEPIN_ON;
        while(1);
      }
      LEDPIN_TOGGLE;
      RF_Rx_Buffer[0] = 0;
    }
  }
}
#endif

//***************************************************************************** 

uint8_t CheckChannel(uint8_t channel, uint8_t *hop_lst) {
  uint8_t new_channel, count = 0, high = 0, i;
  for (i=0; i<16; i++) {
    if (high<hop_lst[i]) high = hop_lst[i];
    if (channel==hop_lst[i]) count++;	
  }
  if (count>0) new_channel = high+2;
  else new_channel = channel;
  return new_channel%255;
}

void Bind_Channels(uint8_t* RF_HEAD, uint8_t* hop_lst) {
  uint8_t n,i,j;
  for (i=0;i<16;i++) hop_lst[i] = 0;	
  for (j=0;j<4;j++) {
    for (i=0;i<4;i++) {
      n = RF_HEAD[i]%128;
      if(j==3) n /= 5; else n /= j+1;
      hop_lst[4*j+i] = CheckChannel(n,hop_lst);
    }
  }
}

//***************************************************************************** 

ISR(PCINT2_vect) { 
  static uint16_t rssifil;
  if(IRQ_0) {
    uint32_t irq_time = millis();
    sei();
    uint8_t St1 = _spi_read(0x03);
    uint8_t St2 = _spi_read(0x04); 
    if((RF_Mode & 4) && (St1 & 0x02))
      RF_Mode |= 8;
    if((RF_Mode & 1) && (St1 & 0x04))
      RF_Mode |= 2;
    if((RF_Mode & 4) && (St2 & 0x80))
      RF_Mode |= 16;

    if(RF_Mode & 8) {
      if(BkgLoc_enable < 2) 
      {
        last_pack_time = irq_time;
        next_pack_time = irq_time + ch_hopping_time;
      }
      Write8bitcommand(0x7f);
      for(uint8_t i = 0; i<22; i++)
        RF_Rx_Buffer[i] = read_8bit_data(); 
      rx_reset();
      RF_Mode = 4;
      if (RF_Rx_Buffer[0] == 0x53) {
        first_run = 0;
        good_frames++;
        ch_hopping_time = (RF_Rx_Buffer[20] & 0x0F)+18;
        DataReady |= 1;
      }
     #ifdef LOCALIZER_ENABLE
       #ifdef TELEMETRY_ENABLE
          else if(BkgLoc_enable==2) {
            if((RF_Rx_Buffer[0] == 'H' && RF_Rx_Buffer[2] == 'L') ||
                RF_Rx_Buffer[0]=='T' || RF_Rx_Buffer[0]=='P' || RF_Rx_Buffer[0]=='G') {
              if(BkgLoc_cnt==0) BkgLoc_cnt = 200;
              to_ready_mode();
              BkgLoc_enable = 0;
              ChannelHopping(0);
              BkgLoc_enable = 2;
              DataReady |= 4;
            }
          }
       #endif // telemetry enable   
      else if(RF_Rx_Buffer[0] == 0x4c && RF_Rx_Buffer[1] == 0x4f && RF_Rx_Buffer[2] == 0x43) {
        localizer_time = irq_time;
        loc_force = 1;
        analog.rssi = 0x1012;
      }
     #endif
      if((DataReady & 2)==0) {
       #ifdef TELEMETRY_ENABLE
        to_tx_mode(9);
       #else
        ChannelHopping(1);
       #endif
      }
    }  
    if(RF_Mode & 2) {
      to_ready_mode();
      if(DataReady & 2) {
        _spi_write(0x79, hop_list[0]);
      } 
      else if(irq_time-last_pack_time <= 1500 && BkgLoc_enable<2)
        ChannelHopping(1);
      to_rx_mode();
    }
    if(RF_Mode & 16) {
      uint8_t rssitmp = _spi_read(0x26);
     #if defined(TELEMETRY_ENABLE) && defined(LOCALIZER_ENABLE)
      if(BkgLoc_enable==2) {
        if(rssitmp>124)	rssitmp = 124;
        if(rssitmp<18)	rssitmp = 18;
        BkgLoc_buf[0][1] = rssitmp + 128;
      } else 
     #endif 
      {
        rssifil -= (rssifil>>3);
        rssifil += rssitmp * QUALITY / 100;
        analog.rssi = (rssifil>>3)+10;
        if(analog.rssi>124)  analog.rssi = 124;
        if(analog.rssi<18)   analog.rssi = 18;
        analog.rssi += 0x1000;
      }
      RF_Mode &= ~16;
    }
  }  
}

//-------------------------------------------------------------- 
void send_8bit_data(uint8_t i) { 
  uint8_t n = 8; 
  while(n--) { 
    SCK_off;
    if(i&0x80) SDI_on; else SDI_off;
    i <<= 1; 
    SCK_on; 
  } 
  SCK_off;
}
//-------------------------------------------------------------- 
uint8_t read_8bit_data(void) { 
  uint8_t Result, i; 
  SCK_off;
  Result=0; 
  for(i=0;i<8;i++) {
    Result <<= 1; 
    SCK_on;
    asm volatile("NOP"); 
    if(SDO_1) Result|=1; 
    SCK_off;
  } 
  return(Result); 
}  
//-------------------------------------------------------------- 
void Write8bitcommand(uint8_t command) {
  nSEL_on;
  SCK_off;
  asm volatile("NOP"); 
  nSEL_off; 
  send_8bit_data(command);
}  
//-------------------------------------------------------------- 
uint8_t _spi_read(uint8_t address) { 
  uint8_t result; 
  address &= 0x7f; 
  Write8bitcommand(address); 
  result = read_8bit_data();  
  nSEL_on; 
  return(result); 
}  
//-------------------------------------------------------------- 
void _spi_write(uint8_t address, uint8_t data) { 
  address |= 0x80; 
  Write8bitcommand(address); 
  send_8bit_data(data);  
  nSEL_on;
}  

//------------------------------------------------------------------- 
void RF22B_init_parameter(void) { 
  uint8_t i;
  static uint8_t cf1[9] = {0x00,0x01,0x00,0x7f,0x07,0x52,0x55,0x07,0x00};
  static uint8_t cf2[6] = {0x68,0x01,0x3a,0x93,0x02,0x6b};
  static uint8_t cf3[8] = {0x0f,0x42,0x07,0x20,0x2d,0xd4,0x00,0x00};
  _spi_read(0x03);         
  _spi_read(0x04); 
  for(i=0; i<9; i++) _spi_write(0x06+i, cf1[i]);
  _spi_write(0x6e, 0x09);  
  _spi_write(0x6f, 0xD5);
  _spi_write(0x1c, 0x02);  
  _spi_write(0x70, 0x00);
  for(i=0; i<6; i++) _spi_write(0x20+i, cf2[i]);
  _spi_write(0x2a, 0x1e);  
  _spi_write(0x72, 0x1F);  
  _spi_write(0x30, 0x8c);  
  _spi_write(0x3e, 22);
  for(i=0; i<8; i++) _spi_write(0x32+i, cf3[i]);
  for(i=0; i<4; i++) _spi_write(0x43+i, 0xff);  
  _spi_write(0x6d, TELEMETRY_POWER | 0x18);
  _spi_write(0x79, 0x00);  
  _spi_write(0x7a, 0x04);
  _spi_write(0x71, 0x23); 
  _spi_write(0x73, 0x00);   
  _spi_write(0x74, 0x00);
  for(i=0; i<4; i++) {
    _spi_write(0x3a+i, RF_Header[i]);
    _spi_write(0x3f+i, RF_Header[i]);
  }
  frequency_configurator(BASE_FREQ);
  _spi_read(0x03);         
  _spi_read(0x04); 
}

//----------------------------------------------------------------------- 
void rx_reset(void) { 
  _spi_write(0x07, 0x01); 
  _spi_write(0x08, 0x03);
  _spi_write(0x08, 0x00);
  _spi_write(0x07, 0x05);
  _spi_write(0x05, 0x02);
  _spi_write(0x06, 0x80);
  _spi_read(0x03);
  _spi_read(0x04);  
}  
//-----------------------------------------------------------------------    
void to_rx_mode(void) {  
  to_ready_mode(); 
  rx_reset();
  RF_Mode = 4; 
}  
//-------------------------------------------------------------- 
void to_tx_mode(uint8_t bytes_to_send) { 
  uint8_t i;
  to_ready_mode();
  _spi_write(0x08, 0x03);
  _spi_write(0x08, 0x00);
  _spi_write(0x3e, bytes_to_send);
  for (i = 0; i<bytes_to_send; i++)
    _spi_write(0x7f, RF_Tx_Buffer[i]); 
  _spi_write(0x05, 0x04);  
  _spi_write(0x06, 0);
  _spi_read(0x03);
  _spi_read(0x04); 
  _spi_write(0x07, 0x09);
  RF_Mode = 1; 
}  
//-------------------------------------------------------------- 
void to_ready_mode(void) { 
  _spi_write(0x07, 1); 
  _spi_write(0x05, 0);
  _spi_write(0x06, 0);
  _spi_read(0x03);   
  _spi_read(0x04); 
  RF_Mode = 0; 
}  
//--------------------------------------------------------------   

void frequency_configurator(uint32_t frequency) {
  // frequency formulation from Si4432 chip's datasheet
  frequency /= 10L;
  uint8_t band;
  if(frequency<48000) {
    frequency -= 24000;
    band = frequency/1000;
    if(band>23) band = 23;
    frequency -= (1000*(uint32_t)band);
    frequency *= 64;
    band |= 0x40;
  } else {
    frequency -= 48000;
    band = frequency/2000;
    if(band>22) band = 22;
    frequency -= (2000*(uint32_t)band);
    frequency *= 32;
    band |= 0x60;
  }
  _spi_write(0x75, band);
  _spi_write(0x76, (uint16_t)frequency >> 8);
  _spi_write(0x77, (uint8_t)frequency);
  _spi_write(0x79, 0);
}
//-------------------------------------------------------------- 
void ChannelHopping(uint8_t hops) {
  static uint8_t hopping_channel = 1;
  hopping_channel += hops;
  while(hopping_channel >= 16) hopping_channel -= 16;
#if defined(TELEMETRY_ENABLE) && defined(LOCALIZER_ENABLE)
  if(BkgLoc_enable && (hopping_channel==BkgLoc_chlist || hopping_channel==(BkgLoc_chlist+1)%16)) {
    _spi_write(0x79,0);
    BkgLoc_enable = 2;
    return;
  }
  if(BkgLoc_enable == 2) BkgLoc_enable = 1;
#endif
  _spi_write(0x79, hop_list[hopping_channel]);
}
//***************************************************************************** 
 #ifdef TELEMETRY_ENABLE
  void telemetry_RX(void) {
    static uint8_t telem_state;
    static uint32_t presfil;
    static int16_t thempfil;
    uint8_t i, themp=90, wii_flymode=0;
    uint16_t pres;
    union {int32_t val; uint8_t b[4];} cnv;
    memset(RF_Tx_Buffer,0,9);
    if(finder_time) {
      if(--finder_time==0) {
        RF_Tx_Buffer[0] = 'W';
	RF_Tx_Buffer[1] = 17|0x80;		// CLS + Beep + center X
	RF_Tx_Buffer[2] = 18|0x40;		// big font + center Y
	memcpy_P(RF_Tx_Buffer+3, PSTR("Finder"), 6);
        return;
      }
    }
  #ifdef TELEMETRY_SEND_DEBUG
    if(++telem_state > 3) telem_state = 0;
  #else
    if(++telem_state > 2) telem_state = 0;
  #endif
  #if BARO
    presfil  -= presfil/4; 
    presfil  += baroPressure;
    thempfil -= thempfil/8;
    thempfil += baroTemperature/10;
  #endif 
    if(telem_state == 0) {
     #if BARO
      if(presfil>200000) pres = presfil/4 - 50000; else pres = 1;
      themp = (uint8_t)(thempfil/80 + 86);
     #endif 
      if     (f.FAILSAFE_ON)    wii_flymode = 7;
      else if(f.PASSTHRU_MODE)  wii_flymode = 8;
      else if(f.GPS_HOME_MODE)  wii_flymode = 6;
      else if(f.GPS_HOLD_MODE)  wii_flymode = 5;
      else if(f.HEADFREE_MODE)  wii_flymode = 4;
      else if(f.BARO_MODE)      wii_flymode = 3;
      else if(f.ANGLE_MODE || f.HORIZON_MODE)  wii_flymode = 2;
      else                      wii_flymode = 1;
      if(f.ARMED)               wii_flymode |= 0x10;
      if(f.GPS_FIX_HOME && !calibratingG) wii_flymode |= 0x20;
      RF_Tx_Buffer[0] = 0x54;
      RF_Tx_Buffer[1] = analog.rssi & 0x7F;
      RF_Tx_Buffer[2] = QUALITY;
      if(BkgLoc_enable) RF_Tx_Buffer[2] |= 0x80;
      RF_Tx_Buffer[3] = analog.vbat;
      RF_Tx_Buffer[4] = themp;
      RF_Tx_Buffer[5] = analog.amperage;
      RF_Tx_Buffer[6] = pres>>8;
      RF_Tx_Buffer[7] = pres & 0xFF;
      RF_Tx_Buffer[8] = wii_flymode;
      RF_Tx_Buffer[8] = wii_flymode | ((analog.amperage>>2) & 0xC0);
    } 
  #if GPS
    else if(telem_state == 1 && GPS_Present) {
      RF_Tx_Buffer[0] = 0x50;
      cnv.val = GPS_coord[LAT]/10;
      for(i=0;i<4;i++) RF_Tx_Buffer[i+1] = cnv.b[i];
      cnv.val = GPS_coord[LON]/10;
      for(i=0;i<4;i++) RF_Tx_Buffer[i+5] = cnv.b[i];
      if(GPS_hdop > 127) GPS_hdop = 127; 
      RF_Tx_Buffer[4] &= 0x0F;                            // cut off unused high part of latitude
      RF_Tx_Buffer[4] |= (GPS_hdop << 4);                 // add lower nibble of HDOP 
      RF_Tx_Buffer[8] &= 0x1F;                            // cut off unused high part of longitude
      RF_Tx_Buffer[8] |= ((GPS_hdop<<1) & 0xE0);          // add higher 3 bits of HDOP
    } 
    else if(telem_state == 2 && GPS_Present) {
      uint16_t gpsspeed =  (GPS_speed*9L)/250L;
      int16_t course = (GPS_ground_course+360)%360;
      uint16_t tim = 10*hex_c(GPS_time[0]) + hex_c(GPS_time[1]);
      tim <<= 11;
      i = 10*hex_c(GPS_time[2]) + hex_c(GPS_time[3]);
      tim |= (uint16_t)i<<5;
      i = 10*hex_c(GPS_time[4]) + hex_c(GPS_time[5]);
      tim |= (i>>1);
      RF_Tx_Buffer[0] = 0x47;
      RF_Tx_Buffer[1] = (f.GPS_FIX<<4) | (GPS_numSat & 0x0F) | ((i&1)<<6);
      if(GPS_numSat > 15) RF_Tx_Buffer[1] |= 0x80;
      RF_Tx_Buffer[2] = ((course>>8) & 0x0F) | ((gpsspeed>>4) & 0xF0);
      RF_Tx_Buffer[3] = course & 0xFF;
      RF_Tx_Buffer[4] = gpsspeed & 0xFF;
      RF_Tx_Buffer[5] = GPS_altitude>>8;
      RF_Tx_Buffer[6] = GPS_altitude & 0xFF;
      RF_Tx_Buffer[7] = tim>>8;
      RF_Tx_Buffer[8] = tim&0xFF;
    } 
  #endif
  #ifdef TELEMETRY_SEND_DEBUG
    else if(telem_state == 3) {
      RF_Tx_Buffer[0] = 0x44;
      memcpy(RF_Tx_Buffer+1,debug,8);
    }
  #endif
  }
 #endif
#endif


