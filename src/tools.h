#ifndef TOOLS_H
#define TOOLS_H

#include <fstream>
#include <iostream>

#include "Entity.h"
#include "Grid.h"

inline void writeCsvHeader(std::ofstream& file)
{
    file << "Time,HerbivoreCount,Births,Deaths,NetScore\n";
}

inline void appendCsvLine(std::ofstream& file, int time, const Grid& grid)
{
    const long long score = static_cast<long long>(grid.births())
                          - static_cast<long long>(grid.deaths());
    file << time << ',' << grid.herbivorePopulation() << ',' << grid.births() << ','
         << grid.deaths() << ',' << score << '\n';
}

#endif // TOOLS_H
