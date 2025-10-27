#This script will compile every shader in shaders folder that does not have ! in front of it

import subprocess
import os
import sys

current_dir = os.path.curdir
shaders_directory = os.path.abspath(current_dir) + "/shaders"

source_files = []
for root, directories, files in os.walk(shaders_directory):
    for file in files:
        if ((file[0]) != '!') and file.endswith(".glsl"):
           filepath = os.path.join(root, file)
           source_files.append(filepath)

destination_files = []
for i in source_files:
    destination_files.append(i.split(".glsl")[0] + ".sshader")

_executable = ""
if sys.platform == "win32":
    _executable = "ShaderCompiler.exe"
else:
    _executable = os.path.abspath(current_dir) + "/" + "ShaderCompiler"

for index in range(len(destination_files)):
    subprocess.run([_executable, "--source-file", source_files[index], "--output-file", destination_files[index]])
    print("Compiling file: " + source_files[index])
