#pragma once

#include <string>

namespace diffcrypto {
    class DifferentialDistributionTable;
    void export_ddt_to_csv(const DifferentialDistributionTable& ddt,
                           const std::string& filename);

} // namespace diffcrypto
