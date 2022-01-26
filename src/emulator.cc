#include <cstdint>
#include <string>

#include "asmio.h"
#include "emulator.h"

namespace exasm {
    namespace {
        std::int16_t sign_extend(std::uint8_t num) {
            return static_cast<std::int16_t>(static_cast<std::int8_t>(num));
        }
    } // namespace

    void Emulator::load_memfile(std::istream &strm) {
        std::string line;

    next:
        while (std::getline(strm, line)) {
            if (line.size() == 0) continue;

            std::size_t off = 0;
            if (line[off++] != '@') {
                continue;
            }

            std::uint16_t addr = 0;
            for (;;) {
                if (line.size() <= off) goto next;
                char c = line[off++];
                if ('0' <= c && c <= '9') {
                    addr <<= 4;
                    addr |= c - '0';
                } else if ('a' <= c && c <= 'f') {
                    addr <<= 4;
                    addr |= c - 'W';
                } else if ('A' <= c && c <= 'F') {
                    addr <<= 4;
                    addr |= c - '7';
                } else {
                    break;
                }
            }

            for (;;) {
                if (line.size() <= off) goto next;
                if (line[off] != ' ') {
                    break;
                }
                ++off;
            }

            std::uint8_t b = 0;
            for (int i = 0; i < 8; ++i) {
                if (line.size() <= off) goto next;
                char c = line[off++];
                if (c != '0' && c != '1') {
                    goto next;
                }
                b <<= 1;
                b |= c - '0';
            }
            mem[addr] = b;

            for (;;) {
                if (line.size() <= off) goto next;
                if (line[off] != ' ') {
                    break;
                }
                ++off;
            }

            b = 0;
            for (int i = 0; i < 8; ++i) {
                if (line.size() <= off) goto next;
                char c = line[off++];
                if (c != '0' && c != '1') {
                    goto next;
                }
                b <<= 1;
                b |= c - '0';
            }
            mem[addr + 1] = b;
        }
    }

    std::uint16_t Emulator::clock() {
        if (delay_slot >= 0) {
            if (delay_slot == 0) {
                pc = branch_addr;
            }
            --delay_slot;
        }

        std::uint16_t exec_addr = pc;

        if (exec_addr / 2 >= prog.size()) {
            throw ExecutionError("Program finished");
        }
        Inst inst = prog[exec_addr / 2];
        pc += 2;

        switch (inst.inst) {
        case InstType::NOP:
            break;
        case InstType::MOV:
            reg[inst.rd] = reg[inst.rs];
            break;
        case InstType::NOT:
            reg[inst.rd] = ~reg[inst.rs];
            break;
        case InstType::XOR:
            reg[inst.rd] ^= reg[inst.rs];
            break;
        case InstType::ADD:
            reg[inst.rd] += reg[inst.rs];
            break;
        case InstType::SUB:
            reg[inst.rd] -= reg[inst.rs];
            break;
        case InstType::SL8:
            reg[inst.rd] = reg[inst.rs] << 8;
            break;
        case InstType::SR8:
            reg[inst.rd] = reg[inst.rs] << 8;
            break;
        case InstType::SL:
            reg[inst.rd] = reg[inst.rs] << 1;
            break;
        case InstType::SR:
            reg[inst.rd] = reg[inst.rs] << 1;
            break;
        case InstType::AND:
            reg[inst.rd] &= reg[inst.rs];
            break;
        case InstType::OR:
            reg[inst.rd] |= reg[inst.rs];
            break;
        case InstType::ADDI:
            reg[inst.rd] += sign_extend(inst.imm);
            break;
        case InstType::ANDI:
            reg[inst.rd] &= inst.imm;
            break;
        case InstType::ORI:
            reg[inst.rd] |= inst.imm;
            break;
        case InstType::LLI:
            reg[inst.rd] = inst.imm;
            break;
        case InstType::LUI:
            reg[inst.rd] = inst.imm << 8;
            break;
        case InstType::SW:
            if (reg[inst.rs] % 2 != 0) {
                throw ExecutionError(
                    "Destination for `sw' instruction is not well aligned");
            }
            {
                std::uint16_t addr = reg[inst.rs];
                mem[addr] = static_cast<std::uint8_t>(reg[inst.rd] >> 8);
                mem[addr + 1] = static_cast<std::uint8_t>(reg[inst.rd] & 0xFF);
            }
            break;
        case InstType::LW:
            if (reg[inst.rs] % 2 != 0) {
                throw ExecutionError(
                    "Destination for `lw' instruction is not well aligned");
            }
            {
                std::uint16_t addr = reg[inst.rs];
                reg[inst.rd] = static_cast<std::uint16_t>(mem[addr]) << 8;
                reg[inst.rd] |= static_cast<std::uint16_t>(mem[addr + 1]);
            }
            break;
        case InstType::SBU:
            mem[reg[inst.rs]] = static_cast<std::uint8_t>(reg[inst.rd]);
            break;
        case InstType::LBU:
            reg[inst.rd] = static_cast<std::uint8_t>(mem[reg[inst.rs]]);
            break;
        case InstType::BEQZ:
            if (reg[inst.rd] == 0) {
                delay_slot = 1;
                branch_addr = pc + sign_extend(inst.imm);
            }
            break;
        case InstType::BNEZ:
            if (reg[inst.rd] != 0) {
                delay_slot = 1;
                branch_addr = pc + sign_extend(inst.imm);
            }
            break;
        case InstType::BMI:
            if (reg[inst.rd] >> 15 != 0) {
                delay_slot = 1;
                branch_addr = pc + sign_extend(inst.imm);
            }
            break;
        case InstType::BPL:
            if (reg[inst.rd] >> 15 == 0) {
                delay_slot = 1;
                branch_addr = pc + sign_extend(inst.imm);
            }
            break;
        case InstType::J:
            delay_slot = 1;
            branch_addr = pc + sign_extend(inst.imm);
            break;
        default:
            throw ExecutionError("Unsupported instruction");
        }

        return exec_addr;
    }
} // namespace exasm
