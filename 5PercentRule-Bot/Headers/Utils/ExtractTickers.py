import csv

input_csv = "tickers.csv"
output_hpp = "ticker_labels.hpp"

labels = []

with open(input_csv, newline='', encoding="utf-8") as f:
    reader = csv.DictReader(f)
    for row in reader:
        name = row["ticker"].strip()
        if name:
            labels.append(name)

with open(output_hpp, "w", encoding="utf-8") as f:
    f.write("#pragma once\n")
    f.write("""#include <vector>\n#include <string>\n#include "pch.h"\n\n""")
    f.write("inline const std::vector<const char*> TICKER_LABELS = {\n")
    for label in labels:
        label_escaped = label.replace('"', '\\"')
        f.write(f'    "{label_escaped}",\n')
    f.write("};\n")
