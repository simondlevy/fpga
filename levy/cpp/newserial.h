#include <stdint.h>

class Serial {

    public:

        static void Begin();

        static void WriteByte(const uint8_t byte);

        static uint8_t Available();

        static void Read();

        static uint8_t ReadOne();

        static void Close();
}; 
