#include "otp.h"

namespace diffcrypto {
    OTP::OTP(const Key master_key) : master_key(master_key) {
    }

    Block OTP::encrypt(const Block plaintext) const {
        return (plaintext ^ master_key) & 0xFFF;
    }

    Block OTP::decrypt(const Block ciphertext) const {
        return this->encrypt(ciphertext);
    }
}
