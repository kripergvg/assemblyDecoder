/* ========================================================================
   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://computerenhance.com for more information

   ======================================================================== */

#include <stdio.h>

#include "sim86_shared.h"
#include <fstream>

unsigned char ExampleDisassembly[1024 * 1024];

#define REGISTERS_COUNT 15
#define REGISTERS_VARIATIONS 3
#define FLAGS_REGISTER_INDEX 14

static s32 Registers[REGISTERS_COUNT] =
{
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000
};

static int16_t flags;
static char const* Names[REGISTERS_COUNT] =
{
    "",
    "ax",
    "bx",
    "cx",
    "dx",
    "sp",
    "bp",
    "si",
    "di",
    "es",
    "cs",
    "ss",
    "ds",
    "ip",
    "flags"
};

#define ZERO_FLAG_SHIFT 6
#define SIGN_FLAG_SHIFT 7

struct DiffPrinter {

    s32 RegistersSnapshot[REGISTERS_COUNT];
    int16_t flagsSnapshot;

    DiffPrinter()
    {
        memcpy(&RegistersSnapshot, Registers, sizeof(Registers));
        flagsSnapshot = flags;
    }

    void CompareFlags(int shift, const char* name) {
        auto newValue = (flags >> shift) & 1;
        auto oldValue = (flagsSnapshot >> shift) & 1;
        if (newValue != oldValue) {
            printf(" %s->%i", name, newValue);
        }
    }

    ~DiffPrinter()
    {
        for (size_t registerIndex = 0; registerIndex < REGISTERS_COUNT; registerIndex++)
        {
            if (RegistersSnapshot[registerIndex] != Registers[registerIndex])
            {
                printf("%s: 0x%02x -> 0x%02x", Names[registerIndex], RegistersSnapshot[registerIndex], Registers[registerIndex]);
            }
        }

        if (flagsSnapshot != flags) {
            printf(" flags:");
            CompareFlags(ZERO_FLAG_SHIFT, "zero");
            CompareFlags(SIGN_FLAG_SHIFT, "sign");
        }
    } 
};

void setFlag(int shift, bool value) {
    if (value) {
        flags |= (1 << shift);
    }
    else {
        flags &= (~(1 << shift));
    }
}

int main(void)
{
    u32 Version = Sim86_GetVersion();
    printf("Sim86 Version: %u (expected %u)\n", Version, SIM86_VERSION);
    if (Version != SIM86_VERSION)
    {
        printf("ERROR: Header file version doesn't match DLL.\n");
        return -1;
    }

    instruction_table Table;
    Sim86_Get8086InstructionTable(&Table);
    printf("8086 Instruction Instruction Encoding Count: %u\n", Table.EncodingCount);

    FILE* File = {};
    size_t readCount = 0;
    if (fopen_s(&File, "resources/test", "rb") == 0)
    {
        readCount = fread(ExampleDisassembly, 1, sizeof(ExampleDisassembly), File);
        fclose(File);
    }

    u32 Offset = 0;
    while (Offset < readCount)
    {
        DiffPrinter printer{};
        instruction Decoded;
        Sim86_Decode8086Instruction(sizeof(ExampleDisassembly) - Offset, ExampleDisassembly + Offset, &Decoded);
        if (Decoded.Op)
        {
            switch (Decoded.Op)
            {
            case Op_mov:
            {
                auto destination = Decoded.Operands[0];
                auto source = Decoded.Operands[1];
                if (destination.Type == operand_type::Operand_Register) {

                    s32* destinationRegister = &Registers[destination.Register.Index];
                    int16_t sourceValue = 0;

                    if (source.Type == operand_type::Operand_Immediate) {
                        sourceValue = source.Immediate.Value;
                    }
                    else if (source.Type == operand_type::Operand_Register) {
                        sourceValue = Registers[source.Register.Index];
                    }

                    if (destination.Register.Count == 2)
                    {
                        *destinationRegister = sourceValue;
                    }
                    else {
                        *(((char*)destinationRegister) + (destination.Register.Offset & 1)) = (char)sourceValue;
                    }
                }

                break;
            }

            case Op_sub:
            {
                auto destination = Decoded.Operands[0];
                auto source = Decoded.Operands[1];
                if (destination.Type == operand_type::Operand_Register) {
                    s32* destinationRegister = &Registers[destination.Register.Index];

                    int16_t sourceValue = 0;
                    if (source.Type == operand_type::Operand_Immediate) {
                        sourceValue = source.Immediate.Value;
                    }
                    else if (source.Type == operand_type::Operand_Register) {
                        sourceValue = Registers[source.Register.Index];
                    }

                    *destinationRegister = ((int16_t)*destinationRegister) - sourceValue;
                    setFlag(ZERO_FLAG_SHIFT, *destinationRegister == 0);
                    setFlag(SIGN_FLAG_SHIFT, *destinationRegister < 0);
                }

                break; 
            }
            case Op_cmp: {
                auto destination = Decoded.Operands[0];
                auto source = Decoded.Operands[1];
                if (destination.Type == operand_type::Operand_Register) {
                    s32* destinationRegister = &Registers[destination.Register.Index];

                    int16_t sourceValue = 0;
                    if (source.Type == operand_type::Operand_Immediate) {
                        sourceValue = source.Immediate.Value;
                    }
                    else if (source.Type == operand_type::Operand_Register) {
                        sourceValue = Registers[source.Register.Index];
                    }

                    auto value = ((int16_t)*destinationRegister) - sourceValue;
                    setFlag(ZERO_FLAG_SHIFT, value == 0);
                    setFlag(SIGN_FLAG_SHIFT, value < 0);
                }

                break;
            }
            case Op_add: {
                auto destination = Decoded.Operands[0];
                auto source = Decoded.Operands[1];
                if (destination.Type == operand_type::Operand_Register) {
                    s32* destinationRegister = &Registers[destination.Register.Index];

                    int16_t sourceValue = 0;
                    if (source.Type == operand_type::Operand_Immediate) {
                        sourceValue = source.Immediate.Value;
                    }
                    else if (source.Type == operand_type::Operand_Register) {
                        sourceValue = Registers[source.Register.Index];
                    }

                    *destinationRegister = ((int16_t)*destinationRegister) + sourceValue;
                    setFlag(ZERO_FLAG_SHIFT, *destinationRegister == 0);
                    setFlag(SIGN_FLAG_SHIFT, *destinationRegister < 0);
                }

                break;
            }
            default:
                break;
            }

            Offset += Decoded.Size;
            printf("\n");
        }
        else
        {
            printf("Unrecognized instruction\n");
            break;
        }
    }
    printf("\n");

    printf("Final state\n");
    for (size_t registerIndex = 0; registerIndex < REGISTERS_COUNT; registerIndex++)
    {
        printf("%s: 0x%02x\n", Names[registerIndex], Registers[registerIndex]);
    }

    return 0;
}