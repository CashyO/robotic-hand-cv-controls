## Robotic Hand CV Control

A computer-vision-based robotic hand control system using **Python**, **OpenCV**, **cvzone**, **MediaPipe**, and an **Arduino Mega 2560**.  
The Python program detects hand gestures from a webcam and sends finger-state packets over serial to the Arduino, which controls 5 servos.

## Features

- Real-time hand tracking with webcam input
- Finger detection using cvzone + MediaPipe
- Serial communication from Python to Arduino
- 5-servo robotic hand control
- Stable packet filtering to reduce jitter
- Arduino packet parsing with fixed 5-finger format

# Demo Setup

## Startup Order

1. Connect the Arduino Mega 2560
2. Upload the Arduino sketch using Arduino IDE
3. Close Arduino IDE
4. Connect external servo power supply
5. Activate Python virtual environment
6. Run the Python script
7. Show your hand to the webcam

## If Serial Port Fails

- Make sure Arduino IDE is closed
- Make sure Serial Monitor is closed
- Check the correct COM port in the Python script
- Replug the Arduino and try again

## If Hand Tracking Fails

- Confirm the webcam index:
  - `0` for default webcam
  - `1` for external webcam
- Check lighting
- Keep your hand fully visible in frame

## If Servos Jitter

- Add packet stability filtering
- Use proper external servo power
- Ensure common ground
- Check mechanical resistance in the fingers
## Packet Format

The Python script sends packets in this format:

```text
$<d0><d1><d2><d3><d4>

## Servo Signal Pins

Default Arduino pin mapping:

- Thumb  -> Pin 7
- Index  -> Pin 8
- Middle -> Pin 9
- Ring   -> Pin 10
- Pinky  -> Pin 11

## Servo Power

Use an external power source for the servos.


### Recommended wiring

- External PSU +V -> all servo red wires
- External PSU GND -> all servo brown/black wires
- Arduino GND -> external PSU GND
- Arduino signal pins -> servo signal wires

## Important

The Arduino and external servo power supply **must share a common ground**.

Without a common ground:
- servos may jitter
- signals may be inconsistent
- the system may not work reliably

## Power Safety

Do not power 5 servos directly from the Arduino 5V pin. 