import os
import json

# Define the root directory where you have your subfolders with images
root_dir = "../Fits"  # Adjust to your specific root folder

# Define the subfolders and their corresponding labels manually (as an example)
subfolders = [
    {"folder": "dd", "label": "20O(d,d)"},
    {"folder": "pp", "label": "20O(p,p)"},
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
        images = [
            f"../Fits/{subfolder['folder']}/Outputs/{f}"
            for f in os.listdir(folder_path)
            if f.endswith(".png")
        ]

        if images:
            image_groups[label] = images

# Write the image groups to a JSON file
output_json_path = "./list.json"  # Ensure to save this outside the current folder
with open(output_json_path, "w") as json_file:
    json.dump(image_groups, json_file, indent=4)

print(f"JSON file generated at: {output_json_path}")
