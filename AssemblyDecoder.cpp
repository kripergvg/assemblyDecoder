#include <iostream>
#include <iterator>
#include <algorithm>
#include <fstream>

int main()
{
    std::ifstream input("resources/listing_0038_many_register_mov", std::ios::binary);
    std::ofstream output("result.asm");
    
    int16_t registers[8];


    std::string regW[] = {"ax","cx","dx","bx","sp","bp","si","di" };
    std::string regNotW[] = { "al","cl","dl","bl","ah","ch","dh","bh" };


    std::istreambuf_iterator<char> end;
    auto it = std::istreambuf_iterator<char>(input);
    auto firstByte = *it;

    while (it != end)
    {
        // simple mov
        if ((*it & 0b11111100) == 0b10001000)
        {
            output << "mov ";
            char d = *it & 0b00000010;
            bool sourceInReg = d == 0b00000000;

            char w = *it & 0b00000001;
            bool wordData = w == 0b00000001;

            it++;
            auto secondByte = *it;

            // register to register
            if ((secondByte & 0b11000000) == 0b11000000) {
                int firstRegister;
                int secondRegister;

                if (sourceInReg) {
                    firstRegister = secondByte & 0b00000111;
                    secondRegister = (secondByte >> 3) & 0b00000111;
                }
                else {
                    firstRegister = (secondByte >> 3) & 0b00000111;
                    secondRegister = secondByte & 0b00000111;
                }

                std::string* registerNames = wordData ? regW : regNotW;
                output << registerNames[firstRegister] << ", ";
                output << registerNames[secondRegister];
            }

            output << std::endl;
        }

        it++;
    }
   
    input.close();
    output.close();

    std::cout << "Hello World!\n";

}