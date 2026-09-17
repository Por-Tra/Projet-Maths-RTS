
BUILD : 
cmake -B build
cmake --build build 
ou 
cmake --build build --config Release


graph avec python :
python plot_simulation.py simulation_data.csv data_plot.png

Template `run.sh`:
```shell
cmake -B build
cmake --build build

# Path to RTS executable (change if needed)
build/bin/main # add --csv to generate CSV instead of display
```