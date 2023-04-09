#include <iostream>
#include <iterator>
#include <algorithm>
#include <fstream>
#include <string>

int main()
{
    std::ifstream input("resources/test", std::ios::binary);
    std::ofstream output("result.asm");
    
    int16_t registers[8];


    std::string regW[] = {"ax","cx","dx","bx","sp","bp","si","di" };
    std::string regNotW[] = { "al","cl","dl","bl","ah","ch","dh","bh" };

    std::string rmValues[] = { "bx + si","bx + di","bp + si","bp + di","si","di","bp","bx" };


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
       
            output << std::endl;
        }
        // immidiate to register/memory
        else if ((*it & 0b11111110) == 0b1100011)
        {
            char w = *it & 0b00000001;
            bool wordData = w == 0b00000001;

            it++;
            auto secondByte = *it;


        }

        it++;
    }
   
    input.close();
    output.close();

    std::cout << "Hello World!\n";

}