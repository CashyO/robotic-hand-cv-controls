# Overview: 
# - Send signal strings to the arduino board
# - Signal strings are based on hand gestures
#
# Need to use Python 3.11 for cvzone / mediapipe to work.
# Create venv:
#   py -3.11 -m venv myenv
# Activate:
#   myenv\Scripts\activate
# Deactivate: 
#   myenv\Scripts\deactivate 
# Install dependencies: 
#  python -m pip install -r requirements.txt
# Run: 
#  python multiServoControl_test.py 
# 
# IMPORTANT:
# - Make sure Arduino IDE Serial Monitor / Plotter is CLOSED (port can only be opened by one program).
# - Update PORT below if Windows changes it.

import time
import serial
import serial.tools.list_ports

# ----------------------------
# SERIAL SETUP FIRST (fails fast if port is busy / wrong)
# ----------------------------

# Update this to match your Arduino's port and baud rate. (COM6 arduino )
PORT = "COM6"
BAUD = 9600

print("Detected serial ports:")
for p in serial.tools.list_ports.comports():
    print(f"  {p.device}  {p.description}")

print(f"\nOpening Arduino on {PORT} @ {BAUD}...")
arduino = serial.Serial(PORT, BAUD, timeout=1)

# Many Arduinos reset when the serial port opens
time.sleep(2)
arduino.reset_input_buffer()
arduino.reset_output_buffer()
print("Serial open OK.\n")

# ----------------------------
# CV IMPORTS/SETUP (only after serial is confirmed working)
# ----------------------------
import cv2
from cvzone.HandTrackingModule import HandDetector

# Camera setup
cap = cv2.VideoCapture(0)  # 0 = default webcam, 1 = external
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)

# Hand detector
detector = HandDetector(maxHands=1, detectionCon=0.8)

def fingers_to_packet(fingers):
    """[1,0,1,1,0] -> '$10110'"""
    return "$" + "".join(str(int(v)) for v in fingers)

# --- filtering / safety ---
STABLE_FRAMES = 4           # require same packet this many frames
NO_HAND_SEND_OPEN_SEC = 0.6 # if no hand for this long, send OPEN_PACKET once

candidate_packet = None
candidate_count = 0
last_sent_packet = None
last_hand_seen = time.time()

# Decide what your "safe" pose is when no hand is detected.
# Keep "$00000" if that means "open" on your hardware, otherwise change it.
# 0 = open ie no power to servo 
# 1 = closed ie power to servo 
OPEN_PACKET = "$00000"

try:
    # Main Loop: read hand & send packets to arduino until 'q' or camera fail
    while True:
        success, img = cap.read()
        if not success:
            print("Failed to read from camera.")
            break

        hands, img = detector.findHands(img, draw=True)
        now = time.time()

        if hands:
            last_hand_seen = now
            # 1 hand
            hand = hands[0]
            fingers = detector.fingersUp(hand)  # [T, I, M, R, P]
            packet = fingers_to_packet(fingers)

            # stability filter
            if packet == candidate_packet:
                candidate_count += 1
            else:
                candidate_packet = packet
                candidate_count = 1

            # Send qualified packets 
            if candidate_count >= STABLE_FRAMES and packet != last_sent_packet:
                print("Sending:", packet, "fingers:", fingers)
                arduino.write(packet.encode("utf-8"))
                last_sent_packet = packet

        else:
            # if hand lost for long enough, send OPEN_PACKET once
            if (now - last_hand_seen) >= NO_HAND_SEND_OPEN_SEC and last_sent_packet != OPEN_PACKET:
                print("No hand -> Sending:", OPEN_PACKET)
                arduino.write(OPEN_PACKET.encode("utf-8"))
                last_sent_packet = OPEN_PACKET
                candidate_packet = None
                candidate_count = 0

        cv2.imshow("Image", img)

        # press 'q' to quit
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

finally:
    # Ensure we always clean up even if you Ctrl+C or crash
    cap.release()
    try:
        arduino.close()
    except Exception:
        pass
    cv2.destroyAllWindows()
    print("Clean shutdown.")