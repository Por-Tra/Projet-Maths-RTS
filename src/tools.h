#ifndef TOOLS_H
#define TOOLS_H

#include <iostream>
#include <fstream>


void createCSV(std::ofstream &file, int time, Grid &grid)
{
    int herbivoreCount = 0;

    for (int row = 0; row < grid.getRows(); ++row) 
    {
        for (int col = 0; col < grid.getCols(); ++col) 
        {
            Cell* cell = grid.getCellAt({col, row});
            if (cell && !cell->isEmpty() && dynamic_cast<Herbivore*>(cell->getContent())) 
            {
                herbivoreCount++;
            }
        }
    }

    file << time << "," << herbivoreCount << "\n";
}


#endif // TOOLS_H