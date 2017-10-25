clc, clear, close all

PINS_NUMBER = 25;
photoMeterPins = 1:PINS_NUMBER;
%photoMeterValues = 
mesureBin = [0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1];
photoMeterValues = mesureBin;

%mesureDec =bi2de(mesureBin);

decimalValue = 0;

for i=1: PINS_NUMBER
    %photoMeterValues(i) = digitalRead(photoMeterPins(i));
    decimalValue = bitshift(decimalValue, 1) + photoMeterValues(i);
end

ratio = 100*decimalValue/2^25;