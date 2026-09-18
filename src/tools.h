#ifndef TOOLS_H
#define TOOLS_H

#include <fstream>
#include <iostream>

#include "Entity.h"
#include "Grid.h"

inline void writeCsvHeader(std::ofstream& file)
{
    file << "Time,HerbivoreCount\n";
}

inline void appendCsvLine(std::ofstream& file, int time, const Grid& grid)
{
    file << time << ',' << grid.populationOf(Species::Herbivore) << '\n';
}

#endif // TOOLS_H
