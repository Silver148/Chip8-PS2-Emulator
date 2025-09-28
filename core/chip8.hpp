#ifndef CHIP8_H
#define CHIP8_H

#include <condition_variable>

class Chip8{
private:
    uint8_t registers[16]{};
	uint8_t memory[4096]{};
	uint16_t index{};
	uint16_t pc{};
	uint16_t stack[16]{};
	uint8_t sp{};
	uint8_t delayTimer{};
	uint8_t soundTimer{};
	uint16_t opcode;
    void init();

public:
    Chip8();
    ~Chip8();
    void emulate_cycles();
    bool LoadROM(const char *path);
    bool drawFlag;
	uint8_t keypad[16]{};
	uint32_t video[64 * 32]{};

	bool keypadLast[16]={0};
    bool is_waiting_for_key;
    uint8_t key_waiting_register;
};

#endif