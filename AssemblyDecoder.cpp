#include <iostream>
#include <iterator>
#include <algorithm>
#include <fstream>
#include <string>

std::string regW[] = { "ax","cx","dx","bx","sp","bp","si","di" };
std::string regNotW[] = { "al","cl","dl","bl","ah","ch","dh","bh" };

std::string rmValues[] = { "bx + si","bx + di","bp + si","bp + di","si","di","bp","bx" };

void ImmediateToRegister(std::ofstream& output, std::istreambuf_iterator<char>& it, bool withSubcode)
{
    auto firstByte = *it;
    bool signedExtended = (firstByte & 0b00000010) == 0b00000010;
    char w = *it & 0b00000001;
    bool wordData = w == 0b00000001;

    it++;
    auto secondByte = *it;

    auto mod = ((secondByte >> 6) & 0b00000011);
    std::string* registerNames = wordData ? regW : regNotW;

    std::string rmFormatted;
    std::string immidiateFormatted;
    // immidiate to register
    if (mod == 0b00000011) {
        rmFormatted = registerNames[secondByte & 0b00000111];
    }
    // handle memory
    else if (mod == 0b00000000 || mod == 0b00000001 || mod == 0b00000010) {

        rmFormatted += "[";
        auto rm = secondByte & 0b00000111;
        // direct address
        if (mod == 0b00000000 && rm == 0b00000110) {
            it++;
            int displacement = *it;

            it++;
            auto hightDisplacement = *it;
            displacement = (hightDisplacement << 8) | displacement;

            rmFormatted += rmValues[rm] + " + " + std::to_string(displacement);
        }
        else {
            rmFormatted += rmValues[rm];
        }

        if (mod == 0b00000001 || mod == 0b00000010) {
            it++;
            int displacement = *it;

            if (mod == 0b00000010) {
                it++;
                auto hightDisplacement = *it;
                displacement = (hightDisplacement << 8) | displacement;
            }
            if (displacement != 0)
                rmFormatted += " + " + std::to_string(displacement);
        }

        rmFormatted += "]";
    }

    it++;
    int16_t immidiate = *it;

    if (wordData) {

        if (signedExtended) {
            bool negative = false;
            negative = ((immidiate >> 7) & 0b0000000000000001) == 1;
            if (negative)
                immidiate |= 0b1111111100000000;
            else
                immidiate &= 0b0000000011111111;
        }
        else {
            char lowByte = *it;
            it++;
            char hightByte = *it;
            immidiate = (hightByte << 8) | lowByte;
        }      
    }

    immidiateFormatted = std::to_string(immidiate);

    const char* instruction = "mov";
    if (withSubcode) {
        char subcode = secondByte & 0b00111000;
        if (subcode == 0b00000000) {
            instruction = "add";
        }
        else if (subcode == 0b00101000) {
            instruction = "sub";
        }
        else if (subcode == 0b00111000) {
            instruction = "cmp";
        }
    }

    output << instruction << " " << rmFormatted << " , " << immidiateFormatted;
}

void MemoryToMemory(const char* instruction, std::ofstream& output, std::istreambuf_iterator<char>& it)
{
    output << instruction << " ";
    char d = *it & 0b00000010;
    bool sourceInReg = d == 0b00000000;

    char w = *it & 0b00000001;
    bool wordData = w == 0b00000001;

    it++;
    auto secondByte = *it;

    auto mod = ((secondByte >> 6) & 0b00000011);
    std::string rmFormatted;
    std::string regFormatted;

    std::string* registerNames = wordData ? regW : regNotW;
    regFormatted = registerNames[(secondByte >> 3) & 0b00000111];

    // register to register
    if (mod == 0b00000011) {
        rmFormatted = registerNames[secondByte & 0b00000111];
    }
    // handle memory
    else if (mod == 0b00000000 || mod == 0b00000001 || mod == 0b00000010) {

        rmFormatted += "[";
        auto rm = secondByte & 0b00000111;
        // direct address
        if (mod == 0b00000000 && rm == 0b00000110) {
            it++;
            int displacement = *it;

            it++;
            auto hightDisplacement = *it;
            displacement = (hightDisplacement << 8) | displacement;

            rmFormatted += rmValues[rm] + " + " + std::to_string(displacement);
        }
        else {
            rmFormatted += rmValues[rm];
        }

        if (mod == 0b00000001 || mod == 0b00000010) {
            it++;
            int displacement = *it;

            if (mod == 0b00000010) {
                it++;
                auto hightDisplacement = *it;
                displacement = (hightDisplacement << 8) | displacement;
            }
            if (displacement != 0)
                rmFormatted += " + " + std::to_string(displacement);
        }

        rmFormatted += "]";
    }

    if (sourceInReg) {
        output << rmFormatted << ", ";
        output << regFormatted;
    }
    else {
        output << regFormatted << ", ";
        output << rmFormatted;
    }
}

