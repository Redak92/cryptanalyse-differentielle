#include "otp.h"

namespace diffcrypto {

    OTP::OTP(uint16_t master_key) : master_key(master_key){}


Block OTP::encrypt(Block plaintext) const {
    return ((plaintext ^ master_key) & 0xFFF);
}

Block OTP::decrypt(Block ciphertext) const {
    return this->encrypt(ciphertext);
}

int OTP::getBlockSize() const {
    return static_cast<int>(BLOCK_BITS);
}
}

