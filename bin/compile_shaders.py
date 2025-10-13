#This script will compile every shader in shaders folder that does not have ! in front of it

import subprocess
import os

current_dir = os.path.curdir
shaders_directory = os.path.abspath(current_dir) + "\shaders"

source_files = []
for root, directories, files in os.walk(shaders_directory):
    for file in files:
        if ((file[0]) != '!') and file.endswith(".glsl"):
           filepath = os.path.join(root, file)
           source_files.append(filepath)

destination_files = []

for i in source_files:
    destination_files.append(i.split(".glsl")[0] + ".sshader")

for index in range(len(destination_files)):
    command = "ShaderCompiler.exe --source-file " + source_files[index] + " --output-file " + destination_files[index]
    subprocess.run(command)
    print("Compiling file: " + source_files[index])