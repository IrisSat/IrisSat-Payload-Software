import os
import subprocess

files = os.listdir("./")
c_files = [file for file in files if ".c" in file]

flags = ["-g","-c","-O0"]
compiler = ["gcc"]



for file in c_files:
    cmd = compiler+flags+ ["-o",file.split(".")[0]+".o",file]
    print(cmd)
    subprocess.run(cmd)
