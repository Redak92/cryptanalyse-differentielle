#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include "../analysis/naive_analysis.h"

namespace diffcrypto {

void printSeparator();
void printHeader(const char* title);
void printDifferentialPair(const DifferentialPair& pair);
void printDDTSummary(const DifferentialDistributionTable& ddt);

}
