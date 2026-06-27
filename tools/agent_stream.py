import serial
import time
import sys
import random
import threading

# Update this to match your Pico's port
PORT = '/dev/tty.usbmodem101' 
BAUD = 115200

# Agent storage
agents = {}
# Thread safe lock for agents dict
agents_lock = threading.Lock()

def read_pico_output(ser):
    """Background thread: Monitor Pico's printf output"""
    while ser.is_open:
        try:
            if ser.in_waiting > 0:
                line = ser.readline().decode('ascii', errors='ignore').strip()
                if line:
                    print(f"\n[PICO LOG]: {line}")
        except Exception as e:
            break
        time.sleep(0.05)

def send_agent_update(ser, agent_id, status, name, message):
    payload = status + name + message
    header = f"SET:{agent_id}:{len(status)}:{len(name)}:{len(message)}:"
    packet = header.encode('ascii') + payload.encode('ascii')
    
    print(f"\n[SEND] ID:{agent_id} | Status:{status} | Name:{name} | Msg:{message}")
    ser.write(packet)
    time.sleep(0.1)

def agent_worker(ser, aid):
    """Background thread: Simulate agent lifecycle"""
    wait_time = random.uniform(5, 10)
    time.sleep(wait_time)
    
    with agents_lock:
        if aid in agents and agents[aid]["status"] == "RUNNING":
            # Weighted outcome: 40% done, 35% error, 25% needs approval
            outcome = random.choices(
                ["DONE", "ERROR", "INPUT"],
                weights=[40, 35, 25]
            )[0]

            done_messages = [
                "All tasks completed successfully. Output written to /workspace/out.",
                "Finished. 47 files processed, 0 errors, 3 warnings.",
                "Build passed. Unit tests: 112/112. Coverage: 94.3%.",
                "Deployment complete. Service healthy on port 8080.",
                "Analysis done. Report saved to results/summary_2024.md.",
            ]

            error_messages = [
                (
                    "Segfault in memory allocator at heap.c:312. "
                    "Attempted to free a pointer that was never allocated. "
                    "Stack trace logged to /tmp/crash_dump_20240610.log. "
                    "Possible cause: double-free in cleanup routine after "
                    "signal handler interrupted malloc. Recommend reviewing "
                    "thread-safety of allocator under concurrent load."
                ),
                (
                    "API rate limit exceeded (429 Too Many Requests) on "
                    "third call to api.openai.com/v1/chat/completions. "
                    "Retried 3x with exponential backoff (2s, 4s, 8s), "
                    "all failed. Current quota: 90,000 TPM. "
                    "Task cannot continue without token budget increase "
                    "or switching to a lower-priority model endpoint."
                ),
                (
                    "CMake configure failed: arm-none-eabi-gcc not found "
                    "in PATH. Checked /usr/bin, /usr/local/bin, /opt/homebrew/bin. "
                    "Build system requires GCC ARM toolchain >= 10.3. "
                    "Install via: brew install --cask gcc-arm-embedded. "
                    "See README.md Section 2 for full environment setup guide."
                ),
                (
                    "Test suite FAILED: 8 of 54 integration tests failed. "
                    "Failures concentrated in auth module (5) and payment "
                    "gateway mock (3). Root cause: JWT secret mismatch between "
                    "test fixtures and updated .env.test config. "
                    "All failures are environment-related, no logic regressions "
                    "detected. Fix: update TEST_JWT_SECRET in test/config.py."
                ),
                (
                    "Out of disk space during dataset preprocessing. "
                    "Required: ~18.4 GB. Available: 2.1 GB on /dev/sda1. "
                    "Process terminated while writing shard 7 of 12. "
                    "Partial outputs cleaned up. Consider mounting additional "
                    "volume or reducing batch size before retrying."
                ),
                (
                    "Git merge conflict in src/core/scheduler.cpp. "
                    "Conflicting changes in dispatch_loop() between branch "
                    "'feat/priority-queue' and 'main'. Auto-merge failed. "
                    "Manual resolution required in lines 204-231. "
                    "Recommend reviewing both branches with: "
                    "git diff main feat/priority-queue -- src/core/scheduler.cpp"
                ),
            ]

            approval_messages = [
                (
                    "Proposed: DELETE all records from users table where "
                    "last_login < 2022-01-01 (est. 14,302 rows). "
                    "This is IRREVERSIBLE without a backup restore. "
                    "Backup from 2024-06-09 03:00 UTC is available. "
                    "Confirm deletion? [APPROVE] to proceed, [REJECT] to cancel."
                ),
                (
                    "Ready to push 3 commits to branch 'main' on origin. "
                    "Includes: refactor auth middleware, update prod DB schema "
                    "(migration v42), remove legacy API v1 endpoints. "
                    "CI is green. This will trigger auto-deploy to production. "
                    "Approve push? [APPROVE] to push, [REJECT] to hold."
                ),
                (
                    "Estimated cost for this GPT-4o batch job: $47.20 USD "
                    "(approx. 2.36M input tokens + 590K output tokens). "
                    "Current monthly spend: $312 / $500 budget. "
                    "Job will run for ~22 min on 4 parallel workers. "
                    "Approve spend? [APPROVE] to start, [REJECT] to abort."
                ),
                (
                    "Requesting sudo access to modify /etc/hosts and restart "
                    "nginx on prod-server-03 (10.0.1.43). "
                    "Reason: redirect legacy subdomain api-v1.example.com "
                    "to new service at api.example.com. "
                    "Change is low-risk but requires root. "
                    "Grant permission? [APPROVE] to continue, [REJECT] to skip."
                ),
                (
                    "Tool call: browser.navigate('https://billing.stripe.com') "
                    "requires access to live Stripe dashboard. "
                    "Agent will read current invoice status for customer "
                    "cus_Px7EXAMPLE and update local records.json. "
                    "No write operations planned. "
                    "Allow browser access? [APPROVE] to proceed, [REJECT] to cancel."
                ),
            ]

            if outcome == "DONE":
                res_msg = random.choice(done_messages)
            elif outcome == "ERROR":
                res_msg = random.choice(error_messages)
            else:
                res_msg = random.choice(approval_messages)

            agents[aid]["status"] = outcome
            agents[aid]["msg"] = res_msg
            send_agent_update(ser, aid, outcome, agents[aid]["name"], res_msg)

