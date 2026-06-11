import subprocess
import os
import sys
import argparse

allowed_configurations = ["Debug", "Release"]

def main():

    parser = argparse.ArgumentParser(description='Select options for build')

    parser.add_argument("--config", 
                        dest='config', 
                        default = allowed_configurations[0], 
                        choices = allowed_configurations)


    args = parser.parse_args()

    print("Building libraries...")
    subprocess.run(["cmake",
                    "-S", "."
                    "-B", "."])

    subprocess.run(["cmake",
                    "--build", ".",
                    "--config", args.config])



    print("Compiling shaders..")
    subprocess.run([sys.executable, "compile_shaders.py"], cwd = "bin")





if __name__ == '__main__':
    main()
