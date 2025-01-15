import os
import json

root_dir = "../Fits"

subfolders = [
    {"folder": "dd", "label": "20O(d,d)"},
    {"folder": "pp", "label": "20O(p,p)"},
    {"folder": "dt", "label": "20O(d,t)"},
    {"folder": "pd", "label": "20O(p,d)"},
]

# Dictionary to hold the image paths for each group
image_groups = {}

# Loop through each subfolder and fetch .png files
for subfolder in subfolders:
    folder_path = os.path.join(root_dir, subfolder["folder"], "Outputs")
    label = subfolder["label"]

    # Check if the subfolder exists
    if os.path.exists(folder_path):
        # Find all .png files in the folder
        files = [f for f in os.listdir(folder_path) if f.endswith(".png")]
        # Sort from oldest to newest
        files = sorted(
            files, key=lambda f: os.path.getmtime(os.path.join(folder_path, f))
        )
        # Append directory data, relative to github's root dir
        images = [f"Fits/{subfolder['folder']}/Outputs/{f}" for f in files]

        if images:
            image_groups[label] = images

# Write the image groups to a JSON file
output_json_path = "./list.json"  # Ensure to save this outside the current folder
with open(output_json_path, "w") as json_file:
    json.dump(image_groups, json_file, indent=4)

print(f"JSON file generated at: {output_json_path}")
