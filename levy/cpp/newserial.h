#include <stdint.h>

class Serial {

    public:

        void Begin();

        void WriteByte(const uint8_t byte);

        void Read();

        void Close();
}; 
