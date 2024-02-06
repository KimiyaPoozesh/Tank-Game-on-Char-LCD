import serial

# Change these values as needed
port = 'COM3'  # or '/dev/ttyUSB0' on Linux
baudrate = 9600
filename = 'output.txt'

ser = serial.Serial(port, baudrate)

print("Script started")

buffer = ""  
end_marker = "."

try:
    while True:
        if ser.in_waiting > 0:
            print("waiting")
            byte = ser.read().decode('utf-8') 
            print("after byte")
            buffer += byte
            print(byte)
            
            if byte == end_marker:
                buffer += byte
                print("End of transmission received.")
                break
except Exception as e:
    print(f"Error: {e}")

with open(filename, 'w') as f:
    f.write(buffer[:-len(end_marker)])

print("Script ended")
