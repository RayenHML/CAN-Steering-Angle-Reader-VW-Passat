// CAN Receive Example (Extended)
//
// Original code provided by the MCP_CAN library example.
// Modified and extended by RayenHML for reading the steering angle of a VW Passat.
// 
// - Filtered the CAN signals to isolate the steering angle signal.
// - Extracted and processed the steering angle value based on a ".DBC" file.

#include <mcp_can.h>
#include <SPI.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];                             // Array to store serial string

#define CAN0_INT 2                               // Set INT to pin 2
MCP_CAN CAN0(9);                                 // Set CS to pin 9

void setup()
{
  Serial.begin(115200);
  
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");
  
  CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.

  pinMode(CAN0_INT, INPUT);                            // Configuring pin for /INT input
  
  Serial.println("MCP2515 Library Receive Example...");
}

void loop()
{
  bool validMessageReceived = false;

  if (!digitalRead(CAN0_INT)) // If CAN0_INT pin is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf); // Read data: len = data length, buf = data byte(s)

    if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
    else
    {
      if (rxId == 0x086)                  // Check if standard ID is 0x086 (Steering angle ID)
      {
        validMessageReceived = true;
        extractSignalData(rxBuf);         // Call function "extractSignalData" to extract and process the signal data
      }
    }
  }
}

void extractSignalData(unsigned char *data)
{
  // Define the signal parameters of the steering angle signal (Refer to README.md)
  int bitPosition = 16;           // Start bit position of the signal
  int bitLength = 13;             // Length of the signal
  double scalingFactor = 0.1;
  double offset = 0.0;
  double minValue = 0.0;
  double maxValue = 800.0;

  // Extract the raw steering angle value from data bytes (little-endian format -- refer to DBC explanation)
  unsigned int angleRawValue = ((unsigned int)data[3] << 8) | data[2];

  // Mask the bits to get only the 13-bit value
  angleRawValue &= 0x1FFF;

  // Calculate the value taking in account the scaling factor and the offset
  double angleActualValue = (angleRawValue * scalingFactor) + offset;

  // Define signal parameters for the sign (positive or negative)
  int signBitPosition = 29;       // Start bit position for the sign is 29

  // Extract the raw sign value from the data bytes (only 1-bit)
  unsigned int signByteIndex = signBitPosition / 8;
  unsigned int signBitOffset = signBitPosition % 8;
  unsigned int signRawValue = (data[signByteIndex] >> signBitOffset) & 0x01;

  // Determine whether the steering angle is positive or negative based on "angleSign"
  char angleSign = (signRawValue == 0) ? '+' : '-';

  // Print the extracted angle value with sign
  Serial.print("Angle Value: ");      
  Serial.print(angleSign);            // Print the sign
  Serial.print(angleActualValue);     // Print the steering angle value
  Serial.println();                  

  // Optional: test if the actual steering angle value is within min and max bounds
  if (angleActualValue < minValue)
  {
    angleActualValue = minValue;
  }
  else if (angleActualValue > maxValue)
  {
    angleActualValue = maxValue;
  }

}
