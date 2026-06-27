#include "test_framework.h"
#include "protocol.h"
#include "tusb.h"
#include <string>

extern int8_t selected_idx;

// Helper to send a string via TinyUSB RX Callback
static void send_serial_data(const std::string& data) {
    mock_cdc_rx_data.clear();
    for (char c : data) {
        mock_cdc_rx_data.push_back(static_cast<uint8_t>(c));
    }
    mock_cdc_rx_idx = 0;
    tud_cdc_rx_cb(0); // Trigger TinyUSB callback to push into rx_ring
}

void test_protocol_init() {
    protocol_init();
    selected_idx = -1;
    for (int i = 0; i < MAX_AGENTS; i++) {
        AgentData* agent = protocol_get_agent(i);
        ASSERT_TRUE(agent != nullptr);
        ASSERT_EQ(agent->id, i);
        ASSERT_FALSE(agent->is_active);
        ASSERT_FALSE(agent->is_dirty);
    }
}

void test_protocol_valid_set() {
    protocol_init();
    selected_idx = -1;
    
    // Header fields: id=0, status_len=7, name_len=7, msg_len=10
    // Payload: RUNNING (7) + Agent_A (7) + Processing (10)
    std::string packet = "SET:0:7:7:10:RUNNINGAgent_AProcessing";
    send_serial_data(packet);
    
    // Process parser state machine
    handle_protocol_event();
    
    AgentData* agent = protocol_get_agent(0);
    ASSERT_TRUE(agent->is_active);
    ASSERT_EQ(agent->status, AGENT_STATUS_RUNNING);
    ASSERT_STR_EQ(agent->name, "Agent_A");
    ASSERT_STR_EQ(agent->message, "Processing");
    ASSERT_TRUE(agent->is_dirty);
    ASSERT_EQ(selected_idx, 0);
}

void test_protocol_valid_del() {
    protocol_init();
    selected_idx = -1;
    
    // First add an agent
    std::string packet_add = "SET:3:4:7:5:DONE Agent_CReady";
    send_serial_data(packet_add);
    handle_protocol_event();
    
    AgentData* agent = protocol_get_agent(3);
    ASSERT_TRUE(agent->is_active);
    ASSERT_EQ(selected_idx, 3);
    
    // Now delete it
    std::string packet_del = "DEL:3:";
    send_serial_data(packet_del);
    handle_protocol_event();
    
    ASSERT_FALSE(agent->is_active);
    ASSERT_EQ(selected_idx, -1); // selected_idx should reset or search for active agents (none active here)
}

void test_protocol_noise_resilience() {
    protocol_init();
    selected_idx = -1;
    
    // Send junk followed by a valid command
    std::string packet = "INVALIDCMD!!!SET:1:4:7:5:DONEAgent_BReady";
    send_serial_data(packet);
    handle_protocol_event();
    
    AgentData* agent = protocol_get_agent(1);
    ASSERT_TRUE(agent->is_active);
    ASSERT_EQ(agent->status, AGENT_STATUS_DONE);
    ASSERT_STR_EQ(agent->name, "Agent_B");
    ASSERT_STR_EQ(agent->message, "Ready");
}

void test_protocol_invalid_id() {
    protocol_init();
    selected_idx = -1;
    
    // Agent ID 9 is invalid (max is 7)
    std::string packet = "SET:9:4:7:5:DONE Agent_BReady";
    send_serial_data(packet);
    handle_protocol_event();
    
    // No crash, and agents remain inactive
    for (int i = 0; i < MAX_AGENTS; i++) {
        ASSERT_FALSE(protocol_get_agent(i)->is_active);
    }
}

void test_protocol_buffer_overflow_protection() {
    protocol_init();
    selected_idx = -1;
    
    // Name is 30 bytes, status is 4, msg is 5
    // MAX_NAME_LEN is 16, so it must truncate name to 15 chars + null terminator
    // String name: "LongAgentNameThatExceedsTheLimit" (32 chars, we specify 30 in header)
    // Payload: DONE (4) + LongAgentNameThatExceedsTheLimit (30) + Ready (5)
    std::string packet = "SET:2:4:30:5:DONELongAgentNameThatExceedsTheLimReady";
    send_serial_data(packet);
    handle_protocol_event();
    
    AgentData* agent = protocol_get_agent(2);
    ASSERT_TRUE(agent->is_active);
    // Truncated to 15 characters: "LongAgentNameTh"
    ASSERT_STR_EQ(agent->name, "LongAgentNameTh");
    ASSERT_STR_EQ(agent->message, "Ready");
}

void register_protocol_tests(std::vector<TestCase>& tests) {
    tests.push_back({"protocol_init", test_protocol_init});
    tests.push_back({"protocol_valid_set", test_protocol_valid_set});
    tests.push_back({"protocol_valid_del", test_protocol_valid_del});
    tests.push_back({"protocol_noise_resilience", test_protocol_noise_resilience});
    tests.push_back({"protocol_invalid_id", test_protocol_invalid_id});
    tests.push_back({"protocol_buffer_overflow_protection", test_protocol_buffer_overflow_protection});
}
