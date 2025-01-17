import os
import json

root_dir = "../Fits"

# JSROOT latest
jsroot = "https://root.cern/js/"
github = "https://raw.githubusercontent.com/loopset/e796-analysis/main/"
src = "website/RootFiles/"


def attachUrl(url: str) -> str:
    return jsroot + "file?url=" + github + url


# List of subfolders, their labels, and manually specified links
subfolders = [
    {
        "folder": "Miscellanea",
        "label": "Miscellanea",
        "links": [
            {"url": "website/RootFiles/sigmas.root", "text": "Sigma study"},
        ],
    },
    {
        "folder": "dd",
        "label": "20O(d,d)",
        "links": [
            {"url": jsroot, "text": "Go to JSRoot"},
            {"url": src + "ang_dd.root", "text": "Angular distributions"},
        ],
    },
    {
        "folder": "pp",
        "label": "20O(p,p)",
        "links": [],
    },
    {
        "folder": "dt",
        "label": "20O(d,t)",
        "links": [],
    },
    {"folder": "pd", "label": "20O(p,d)", "links": []},
]

# Dictionary to hold the image paths and links for each group
folder_data = {}

# Loop through each subfolder and fetch .png files
for subfolder in subfolders:
    label = subfolder["label"]
    links = subfolder["links"]
    # Process subdirs
    if label != "Miscellanea":
        folder_path = os.path.join(root_dir, subfolder["folder"], "Outputs")

        # Check if the subfolder exists
        if os.path.exists(folder_path):
            # Find all .png files in the folder
            files = [f for f in os.listdir(folder_path) if f.endswith(".png")]
            # Sort from oldest to newest based on modification time
            files = sorted(
                files, key=lambda f: os.path.getmtime(os.path.join(folder_path, f))
            )
            # Create image paths relative to the root directory
            images = [f"Fits/{subfolder['folder']}/Outputs/{f}" for f in files]
    else:  # Withouth pictures
        images = []

    # Modify links
    for link in links:
        for key in link:
            if key == "url":
                link[key] = attachUrl(link[key])
    # Write to table!
    folder_data[label] = {"images": images, "links": links}

# Write the folder data to a JSON file
output_json_path = "./list.json"  # Ensure to save this outside the current folder
with open(output_json_path, "w") as json_file:
    json.dump(folder_data, json_file, indent=4)

print(f"JSON file generated at: {output_json_path}")
