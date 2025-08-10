#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MPU_ADDR 0x68
#define BAUD 9600

uint8_t i2c_wait();
uint8_t i2c_start();
uint8_t i2c_write(uint8_t);

//UART

//initialize
void uart_init(){
  UBRR0H = 0; // each usart has its own pair (3 pairs in atmega328p)
  UBRR0L = 103; // 9600 baud for 16MHz (full)
  UCSR0B = /*(1<<RXEN0)|*/(1<<TXEN0); //enable rx and tx in ucsrnB reg
  UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); //set frame format as 8 bits(1 byte) with 1 bit stop bit
}

void uart_tx(char c){
  // udrre01 is set to 1, which means its empty
  while(!((UCSR0A)&(1<<UDRE0))) {
    ; // wait until buffer empty
  }//wait till bit is set to 1
  UDR0 = c; //
  
}

void uart_print(const char* s){ //string is literal memeory and is stored in flash mem
  while(*s){
    uart_tx(*s++);
  }
}

//I2C

void i2c_init(){
  TWSR = 0x00; //prescaler =1 i.e. 100KHZ; SCLf = F_CPU/(16+2*TWBR*Prescaler)
  TWBR = 72; //for 100khz
  TWCR = (1<<TWEN); // activate TWI 
}

uint8_t i2c_start(){
  TWCR = (1<<TWSTA)|(1<<TWEN)|(1<<TWINT);// twsta -> start condn; twint -> clear interrupt flag to start (necessary)
  //while(!(TWCR & (1<<TWINT))); // wait until interrupt clear
  if(i2c_wait()) return 1;
  return 0;
}

void i2c_stop(){
  TWCR = (1<<TWSTO)|(1<<TWINT)|(1<<TWEN);
}

uint8_t i2c_write(uint8_t data){
  TWDR = data; // load data to data reg
  TWCR = (1<<TWEN)|(1<<TWINT); // start transmission
  //while(!(TWCR & (1<<TWINT))); //wait till interrupt clear
  if(i2c_wait()) return 1;
  return 0;
}

uint8_t i2c_read_ack(){  // send ack after reading, so slave sends next byte
  TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWEA);
  while(!(TWCR & (1<<TWINT)));
  return TWDR;
}

uint8_t i2c_read_nack(){  //final ack for slave after reading is finished
  TWCR = (1<<TWEN)|(1<<TWINT);
  while(!(TWCR & (1<<TWINT)));
  return TWDR;
}

void i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t data){
  i2c_start();
  i2c_write(addr<<1); // address + write (r/w=0)
  i2c_write(reg); // reg address inside mpu
  i2c_write(data); // data
  i2c_stop(); //stop condn
}

int16_t i2c_read_word(uint8_t addr, uint8_t reg) {
    i2c_start();
    i2c_write(addr << 1);
    i2c_write(reg);
    i2c_start(); // repeated start
    i2c_write((addr << 1) | 1);
    uint8_t high = i2c_read_ack();
    uint8_t low  = i2c_read_nack();
    i2c_stop();
    return (int16_t)((high << 8) | low);
}

uint8_t i2c_wait(){
  uint16_t timeout = 10000; //10ms
  while(!(TWCR & (1<<TWINT))){
    if(--timeout == 0) return 1; // timeout
  }
  return 0;
}

// MPU6050 Setup
void mpu_init(void) {
    uart_print("Writing to PWR_MGMT_1...\n");
    i2c_write_reg(MPU_ADDR, 0x6B, 0x00); //PWR_MGMT_1 = 0 : wake up
    uart_print("Done\n");
    
    uart_print("Writing to SMPLRT...\n");
    i2c_write_reg(MPU_ADDR, 0x19, 0x07); // SMPLRT_DIV = 0x07 : 125hz
    uart_print("Done\n");
    
    uart_print("Writing to CONFIG...\n");
    i2c_write_reg(MPU_ADDR, 0x1A, 0x06); // CONFIG = 0x06 : dgital lpf
    uart_print("Done\n");
    
    uart_print("Writing to GYRO_CONFIG...\n");
    i2c_write_reg(MPU_ADDR, 0x1B, 0x00); // GYRO_CONFIG = 0x00 :+- 250deg/sec range
    uart_print("Done\n");
    
    uart_print("Writing to ACCEL_CONFIG...\n");
    i2c_write_reg(MPU_ADDR, 0x1C, 0x00); // ACCEL_CONFIG = 0x00 : +- 2g range
    uart_print("Done\n");
    
    if (i2c_start()) uart_print("I2C START failed\n");
    if (i2c_write(MPU_ADDR<<1)) uart_print("MPU not ACKing\n");


}

// Angle calculation
float calc_pitch(int16_t ax, int16_t ay, int16_t az) {
    float axg = ax / 16384.0;
    float ayg = ay / 16384.0;
    float azg = az / 16384.0;
    return atan2(axg, sqrt(ayg*ayg + azg*azg)) * 180.0 / M_PI;
}

float calc_yaw(int16_t gz){
  float gzs = gz/131.0;
  static float yaw = 0;
  yaw = yaw + gzs*0.5;
  return yaw;
}

float calc_roll(int16_t ay, int16_t az){
  float ayg = ay/16384.0;
  float azg = az/16384.0;
  return atan2(ayg, azg)*180.0/ M_PI;
}

char buffer[32];

int main(void) {
    uart_init();
    uart_print("uart init done \n");
    i2c_init();
    _delay_ms(100); // allows sensor to stabilize after powering up and prevents immidienate register access before sensor is functional (from datasheet)
    mpu_init();

    while (1) {
        int16_t ax = i2c_read_word(MPU_ADDR, 0x3B);
        int16_t ay = i2c_read_word(MPU_ADDR, 0x3D);
        int16_t az = i2c_read_word(MPU_ADDR, 0x3F);
        
        //int16_t gx = i2c_read_word(MPU_ADDR, 0x43);
        //int16_t gy = i2c_read_word(MPU_ADDR, 0x45);
        int16_t gz = i2c_read_word(MPU_ADDR, 0x47);
        
        float roll = calc_roll(ay, az);
        float pitch = calc_pitch(ax, ay, az);
        float yaw = calc_yaw(gz);
        
        dtostrf(roll, 6, 2, buffer); // double to string
        uart_print("Roll: ");
        uart_print(buffer);
        uart_print(" deg | ");
        
        dtostrf(pitch, 6, 2, buffer);
        uart_print("Pitch: ");
        uart_print(buffer);
        uart_print(" deg\n");
        
        dtostrf(yaw, 6, 2, buffer);
        uart_print("Yaw: ");
        uart_print(buffer);
        uart_print(" deg\n");

        _delay_ms(500);
    }
    /*while(1){
      uart_print("hello! \n");
      _delay_ms(500);
    }*/
}


  


