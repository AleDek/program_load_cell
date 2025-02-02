#include "I2C.h"
#include "ThisThread.h"
#include "mbed.h"
#include "ZSC31014.h"
#include <cstdint>
#include <cstdio>

#define THERM_1_SDA PB_7 //front thermal
#define THERM_1_SCL PB_6

#define THERM_2_SDA PB_4 //rear thermal
#define THERM_2_SCL PA_7


#define  GAIN      x192    //1.5 - 3 - 6 - 12 - 24 - 48 - 96 - 192
#define New_address (0x33)

using namespace metromotive;


DigitalOut enable(PB_3); // The GPIO that you use to power the IC.
I2C i2c(THERM_1_SDA, THERM_1_SCL); // The I2C bus SDA/SCL pins.
char i2cAddress = 0x28;
// char i2cAddress = 0x33;
ZSC31014 DYMH(i2c, i2cAddress, enable); // The ZSC31014 IC, using the default address.
Serial pc(USBTX, USBRX, 115200);  

void calib() {
    printf("\n****\nSTART CALIB\n****\n");
    printf("\nNew Address = 0x%3x \n",New_address);
    // rtos::ThisThread::sleep_for(15ms);
    wait(0.015);
    DYMH.startCommandMode();
    printf("\nNew Address = 0x%3x \n",New_address);
    // rtos::ThisThread::sleep_for(15ms);
    wait(0.015);

    unsigned int customerID0 = DYMH.getCustomerID0();
    unsigned int customerID1 = DYMH.getCustomerID1();
    unsigned int customerID2 = DYMH.getCustomerID2();

    struct ZSC31014::FactoryID factoryID = DYMH.getFactoryID();

    //printf(
    //    "Factory ID:\nLot # %d\nWafer # %d\nCoordinate (x/y): %d/%d,\nI2C Address: 0x%x\n",
    //    factoryID.lotNumber,
    //    factoryID.waferNumber,
    //    factoryID.waferXCoordinate, 
    //    factoryID.waferYCoordinate,
    //    i2cAddress
    //);

    struct ZSC31014::ZMDIConfig1 zmdiConfig1 = DYMH.getZMDIConfig1();

    zmdiConfig1.updateRate = ZSC31014::UpdateRate::fastest;

    DYMH.setZMDIConfig1(zmdiConfig1);

    printf("set setZMDIConfig1\n");
    // rtos::ThisThread::sleep_for(15ms);
    wait(0.015);

    struct ZSC31014::ZMDIConfig2 zmdiConfig2 = DYMH.getZMDIConfig2();

    zmdiConfig2.enableSensorConnectionCheck = 1;
    zmdiConfig2.enableSensorShortCheck = 1;
    zmdiConfig2.slaveAddress = New_address; // Set to whatever you want this to be.
    zmdiConfig2.lockAddress = true;
    zmdiConfig2.lockEEPROM = false;

    DYMH.setZMDIConfig2(zmdiConfig2);

    printf("set setZMDIConfig2\n");
    // rtos::ThisThread::sleep_for(15ms);
    wait(0.015);

    struct ZSC31014::BridgeConfig bridgeConfig = DYMH.getBridgeConfig();

    bridgeConfig.disableNulling = false;
    bridgeConfig.mux = ZSC31014::MuxMode::fullBridge;
    bridgeConfig.useBSink = true;
    bridgeConfig.useLongIntegration = true;
    bridgeConfig.preAmpGain = ZSC31014::PreAmpGain::GAIN;
    bridgeConfig.preAmpOffset = 0b0001;

    DYMH.setBridgeConfig(bridgeConfig);

    printf("set bridgeconf\n");
    // rtos::ThisThread::sleep_for(15ms);
    wait(0.015);

    DYMH.setOffset(0xE400);

    // printf("\nNew Address = 0x%3x \n",New_address);
    printf("set Offset\n");
    // rtos::ThisThread::sleep_for(15ms);
    wait(0.015);

    DYMH.startNormalOperationMode();

    printf("set normal operations\n");
    // rtos::ThisThread::sleep_for(15ms);
    wait(0.015);

    printf("Wrote basic configuration and started normal operation mode.\n");
}

int main()
{
    calib();
    printf("\nNew Address = 0x%3x \n",New_address);
    // rtos::ThisThread::sleep_for(500ms);
    wait(0.5);

    enable = true;

    uint16_t temp = 0;
    int sum_of_elems = 0;
    int average = 0;
    char reading[2] = {0, 0};

    i2c.read(i2cAddress << 1, reading, 2);
    // printf("\nNew Address = 0x%3x \n",New_address);
    // rtos::ThisThread::sleep_for(500ms);
    wait(0.5);
    for (int i=0; i<20; i++) {
        uint16_t tare[20] = {0};
        i2c.read(i2cAddress << 1, reading, 2);
        tare[i] = reading[0] & 0b0111111;
        tare[i] <<= 8;
        tare[i] |= reading[1];
        printf("Read %d  %d\n", i, tare[i]);
        sum_of_elems += tare[i];
        // printf("\nNew Address = 0x%3x \n",New_address);
        // rtos::ThisThread::sleep_for(100ms);
        wait(0.1);
    }
    average = sum_of_elems / 20;
    printf("Average %d\n\n-------\n\n",average);

    while(1) {
        reading[0] = 0;
        reading[1] = 0;
        i2c.read(i2cAddress << 1, reading, 2);
        // printf("\nNew Address = 0x%3x \n",New_address);
     // rtos::ThisThread::sleep_for(20ms);
        wait(0.02);
        temp = reading[0] & 0b0111111;
        temp <<= 8;
        temp |= reading[1];
        printf("Read:  %d g\n", temp-average);
        // printf("\nNew Address = 0x%3x \n",New_address);
     // rtos::ThisThread::sleep_for(10ms);
        wait(0.1);
    }
}