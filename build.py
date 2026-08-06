import subprocess
import argparse

import scripts.process_assets

allowed_configurations = ["Debug", "Release"]

def main():

    parser = argparse.ArgumentParser(description='Select options for build')

    parser.add_argument("--config", 
                        dest='config', 
                        default = allowed_configurations[0], 
                        choices = allowed_configurations)


    args = parser.parse_args()

    print("Building binaries...")
    subprocess.run(["cmake",
                    "-S", "."
                    "-B", "."])

    subprocess.run(["cmake",
                    "--build", ".",
                    "--config", args.config])


    print("Processing assets...")
    scripts.process_assets.process_assets()


if __name__ == '__main__':
    main()
