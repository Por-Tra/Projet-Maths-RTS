import sys
import csv
import matplotlib.pyplot as plt

def read_csv(path):
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        fieldnames = reader.fieldnames
        data = {name: [] for name in fieldnames}
        for row in reader:
            for name in fieldnames:
                data[name].append(float(row[name]))
    return fieldnames, data

def plot(fieldnames, data, output_path):
    x_field = fieldnames[0]
    y_fields = fieldnames[1:]

    plt.figure(figsize=(10, 6))
    for field in y_fields:
        plt.plot(data[x_field], data[field], label=field)

    plt.xlabel(x_field)
    plt.ylabel("Valeur")
    plt.title("Évolution de la simulation")
    plt.legend()
    plt.grid(True)
    plt.savefig(output_path)

def main():
    if len(sys.argv) < 2:
        print("Usage: python plot_simulation.py <input.csv> <output.png>")
        sys.exit(1)

    csv_path = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) > 2 else "data_plot.png"

    fieldnames, data = read_csv(csv_path)
    plot(fieldnames, data, output_path)

if __name__ == "__main__":
    main()