float readO2Vout() {
  long sum = 0;
  for (int i = 0; i < 32; i++) {
    sum += analogRead(pinAdc);
  }
  sum >>= 5;
  float MeasuredVout = sum * (VRefer / 1023.0);
  return MeasuredVout;
}

double readConcentration() {
  // Vout samples are with reference to 3.3V
  double MeasuredVout = readO2Vout();

  //float Concentration = FmultiMap(MeasuredVout, VoutArray,O2ConArray, 6);
  //when its output voltage is 2.0V,
  double Concentration = MeasuredVout * 0.2100 / 2.0000;
  double Concentration_Percentage = Concentration * 100.0000 * 10000.0000;
  return Concentration_Percentage;
}
