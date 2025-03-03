import tkinter as tk
from tkinter import ttk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
import matplotlib.pyplot as plt
import numpy as np
from numpy.typing import NDArray
from typing import Tuple, Optional
import utils

class Data:
    data : np.ndarray
    labels : np.ndarray
    npadx : int
    npady : int

    def __init__(self, data : np.ndarray, labels : np.ndarray = None):
        self.data = data
        self.labels = labels
        self.npadx = 128
        self.npady = 128

    def get(self, idx : int)-> Tuple[np.ndarray, np.ndarray]:
        if idx < len(self.data):
            dense = self.__toDense(self.data[idx])
            return (dense, self.labels[idx] if self.labels else None)
        else:
            return ()
            
    def __toDense(self, array : np.ndarray) -> np.ndarray:
        ret = np.zeros((self.npadx, self.npady), dtype=int)
        for i in range(len(array)):
            assert array[i].shape[0] == 3, "Data must have 3 columns per entry"
            x, y, q = array[i, 0], array[i, 1], array[i, 2]
            ret[x, y] += q
        return ret

    def __len__(self) -> int:
        return len(self.data)


class MatplotlibGUI:
    root : tk.Tk
    data : Data
    index : int
    def __init__(self, data : Data):
        self.root = tk.Tk()
        self.root.title("ActPy projection plotter")
        self.data = data
        self.index = 0

        # Buttons at the top
        self.buttons_frame = ttk.Frame(self.root)
        self.buttons_frame.pack(side=tk.TOP, fill=tk.X)

        self.prev_button = ttk.Button(self.buttons_frame, text="Back", command=self.show_previous)
        self.prev_button.pack(side=tk.LEFT, padx=5, pady=5)

        self.next_button = ttk.Button(self.buttons_frame, text="Forward", command=self.show_next)
        self.next_button.pack(side=tk.LEFT, padx=5, pady=5)

        # Matplotlib figure and canvas in the middle
        self.fig, self.ax = plt.subplots()
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.root)
        self.canvas_widget = self.canvas.get_tk_widget()
        self.canvas_widget.pack(fill=tk.BOTH, expand=True)

        # Toolbar at the bottom
        self.toolbar = NavigationToolbar2Tk(self.canvas, self.root)
        self.toolbar.update()
        self.toolbar.pack(side=tk.BOTTOM, fill=tk.X)

        # Bind resize event and close events
        self.root.bind("<Configure>", self.on_resize)
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)


        # Initial plot
        self.update_plot()

    def update_plot(self):
        self.ax.clear()
        proj, label = self.data.get(self.index)
        self.ax.imshow(proj.T, origin='lower', cmap='viridis')
        self.ax.set_title(f"Entry : {self.index}")
        self.canvas.draw()

    def show_previous(self):
        self.index = (self.index - 1) % len(self.data)
        self.update_plot()

    def show_next(self):
        self.index = (self.index + 1) % len(self.data)
        self.update_plot()

    def on_resize(self, event):
        # Resize the figure dynamically with the window
        width, height = self.canvas_widget.winfo_width(), self.canvas_widget.winfo_height()
        self.fig.set_size_inches(width / 100, height / 100)  # Match widget size (100 DPI scaling)
        self.canvas.draw()

    def on_close(self):
        plt.close(self.fig)
        self.root.destroy()

    def run(self):
        self.root.mainloop()

if __name__ == "__main__":
    # ## Toy data
    # N = 50
    # random = np.empty(N, dtype=object)
    # for i in range(N):
    #     length = np.random.randint(80, 320)
    #     x = np.random.randint(0, 128, length)
    #     y = np.random.randint(0, 128, length)
    #     q = np.random.randint(1, 5000, length)
    #     random[i] = np.column_stack((x, y, q))
    # data = Data(random)

    data, labels = utils.ReadROOTFile('./dataset.root', 50)

    myData = Data(data)
    app = MatplotlibGUI(myData)
    app.run()