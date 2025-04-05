import os
import subprocess
import argparse

SLANGC_PATH = "/home/wpsimon09/SDKs/slang/bin/slangc" ##path to slanc compiler executable 

def compile_shader(command, success_msg, fail_msg, verbose):
    try:
        subprocess.check_output(command, stderr=subprocess.STDOUT)
        if verbose:
            print(success_msg)
        return True
    except subprocess.CalledProcessError as e:
        print(fail_msg)
        if verbose:
            print(e.output.decode())
        return False

def compile_shaders(extension, folder, output_ext, extra_args=None, verbose=True):
    if extra_args is None:
        extra_args = []
    found = False
    for file in os.listdir(folder):
        if file.endswith(extension):
            found = True
            input_path = os.path.join(folder, file)
            base_name = os.path.splitext(file)[0]
            output_path = os.path.join("Compiled", f"{base_name}{output_ext}")
            compile_shader(
                [SLANGC_PATH, "-o", output_path, input_path] + extra_args,
                f"✓ {file}",
                f"✗ Failed: {file}",
                verbose
            )
    if not found and verbose:
        print(f"(No *{extension} files in {folder})")

def compile_env_shaders(verbose):
    print("== ENV Generation ==")
    for file in os.listdir("EnvGeneration"):
        if file.endswith(".slang"):
            path = os.path.join("EnvGeneration", file)
            name = os.path.splitext(file)[0]
            # Vertex
            compile_shader(
                [SLANGC_PATH, "-target", "spirv", "-stage", "vertex", "-entry", "vertexMain",
                 "-o", f"Compiled/{name}.vert.spv", path],
                f"✓ {name}.vert",
                f"✗ Failed: {name}.vert",
                verbose
            )
            # Fragment
            compile_shader(
                [SLANGC_PATH, "-target", "spirv", "-stage", "fragment", "-entry", "fragmentMain",
                 "-o", f"Compiled/{name}.frag.spv", path],
                f"✓ {name}.frag",
                f"✗ Failed: {name}.frag",
                verbose
            )

def main():
    parser = argparse.ArgumentParser(description="Slang Shader Compiler")
    parser.add_argument('--verbose', action='store_true', help='Enable verbose output')
    args = parser.parse_args()

    if not os.path.isfile(SLANGC_PATH) or not os.access(SLANGC_PATH, os.X_OK):
        print(f"Error: slangc not found at {SLANGC_PATH}")
        return

    os.makedirs("Compiled", exist_ok=True)

    compile_shaders(".vert.slang", "Vertex", ".spv", ["-allow-glsl", "-matrix-layout-column-major"], args.verbose)
    compile_shaders(".frag.slang", "Fragment", ".spv", ["-allow-glsl", "-matrix-layout-column-major"], args.verbose)
    compile_shaders(".comp", "Compute", ".spv", ["-matrix-layout-column-major"], args.verbose)

    compile_env_shaders(args.verbose)

if __name__ == "__main__":
    main()
