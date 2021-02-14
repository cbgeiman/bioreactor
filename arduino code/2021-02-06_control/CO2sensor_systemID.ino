void sendRequest(byte packet[])
{
  while (!cots1.available()) //keep sending request until we start to get a response
  {
    cots1.write(readCO2, 7);
    delay(50);
  }

  int timeout = 0; //set a timeoute counter
  while (cots1.available() < 7 ) //Wait to get a 7 byte response
  {
    timeout++;
    if (timeout > 10)   //if it takes to long there was probably an error
    {
      while (cots1.available()) //flush whatever we have
        cots1.read();
      Serial.println("CO2 timeout...");
      break;                        //exit and try again
    }
    delay(50);
  }

  for (int i = 0; i < 7; i++)
  {
    response[i] = cots1.read();
  }
}

unsigned long getValue(byte packet[])
{
  int high = packet[3];                        //high byte for value is 4th byte in packet in the packet
  int low = packet[4];                         //low byte for value is 5th byte in the packet


  unsigned long val = high * 256 + low;              //Combine high byte and low byte with this formula to get value
  return val * valMultiplier;
}
