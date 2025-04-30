import pandas as pd

first = pd.read_csv("./onesil_first.csv")

second = pd.read_csv("./onesil.csv")

concat = pd.concat([first, second]).drop_duplicates().reset_index(drop=True)  # type: ignore

concat.to_csv("onesil.csv", index=False)
