#pragma once
#include "../../interfaces/ICipher.h"


namespace diffcrypto
{
 class OTP : public ICipher {
    public:
        explicit OTP(uint16_t master_key);

        Block encrypt(Block plaintext) const override;
        Block decrypt(Block ciphertext) const override;
        int getBlockSize() const override;

    private:
        static constexpr uint16_t BLOCK_BITS = 12;
        uint16_t master_key;
    };

}