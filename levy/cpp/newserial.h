#include <stdint.h>

class Serial {

    public:

        static void Begin();

        static void WriteByte(const uint8_t byte);

        static void Read();

        static void Close();
}; 
