#!/bin/bash

SLANGC_PATH="/home/wpsimon09/SDKs/slang/bin/slangc"  # Update this path to point to the slangc binary

if [[ ! -x "$SLANGC_PATH" ]]; then
    echo "Error: Slang compiler not found at $SLANGC_PATH"
    echo "Change SLANGC_PATH in this file: $PWD/compile.sh"
    exit 1
fi

mkdir -p Compiled

for vert_shader in Vertex/*.vert.slang; do
    if [[ -f "$vert_shader" ]]; then
        shader_name=$(basename "$vert_shader")
        echo "Compiling vertex shader: $shader_name"
        $SLANGC_PATH -o "Compiled/${shader_name%.vert}.spv" "$vert_shader" -allow-glsl -matrix-layout-column-major 
        if [[ $? -eq 0 ]]; then
            echo "Compiled $vert_shader to Compiled/${shader_name%.vert}.spv"
        else
            echo "Failed to compile $vert_shader"
        fi
    else
        echo "No vertex shaers found in Vertex directory"
    fi
done

for frag_shader in Fragment/*.frag.slang; do
    if [[ -f "$frag_shader" ]]; then
        shader_name=$(basename "$frag_shader")
        echo "Compiling fragment shader: $shader_name"
        $SLANGC_PATH -o "Compiled/${shader_name%.frag}.spv" "$frag_shader" -allow-glsl -matrix-layout-column-major
        if [[ $? -eq 0 ]]; then
            echo "Compiled $frag_shader to Compiled/${shader_name%.frag}.spv"
        else
            echo "Failed to compile $frag_shader"
        fi
    else
        echo "No fragment shaders found in Fragment directory"
    fi
done

for util_shader in Utils/*.slang; do
    if [[ -f "$util_shader" ]]; then
        shader_name=$(basename "$util_shader")
        echo "Compiling Utility shader: $shader_name"
        $SLANGC_PATH -o "Compiled/${shader_name%}.spv" "$frag_shader" -allow-glsl -matrix-layout-column-major  
        if [[ $? -eq 0 ]]; then
            echo "Compiled $util_shader to Compiled/${shader_name%.frag}.spv"
        else
            echo "Failed to compile $util_shader"
        fi
    else
        echo "No utils shaders found in Utils directory"
    fi
done

for comp_shader in Compute/*.comp; do
    if [[ -f "$comp_shader" ]]; then
        shader_name=$(basename "$comp_shader")
        echo "Compiling compute shader: $shader_name"
        $SLANGC_PATH -o -matrix-layout-column-major "Compiled/${shader_name%.comp}.spv" "$comp_shader"
        if [[ $? -eq 0 ]]; then
            echo "Compiled $comp_shader to Compiled/${shader_name%.comp}.spv"
        else
            echo "Failed to compile $comp_shader"
        fi
    else
        echo "No compute shaders found in Compute directory"
    fi
done

echo "==============Compiling ENV================"
for slang_shader in EnvGeneration/*.slang; do
    if [[ -f "$slang_shader" ]]; then
        shader_name=$(basename "$slang_shader" .slang)
        echo "Compiling shaders from: $shader_name.slang"

        # Compile vertex shader
        $SLANGC_PATH -target spirv -stage vertex -entry vertexMain -o "Compiled/${shader_name}.vert.spv" "$slang_shader"
        if [[ $? -eq 0 ]]; then
            echo "Compiled vertex shader to Compiled/${shader_name}.vert.spv"
        else
            echo "Failed to compile vertex shader from $slang_shader"
        fi

        # Compile fragment shader
        $SLANGC_PATH -target spirv -stage fragment -entry fragmentMain -o "Compiled/${shader_name}.frag.spv" "$slang_shader"
        if [[ $? -eq 0 ]]; then
            echo "Compiled fragment shader to Compiled/${shader_name}.frag.spv"
        else
            echo "Failed to compile fragment shader from $slang_shader"
        fi
    else
        echo "No Slang shaders found in Shaders directory"
    fi
done