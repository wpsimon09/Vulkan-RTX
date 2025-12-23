import os
import subprocess
import argparse
from rich.console import Console
from rich.text import Text

# Initialize Rich console
console = Console()

# Path to slangc compiler
SLANGC_PATH = "/home/simon/SDKs/slang/bin/slangc"

def has_changed_with_git(path):
    """Check if a file has uncommitted changes in git."""
    try:
        result = subprocess.run(["git", "status", "--porcelain", path], capture_output=True, text=True)
        return result.stdout.strip() != ""
    except Exception:
        return True  # Assume changed if any error occurs

def print_status(msg, status="info"):
    """Print status messages with colors and bullet points."""
    if status == "success":
        console.print(f"• [green]{msg}[/green]")
    elif status == "fail":
        console.print(f"• [red]{msg}[/red]")
    elif status == "skip":
        console.print(f"• [green]{msg}[/green]")
    else:
        console.print(f"• {msg}")

def compile_shader(input_path, command, success_msg, fail_msg, verbose, skip_unchanged):
    """Compile a single shader file."""
    if not has_changed_with_git(input_path) and skip_unchanged:
        print_status(f"Skipped (unchanged): {input_path} (use --no-skip-unchanged to force compile)", "skip")
        return False
    try:
        subprocess.check_output(command, stderr=subprocess.STDOUT)
        print_status(success_msg, "success")
        return True
    except subprocess.CalledProcessError as e:
        print_status(fail_msg, "fail")
        if verbose:
            console.print(f"[red]{e.output.decode()}[/red]")
        return False

def compile_shaders(extension, folder, output_ext, extra_args=None, verbose=True, skip_unchanged=True):
    """Compile all shaders in a folder with a specific extension."""
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
                input_path,
                [SLANGC_PATH, "-o", output_path, input_path] + extra_args,
                f"Compiled: {file}",
                f"Failed: {file}",
                verbose,
                skip_unchanged
            )
    if not found and verbose:
        print_status(f"(No *{extension} files in {folder})")

def compile_bundled_shaders(folder, verbose, skip_unchanged=True):
    """Compile vertex and fragment shaders from .slang files."""
    for file in os.listdir(folder):
        if file.endswith(".slang"):
            path = os.path.join(folder, file)
            name = os.path.splitext(file)[0]
            
            # Vertex shader
            compile_shader(
                path,
                [SLANGC_PATH, "-target", "spirv", "-stage", "vertex", "-I", "Source/Modules", "-entry", "vertexMain",
                 "-o", f"Compiled/{name}.vert.spv", path],
                f"{name}.vert",
                f"Failed: {name}.vert",
                verbose,
                skip_unchanged
            )
            # Fragment shader
            compile_shader(
                path,
                [SLANGC_PATH, "-target", "spirv", "-stage", "fragment", "-I", "Source/Modules", "-entry", "fragmentMain",
                 "-o", f"Compiled/{name}.frag.spv", path],
                f"{name}.frag",
                f"Failed: {name}.frag",
                verbose,
                skip_unchanged
            )

def compile_raytracing_shaders(folder, verbose, skip_unchanged):
    """Compile all ray tracing stages from .slang files."""
    for file in os.listdir(folder):
        if file.endswith(".slang"):
            path = os.path.join(folder, file)
            name = os.path.splitext(file)[0]

            shaders = [
                ("raygeneration", "rayGenMain", "rgen", f"{name}.rgen.spv", f"{name}.raygen"),
                ("miss", "missMain", "miss", f"{name}.miss.spv", f"{name}.miss"),
                ("miss", "missMain2", "miss2", f"{name}.miss2.spv", f"{name}.miss2"),
                ("closesthit", "closestHitMain", "chit", f"{name}.chit.spv", f"{name}.chit")
            ]

            for stage, entry, label, out_file, log_name in shaders:
                compile_shader(
                    path,
                    [SLANGC_PATH, "-target", "spirv", "-stage", stage, "-I", "Source/Modules", "-entry", entry,
                     "-o", f"Compiled/{out_file}", path],
                    f"{log_name}",
                    f"Failed: {log_name}",
                    verbose,
                    skip_unchanged
                )

def main():
    parser = argparse.ArgumentParser(description="Slang Shader Compilation Tool")
    parser.add_argument('--verbose', action='store_true', help='Enable verbose output')

    parser.add_argument(
        '--skip_unchanged',
        dest='skip_unchanged',
        action='store_true',
        help='Skip unchanged files (default: True)'
    )
    parser.add_argument(
        '--no-skip-unchanged',
        dest='skip_unchanged',
        action='store_false',
        help='Force recompilation of all files, even unchanged'
    )
    parser.set_defaults(skip_unchanged=True)

    parser.add_argument('--stage', choices=[
        "vertex", "fragment", "compute", "env", "depth", "postprocessing", "raytracing", "rtshadows", "all"
    ], default="all", help='Which shader stage(s) to compile')

    args = parser.parse_args()

    if not os.path.isfile(SLANGC_PATH) or not os.access(SLANGC_PATH, os.X_OK):
        console.print(f"[red]Error:[/red] slangc not found or not executable at {SLANGC_PATH}")
        return

    os.makedirs("Compiled", exist_ok=True)

    if args.skip_unchanged:
        console.print("[yellow]Skipping unchanged files...[/yellow]")
    else:
        console.print("[yellow]Forcing compilation of all shaders...[/yellow]")

    if args.stage in ("vertex", "all"):
        console.print("[bold cyan]== Vertex Shaders ==[/bold cyan]")
        compile_shaders(".vert.slang", "Source/Vertex", ".spv", [
            "-allow-glsl", "-matrix-layout-column-major", "-I", "Source/Modules"
        ], args.verbose, args.skip_unchanged)

    if args.stage in ("fragment", "all"):
        console.print("[bold cyan]== Fragment Shaders ==[/bold cyan]")
        compile_shaders(".frag.slang", "Source/Fragment", ".spv", [
            "-allow-glsl", "-matrix-layout-column-major", "-I", "Source/Modules"
        ], args.verbose, args.skip_unchanged)

    if args.stage in ("compute", "all"):
        console.print("[bold cyan]== Compute Shaders ==[/bold cyan]")
        compile_shaders(".slang", "Source/Compute", ".spv", [
            "-matrix-layout-column-major", "-I", "Source/Modules"
        ], args.verbose, args.skip_unchanged)

    if args.stage in ("env", "all"):
        console.print("[bold cyan]== Environment Generation ==[/bold cyan]")
        compile_bundled_shaders("Source/EnvGeneration", args.verbose, args.skip_unchanged)

    if args.stage in ("depth", "all"):
        console.print("[bold cyan]== Depth Pre-Pass ==[/bold cyan]")
        compile_bundled_shaders("Source/DepthPrePass", args.verbose, args.skip_unchanged)

    if args.stage in ("postprocessing", "all"):
        console.print("[bold cyan]== Post-Processing ==[/bold cyan]")
        compile_bundled_shaders("Source/PostProcessing", args.verbose, args.skip_unchanged)

    if args.stage in ("raytracing", "all"):
        console.print("[bold cyan]== Ray Tracing ==[/bold cyan]")
        compile_raytracing_shaders("Source/RTX", args.verbose, args.skip_unchanged)

    if args.stage in ("rtshadows", "all"):
        console.print("[bold cyan]== RT Shadows ==[/bold cyan]")
        compile_bundled_shaders("Source/RTShadowPass", args.verbose, args.skip_unchanged)

    console.print("[bold green]Done[/bold green]")

if __name__ == "__main__":
    main()
