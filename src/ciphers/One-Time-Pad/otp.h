#pragma once

#include "../../interfaces/ICipher.h"

namespace diffcrypto {
    class OTP : public ICipher {
    public:
        explicit OTP(Key master_key);

        [[nodiscard]] Block encrypt(Block plaintext) const override;

        [[nodiscard]] Block decrypt(Block ciphertext) const override;

        [[nodiscard]] int getBlockSize() const override { return 12; }

    private:
        Key master_key;
    };
}
