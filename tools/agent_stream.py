import serial
import time
import sys

# Update this to match your Pico's port
PORT = '/dev/tty.usbmodem101' 
BAUD = 115200

def run_interactive():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        print(f"Connected to {PORT}")
    except Exception as e:
        print(f"Error: Could not open {PORT}. {e}")
        sys.exit(1)

    def send_agent_update(agent_id, status, name, message):
        payload = status + name + message
        header = f"SET:{agent_id}:{len(status)}:{len(name)}:{len(message)}:"
        packet = header.encode('ascii') + payload.encode('ascii')
        
        print(f"Sending: {packet}")
        ser.write(packet)

    try:
        while True:
            print("\n--- Agent Monitor Control ---")
            agent_id = input("Agent ID (0-3): ")
            status = input("Status (e.g., RUNNING): ")
            name = input("Name (e.g., Agent_A): ")
            msg = input("Message: ")
            
            send_agent_update(agent_id, status, name, msg)
            
            # Listen for brief response
            print("Pico response:")
            time.sleep(0.5)
            while ser.in_waiting > 0:
                print(ser.readline().decode('ascii', errors='ignore').strip())
                
    except KeyboardInterrupt:
        print("\nExiting.")
    finally:
        ser.close()

if __name__ == "__main__":
    run_interactive()
