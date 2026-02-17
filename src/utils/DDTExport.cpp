#include "DDTExport.h"
#include "../analysis/naive_analysis.h"
#include <fstream>
#include <iostream>

namespace diffcrypto {

void export_ddt_to_csv(const DifferentialDistributionTable& ddt,
                       const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: could not open " << filename << "\n";
        return;
    }

    file << "delta_in,delta_out,count,probability\n";
    uint64_t dim = ddt.table_dimension();
    for (uint64_t dx = 0; dx < dim; ++dx)
    {
        for (uint64_t dy = 0; dy < dim; ++dy)
        {
            auto cnt = ddt.get_count(dx, dy);
            if (cnt > 0)
            {
                file << dx << "," << dy << "," << cnt << "," 
                     << ddt.get_probability(dx, dy) << "\n";
            }
        }
    }
    file.close();
}

} // namespace diffcrypto
