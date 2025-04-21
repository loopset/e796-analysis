import tkinter as tk

root = tk.Tk()

# Set font for the specific label widget
label = tk.Label(root, text="Hello, Tkinter!", font=("Helvetica", 10))
label.pack()

root.mainloop()
