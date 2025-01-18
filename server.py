import tkinter as tk
import ROOT as r
import subprocess

# Disable rootlogon
r.PyConfig.DisableRootLogon = True

# GUI
root = tk.Tk()

# Server
server = r.THttpServer("http:8080")


# Define buttons
def open(url: str = "http://localhost:8080/currentdir/index.html") -> None:
    subprocess.run(f"xdg-open {url}", shell=True)
    return


# Stop and close
def stop() -> None:
    root.destroy()


# Other settings
root.title("e796 analysis server")
root.configure(bg="lightblue")

# Display Text in the window
text_label = tk.Label(
    root,
    text="e796 analysis server",
    font=("Arial", 20, "bold"),
    bg="lightblue",
    fg="darkblue",
)
text_label.pack(padx=10, pady=20)

# Buttons
button1 = tk.Button(root, text="Open browser", command=open, bg="lightgreen")
button1.pack(pady=5)

close_button = tk.Button(root, text="Close", command=stop, bg="red", fg="white")
close_button.pack(pady=10)

root.mainloop()