void ImmidiateToRegister(const char* instruction, std::ofstream& output, std::istreambuf_iterator<char>& it)
{
    char w = *it & 0b00000001;
    bool wordData = w == 0b00000001;


    std::string regFormatted;

    std::string* registerNames = wordData ? regW : regNotW;
    regFormatted = registerNames[0];


    it++;
    int immidiate = *it;

    if (wordData) {
        it++;
        int hightImmidiate = *it;
        immidiate = ((hightImmidiate << 8) | (immidiate & 255)) & 65535;
    }

    std::string immidiateFormatted = std::to_string(immidiate);

    output << instruction<< " " << regFormatted << ", " << immidiateFormatted;
}

void ConditionalJump(const char* instruction, std::ofstream& output, std::istreambuf_iterator<char>& it) {
    it++;
    int immidiate = *it;
    std::string immidiateFormatted = std::to_string(immidiate);

    output << instruction << " " << immidiateFormatted;
}

int main()
{
    std::ifstream input("resources/test", std::ios::binary);
    std::ofstream output("result.asm");
    
    int16_t registers[8];

    std::istreambuf_iterator<char> end;
    auto it = std::istreambuf_iterator<char>(input);
    auto firstByte = *it;

    while (it != end)
    {
        // simple mov
        if ((*it & 0b11111100) == 0b10001000)
        {
            MemoryToMemory("mov", output, it);
        }
        // simple add
        else if ((*it & 0b11111100) == 0b00000000)
        {
            MemoryToMemory("add", output, it);
        }
        // simple sub
        else if ((*it & 0b11111100) == 0b00101000)
        {
            MemoryToMemory("sub", output, it);
        }
        // simple cmp
        else if ((*it & 0b11111100) == 0b00111000)
        {
            MemoryToMemory("cmp", output, it);
        }
        // immidiate to register/memory
        else if ((*it & 0b11111110) == 0b11000110)
        {
            ImmediateToRegister(output, it, false);
        }
        // immidiate to register/memory
        else if ((*it & 0b11111100) == 0b10000000)
        {
            ImmediateToRegister(output, it, true);
        }
        // immidate to accumulator add
        else if ((*it & 0b11111110) == 0b00000100)
        {
            ImmidiateToRegister("add", output, it);
        }
        // immidate to accumulator sub
        else if ((*it & 0b11111110) == 0b00101100)
        {
            ImmidiateToRegister("sub", output, it);
        }
        // immidate to accumulator add
        else if ((*it & 0b11111110) == 0b00111100)
        {
            ImmidiateToRegister("cmp", output, it);
        }
        else if ((*it) == (char)0b01110100) {
            ConditionalJump("je", output, it);
        }
        else if ((*it) == (char)0b01111100) {
            ConditionalJump("jl", output, it);
        }
        else if ((*it) == (char)0b01111110) {
            ConditionalJump("jle", output, it);
        }
        else if ((*it) == (char)0b01110010) {
            ConditionalJump("jb", output, it);
        }
        else if ((*it) == (char)0b01110110) {
            ConditionalJump("jbe", output, it);
        }
        else if ((*it) == (char)0b01111010) {
            ConditionalJump("jp", output, it);
        }
        else if ((*it) == (char)0b01110000) {
            ConditionalJump("jo", output, it);
        }
        else if ((*it) == (char)0b01111000) {
            ConditionalJump("js", output, it);
        }
        else if ((*it) == (char)0b01110101) {
            ConditionalJump("jne", output, it);
        }
        else if ((*it) == (char)0b01111101) {
            ConditionalJump("jnl", output, it);
        }
        else if ((*it) == (char)0b01111111) {
            ConditionalJump("jnle", output, it);
        }
        else if ((*it) == (char)0b01110011) {
            ConditionalJump("jnb", output, it);
        }
        else if ((*it) == (char)0b01110111) {
            ConditionalJump("jnbe", output, it);
        }
        else if ((*it) == (char)0b01111011) {
            ConditionalJump("jnp", output, it);
        }
        else if ((*it) == (char)0b01110001) {
            ConditionalJump("jno", output, it);
        }
        else if ((*it) == (char)0b01111001) {
            ConditionalJump("jns", output, it);
        }
        else if ((*it) == (char)0b11100010) {
            ConditionalJump("loop", output, it);
        }
        else if ((*it) == (char)0b11100001) {
            ConditionalJump("loopz", output, it);
        }
        else if ((*it) == (char)0b11100000) {
            ConditionalJump("loopnz", output, it);
        }
        else if ((*it) == (char)0b11100011){
            ConditionalJump("jcxz", output, it);
        }

        else if ((*it & 0b11110000) == 0b10110000) {
            char w = *it & 0b00001000;
            bool wordData = w == 0b00001000;


            std::string regFormatted;

            std::string* registerNames = wordData ? regW : regNotW;
            regFormatted = registerNames[*it & 0b00000111];


            it++;
            int immidiate = *it;

            if (wordData) {
                it++;
                int hightImmidiate = *it;
                immidiate = ((hightImmidiate << 8) | (immidiate & 255)) & 65535;
            }

            std::string immidiateFormatted = std::to_string(immidiate);

            output << "mov " << regFormatted << ", " << immidiateFormatted;
        }

        output << std::endl;
        it++;
    }
   
    input.close();
    output.close();

    std::cout << "Hello World!\n";

}