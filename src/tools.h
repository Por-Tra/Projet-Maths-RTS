#ifndef TOOLS_H
#define TOOLS_H

#include <fstream>
#include <iostream>

#include "Entity.h"
#include "Grid.h"

//! Deux problemes dans l'ancienne version :
//!  1. la fonction n'etait pas `inline`. Dans un .h inclus par deux .cpp differents,
//!     c'est une violation de l'ODR et une erreur de lien "multiple definition".
//!  2. elle utilisait Grid, Cell et Herbivore sans les inclure : ca ne compilait que
//!     par chance, grace a l'ordre des #include dans main.cpp.

inline void writeCsvHeader(std::ofstream& file)
{
    file << "Time,HerbivoreCount\n";
}

//* Comptage en O(nombre d'entites) au lieu de O(cols * rows) avec un dynamic_cast
//* sur chaque case occupee.
inline void appendCsvLine(std::ofstream& file, int time, const Grid& grid)
{
    file << time << ',' << grid.populationOf(Species::Herbivore) << '\n';
}

#endif // TOOLS_H
