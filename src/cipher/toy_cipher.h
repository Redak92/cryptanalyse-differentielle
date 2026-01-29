#ifndef TOY_CIPHER_H
#define TOY_CIPHER_H

#include "../utils/types.h"

class ToyCipher {
public:
    ToyCipher(Key key, int numRounds = 4);
    
    Block encrypt(Block plaintext);
    
    Block decrypt(Block ciphertext);

    int getNumRounds() const { return numRounds; }

private:
    Key masterKey;
    int numRounds;

    Block feistelRounds(uint16_t initialL, uint16_t initialR, bool isEncryption);

    Block functionF(Block right, uint32_t roundKey);

    static Block sBox(Block value);

    uint32_t deriveRoundKey(int round);
};

#endif
