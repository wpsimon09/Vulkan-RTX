#!/bin/bash

VULKAN_SDK_PATH="/home/wpsimon09/SDKs/vuklan-sdk/x86_64/bin/glslang"

if [[ ! -x "$VULKAN_SDK_PATH" ]]; then
    echo "Error: Vulkan SDK compiler not found at $VULKAN_SDK_PATH"
    echo "Change VULKAN_SDK_PATH at this file"$PWD/compile.sh
    exit 1
fi

mkdir -p Compiled

for vert_shader in Vertex/*.vert; do
    if [[ -f "$vert_shader" ]]; then
        shader_name=$(basename "$vert_shader")
        echo "Compiling vertex shader: $shader_name"
        $VULKAN_SDK_PATH "$vert_shader" -V -o "Compiled/${shader_name%.vert}.spv"
        if [[ $? -eq 0 ]]; then
            echo "Compiled $vert_shader to Compiled/${shader_name%.vert}"
        else
            echo "Failed to compile $vert_shader"
        fi
    else
        echo "No vertex shaders found in Vertex directory"
    fi
done

for frag_shader in Fragment/*.frag; do
    if [[ -f "$frag_shader" ]]; then
        shader_name=$(basename "$frag_shader")
        echo "Compiling fragment shader: $shader_name"
        $VULKAN_SDK_PATH "$frag_shader" -V -o "Compiled/${shader_name%.frag}.spv"
        if [[ $? -eq 0 ]]; then
            echo "Compiled $frag_shader to Compiled/${shader_name%.frag}.spv"
        else
            echo "Failed to compile $frag_shader"
        fi
    else
        echo "No fragment shaders found in Fragment directory"
    fi
done

for comp_shader in Compute/*.comp; do
    if [[ -f "$comp_shader" ]]; then
        shader_name=$(basename "$comp_shader")
        echo "Compiling compute shader: $shader_name"
        $VULKAN_SDK_PATH "$comp_shader" -V -o "Compiled/${shader_name%.comp}.spv"
        if [[ $? -eq 0 ]]; then
            echo "Compiled $comp_shader to Compiled/${shader_name%.comp}.spv"
        else
            echo "Failed to compile $comp_shader"
        fi
    else
        echo "No fragment shaders found in Fragment directory"
    fi
done