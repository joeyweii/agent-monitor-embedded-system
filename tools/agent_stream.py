import serial
import time
import sys
import random
import threading

# Usage:
# 1. Ensure pyserial is installed: pip install pyserial
# 2. Update the PORT variable to match your Pico device (e.g., /dev/tty.usbmodem101)
# 3. Run the script: python3 tools/agent_stream.py
#
# Commands:
# - ADD <id> <name>       : Register a new agent. Status defaults to DONE.
# - RUN <id> <message>    : Transition agent to RUNNING state. 
#                           After 3 seconds, it will randomly transition to DONE, ERROR, or INPUT.
# - QUIT                  : Exit the simulator.

# Update this to match your Pico's port
PORT = '/dev/tty.usbmodem101' 
BAUD = 115200

# Agent storage
agents = {}
# Thread safe lock for agents dict
agents_lock = threading.Lock()

def send_agent_update(ser, agent_id, status, name, message):
    # Construct the payload
    payload = status + name + message
    header = f"SET:{agent_id}:{len(status)}:{len(name)}:{len(message)}:"
    packet = header.encode('ascii') + payload.encode('ascii')
    
    print(f"\n[SEND] ID:{agent_id} | Status:{status} | Name:{name} | Msg:{message}")
    ser.write(packet)
    time.sleep(0.1) # Small delay for buffer stability

def agent_worker(ser, aid):
    """Background thread: Monitor individual agent state and handle simulated transition"""
    time.sleep(3) # Simulate execution time
    
    with agents_lock:
        if aid in agents and agents[aid]["status"] == "RUNNING":
            final_states = [("DONE", "Success"), ("ERROR", "Failed"), ("INPUT", "Need approval")]
            res_status, res_msg = random.choice(final_states)
            agents[aid]["status"] = res_status
            agents[aid]["msg"] = res_msg
            send_agent_update(ser, aid, res_status, agents[aid]["name"], res_msg)

def run_simulator():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        print(f"Connected to {PORT}")
    except Exception as e:
        print(f"Error: Could not open {PORT}. {e}")
        sys.exit(1)

    print("\n--- Interactive Agent Simulator ---")
    print("ADD <id> <name>          - Add a new agent (Default: DONE)")
    print("RUN <id> <msg>           - Start agent (Status: RUNNING)")
    print("QUIT                     - Exit")

    try:
        while True:
            cmd_line = input("> ").split()
            if not cmd_line: continue
            cmd = cmd_line[0].upper()
            
            if cmd == "ADD":
                aid = int(cmd_line[1])
                name = cmd_line[2]
                with agents_lock:
                    agents[aid] = {"name": name, "status": "DONE", "msg": "Ready"}
                send_agent_update(ser, aid, "DONE", name, "Ready")
            
            elif cmd == "RUN":
                aid = int(cmd_line[1])
                msg = " ".join(cmd_line[2:])
                with agents_lock:
                    if aid in agents:
                        agents[aid]["status"] = "RUNNING"
                        agents[aid]["msg"] = msg
                        send_agent_update(ser, aid, "RUNNING", agents[aid]["name"], msg)
                        # Start background thread to handle transition
                        threading.Thread(target=agent_worker, args=(ser, aid), daemon=True).start()
                    else:
                        print("Agent ID not found. ADD it first.")
            elif cmd == "QUIT":
                break
    except Exception as e:
        print(f"Error: {e}")
    finally:
        ser.close()

if __name__ == "__main__":
    run_simulator()
