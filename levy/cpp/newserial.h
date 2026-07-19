#include <stdint.h>

class Serial {

    public:

        static void Begin();

        static void Write(const uint8_t byte);

        static uint8_t Available();

        static uint8_t Read();

        static void Close();
}; 
