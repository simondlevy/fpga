#include <stdint.h>

class Serial {

    public:

        static uint8_t Available();

        static void Begin();

        static void Close();

        static uint32_t GetBaudRate();

        static uint8_t Read();

        static void ReadBuffer();

        static void Write(const uint8_t byte);
}; 
