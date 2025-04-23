import tkinter as tk
from tkinter import ttk
from numpy import pad
import pandas as pd
import uncertainties as un
import math


class App:
    def __init__(self, root: tk.Tk, df: pd.DataFrame, file: str) -> None:
        self.root = root
        self.df = df
        self.file = file
        self.types = ["Binary", "Multi", "Other"]
        self.status = [True, False]

        # First unlabelled entry
        self.index = df[df["type"].isna() | (df["type"] == "")].index.min()
        if pd.isna(self.index):
            self.index = 0  # start from the beginning

        # Variables from tk
        self.info_var = tk.StringVar(value="")
        self.type_var = tk.StringVar(value=self.types[0])
        self.status_var = tk.StringVar(value="False")
        self.stats_val = tk.StringVar(value="")

        self._create_ui()

    def _create_ui(self) -> None:
        """Init UI structure"""
        # Style
        style = ttk.Style()
        style.theme_use("clam")

        # Set root grid layout
        self.root.grid_rowconfigure(0, weight=1)
        self.root.grid_rowconfigure(1, weight=1)
        self.root.grid_rowconfigure(2, weight=1)
        self.root.grid_rowconfigure(3, weight=1)
        self.root.grid_columnconfigure(0, weight=1)
        self.root.grid_columnconfigure(1, weight=1)

        # Title
        tk.Label(self.root, textvariable=self.info_var, font=(None, 14, "bold")).grid(
            row=0, column=0, columnspan=2, pady=10, sticky="ew"
        )

        # Type of reaction
        type_frame = tk.Frame(self.root)
        type_frame.grid(row=1, column=0, padx=10, pady=5, sticky="n")
        tk.Label(type_frame, text="Reaction type:", font=(None, 12, "bold")).grid(
            row=0, column=0, padx=10, pady=5, sticky="ew"
        )
        for i, type in enumerate(self.types):
            ttk.Radiobutton(
                type_frame, text=type, variable=self.type_var, value=type
            ).grid(row=1 + i, column=0, padx=5, pady=5, sticky="w")

        ## Status frame
        status_frame = tk.Frame(self.root)
        status_frame.grid(row=1, column=1, padx=10, pady=5, sticky="n")
        tk.Label(status_frame, text="Reconstructed ?", font=(None, 12, "bold")).grid(
            row=0, column=0, padx=10, pady=5, sticky="ew"
        )
        for i, status in enumerate(self.status):
            ttk.Radiobutton(
                status_frame,
                text=str(status),
                variable=self.status_var,
                value=str(status),
            ).grid(row=1 + i, column=0, padx=5, pady=5, sticky="w")

        buttons_frame = tk.Frame(self.root)
        buttons_frame.grid(row=2, column=0, columnspan=2, pady=10)
        ttk.Button(buttons_frame, text="Previous", command=self._previous).pack(
            side="left", padx=5
        )
        ttk.Button(buttons_frame, text="Set", command=self._set).pack(
            side="left", padx=5
        )
        ttk.Button(buttons_frame, text="Next", command=self._next).pack(
            side="left", padx=5
        )
        ttk.Button(buttons_frame, text="Exit", command=self._exit_app).pack(
            side="left", padx=5
        )

        ## Status frame
        tk.Label(
            self.root, textvariable=self.stats_val, font=(None, 12, "italic")
        ).grid(row=3, column=0, columnspan=2, pady=10, sticky="ew")

        self._update_ui()

    def _update_ui(self) -> None:
        """With info from current entry"""
        run = self.df.at[self.index, "run"]
        entry = self.df.at[self.index, "entry"]
        self.info_var.set(f"Run {run} Entry {entry}")
        type = self.df.at[self.index, "type"]
        if pd.isna(type) or type == "":
            type = "Other"
        self.type_var.set(type)
        status = str(self.df.at[self.index, "status"])
        self.status_var.set(status)
        ## And stats
        self._stats()

    def _set(self) -> None:
        type = self.type_var.get()
        status = self.status_var.get()
        self.df.at[self.index, "type"] = str(self.type_var.get())
        self.df.at[self.index, "status"] = bool(self.status_var.get() == "True")
        self._update_ui()

    def _next(self) -> None:
        self._set()
        if self.index < len(self.df) - 1:
            self.index += 1
            self._update_ui()
        else:
            self._exit_app()

    def _previous(self) -> None:
        if self.index > 0:
            self.index -= 1
            self._update_ui()

    def _stats(self) -> None:
        # Binary events
        binary = self.df[self.df["type"] == "Binary"]
        ok_binary = binary[binary["status"] == True]
        n = un.ufloat(len(ok_binary), math.sqrt(len(ok_binary)))
        ratio_ok = n / len(binary) if len(binary) > 0 else -1  # type: ignore
        ratio_ok *= 100
        ratio_bin = len(binary) / len(self.df)
        ratio_bin *= 100
        # Set stats
        try:
            self.stats_val.set(
                f"OK binaries {len(ok_binary)},  eff : {ratio_ok:.2uS} %\nProcessed {self.index}\nTotal {len(self.df)},  {self.index / len(self.df) * 100:.2f} %"
            )
        except:
            return

    def _exit_app(self) -> None:
        # Save df!
        self.df.to_csv(self.file, index=False)
        self.root.quit()


def main() -> None:
    file = "./onesil.csv"
    print(f"Labelling {file} file...")

    ## Dataframe
    df = pd.read_csv(file, dtype={"type": str})

    # UI
    root = tk.Tk()
    root.title("Reconstruction efficiency")
    app = App(root, df, file)
    root.mainloop()


if __name__ == "__main__":
    main()
