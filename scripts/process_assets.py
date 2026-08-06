import subprocess
import os
import sys
from pathlib import Path
import tomllib
from concurrent.futures import ThreadPoolExecutor

def build_argumens_shader(input_file: str, assets_folder: str, bin_folder: str) -> list:

    #skip any "include" folders where are .slang files
    file_path = Path(input_file).resolve()
    if file_path.parent.name == "include": 
        return []


    relative_path = Path()
    try:
        relative_path = file_path.relative_to(assets_folder)
    except ValueError:
        raise Exception("Input file: {} must be relative to assets folder: {}".format(input_file, assets_folder))

    output_file = Path(bin_folder).resolve()
    output_file = output_file.joinpath(relative_path.with_suffix(".sshader"))

    #Create directories if they dont already exist
    output_file.parent.mkdir(parents=True, exist_ok=True)

    arguments = ["shader"]
    arguments.extend(["--input-file", input_file])
    arguments.extend(["--output-file", str(output_file)])

    return arguments

def build_argumens_geometry(input_file: str, assets_folder: str, bin_folder: str) -> list:

    file_path = Path(input_file).resolve()

    relative_path = Path()
    try:
        relative_path = file_path.relative_to(assets_folder)
    except ValueError:
        raise Exception("Input file: {} must be relative to assets folder: {}".format(input_file, assets_folder))

    vertex_formats_file_name = "vertex_templates.toml"
    vertex_templates_file = Path(assets_folder).resolve().joinpath(vertex_formats_file_name)
    if vertex_templates_file.exists(follow_symlinks=False) == False:
        raise Exception("File {} must be inside {} !".format(vertex_formats_file_name, assets_folder))

    config_file_path = file_path.parent.joinpath("asset-pipeline.toml")
    if file_path.exists() == False:
        raise Exception("Configuration file {} does not exist but it must be provided!".format(config_file_path))

    
    with open(config_file_path, "rb") as f:
        configuration_file = tomllib.load(f)

    template = str()
    try:
        conf_geometry_template = "geometry-template"
        template = configuration_file[conf_geometry_template]
    except Exception:
        raise Exception("Configuration file {} must have a {} parameter set!".format(config_file_path, conf_geometry_template))
    
    output_file = Path(bin_folder).resolve()
    output_file = output_file.joinpath(relative_path.with_suffix(".sgeometry"))

    #Create directories if they dont already exist
    output_file.parent.mkdir(parents=True, exist_ok=True)

    arguments = ["geometry"]
    arguments.extend(["--input-file", input_file])
    arguments.extend(["--output-file", str(output_file)])
    arguments.extend(["--templates-file="+str(vertex_templates_file)])
    arguments.extend(["--template=" + str(template)])
    
    return arguments

def build_arguments_text(input_file: str, assets_folder: str, bin_folder: str) -> list:

    file_path = Path(input_file).resolve()

    relative_path = Path()
    try:
        relative_path = file_path.relative_to(assets_folder)
    except ValueError:
        raise Exception("Input file: {} must be relative to assets folder: {}".format(input_file, assets_folder))

    output_file = Path(bin_folder).resolve()
    output_file = output_file.joinpath(relative_path.with_suffix(".stext"))
    
    #Create directories if they dont already exist
    output_file.parent.mkdir(parents=True, exist_ok=True)

    arguments = ["text"]
    arguments.extend(["--input-file", input_file])
    arguments.extend(["--output-file", str(output_file)])
    return arguments

def copy_file(input_file: str, assets_folder: str, bin_folder: str) -> None:

    file_path = Path(input_file).resolve()
    
    relative_path = Path()
    try:
        relative_path = file_path.relative_to(assets_folder)
    except ValueError:
        raise Exception("Input file: {} must be relative to assets folder: {}".format(input_file, assets_folder))
    
    output_file = Path(bin_folder).resolve()
    output_file = output_file.joinpath(relative_path)

    #Create directories if they dont already exist
    output_file.parent.mkdir(parents=True, exist_ok=True)

    file_path.copy(output_file)
    return None

def run_pipeline(cmd):
    if "--input-file" in cmd:
        try:
            print("Processing file:", cmd[cmd.index("--input-file")+1])
        except Exception: 
            return None

    return subprocess.run(cmd)

def process_assets():

    scripts_dir = Path(__file__).resolve().parent
    root_dir = scripts_dir.parent
    assets_dir = root_dir.joinpath("assets")
    bin_dir = root_dir.joinpath("bin")
    editor_dir = assets_dir.joinpath("editor")
    executable = bin_dir.joinpath("asset-pipeline")

    if sys.platform == "win32":
        executable.joinpath(".exe")

    if executable.is_file() == False:
        raise Exception("No asset-pipeline in bin folder!")

    vertex_templates = str(assets_dir.joinpath("vertex_templates.toml").resolve())

    excluded_files = [vertex_templates]
    excluded_directories = [
        str(assets_dir.joinpath("vertex_templates.toml").resolve()),
        str(assets_dir.joinpath("global").resolve()),
        str(assets_dir.joinpath("editor-projects").resolve()) 
                            ]
    
    excluded_file_names = ["asset-pipeline.toml"]

    detected_files = []
    for root, directories, files in os.walk(assets_dir):
        if root in excluded_directories: 
            continue
        
        for file in files:
            if not file.startswith("!") and file not in excluded_file_names:
                detected_files.append(os.path.join(root, file))

    for file in excluded_files:
        try:
            detected_files.remove(file)
        except ValueError:
            continue;

    
    arguments = []
    for file in detected_files:

        if file.endswith(".slang"): 
            arg = build_argumens_shader(file, assets_dir, bin_dir) 
            if arg: arguments.append(arg);continue;
        
        if file.endswith(".glb") or file.endswith(".gltf"):
            arg = build_argumens_geometry(file, assets_dir, bin_dir)
            if arg: arguments.append(arg);continue;

        if file.endswith(".toml"):
            arg = build_arguments_text(file, assets_dir, bin_dir)
            if arg: arguments.append(arg);continue;

        if file.endswith((".ttf", ".png", ".hdr")):
            print("Copyting file: {}".format(file))
            copy_file(file, assets_dir, bin_dir); continue;

    for arg in arguments:
        arg.insert(0, executable)

    with ThreadPoolExecutor(max_workers=23) as executor:
        executor.map(run_pipeline, arguments)

    return None

if __name__ == "__main__":
    process_assets()