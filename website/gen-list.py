import os
import json

root_dir = "../Fits"

# JSROOT latest
jsroot = "https://root.cern/js/latest/"
github = "https://loopset.github.io/e796-analysis/"


def attachUrl(url: str | list, path: str, title: str = "") -> str:
    if len(url) == 0:  # empty: return jsroot default web
        return jsroot
    if isinstance(url, str):  # single url
        base = jsroot + "?file=" + path + url
    else:  # a list of urls
        files = f"[{', '.join([repr(path + item) for item in url])}]"
        base = jsroot + "?files=" + files
    # Append options
    options = "&status=size&layout=tabs&title=e796 " + title
    return base + options


# List of subfolders, their labels, and manually specified links
subfolders = [
    {
        "folder": "dd",
        "label": "20O(d,d)",
        "links": [
            {
                "url": "website/RootFiles/dd.root",
                "text": "Global fit and angular distributions",
            },
        ],
    },
    {
        "folder": "pp",
        "label": "20O(p,p)",
        "links": [
            {
                "url": "website/RootFiles/pp.root",
                "text": "Global fit and angular distributions",
            },
        ],
    },
    {
        "folder": "dt",
        "label": "20O(d,t)",
        "links": [
            {
                "url": "website/RootFiles/dt.root",
                "text": "Global fit and angular distributions",
            },
        ],
    },
    {
        "folder": "pd",
        "label": "20O(p,d)",
        "links": [
            {
                "url": "website/RootFiles/pd.root",
                "text": "Global fit and angular distributions",
            },
        ],
    },
    {
        "folder": "Miscellanea",
        "label": "Miscellanea",
        "links": [
            {"url": "", "text": "Go to JSRoot"},
            {"url": "website/RootFiles/sigmas.root", "text": "Sigma study with"},
        ],
    },
]

# Dict to hold structure for web
structure = {}
# Same but holding local paths to files!
localhost = {}

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

    # Modify online
    for link in links:
        for key in link:
            if key == "url":
                link[key] = attachUrl(link[key], github, label)

    # Write to table!
    structure[label] = {"images": images, "links": links}

# Write to two JSON files
with open("./list.json", "w") as json_file:
    json.dump(structure, json_file, indent=4)