def send_agent_delete(ser, agent_id):
    packet = f"DEL:{agent_id}:".encode('ascii')
    print(f"\n[SEND] DELETE ID:{agent_id}")
    ser.write(packet)
    time.sleep(0.1)

def run_simulator():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        print(f"Connected to {PORT}")
    except Exception as e:
        print(f"Error: Could not open {PORT}. {e}")
        sys.exit(1)

    # Start background thread to monitor Pico output
    monitor_thread = threading.Thread(target=read_pico_output, args=(ser,), daemon=True)
    monitor_thread.start()

    print("\n--- Interactive Agent Simulator ---")
    print("ADD <id> <name>          - Add a new agent (Default: DONE)")
    print("RUN <id> <msg>           - Start agent (Status: RUNNING)")
    print("DEL <id>                 - Delete an agent")
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
                        threading.Thread(target=agent_worker, args=(ser, aid), daemon=True).start()
                    else:
                        print("Agent ID not found. ADD it first.")
            elif cmd == "DEL":
                aid = int(cmd_line[1])
                with agents_lock:
                    if aid in agents:
                        del agents[aid]
                        send_agent_delete(ser, aid)
                    else:
                        print("Agent ID not found.")
            elif cmd == "QUIT":
                break
    except Exception as e:
        print(f"Error: {e}")
    finally:
        ser.close()

if __name__ == "__main__":
    run_simulator()
