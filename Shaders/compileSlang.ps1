s$SLANGC_PATH="C:\SDKs\slang\slangc.exe"  # Update this path to point to the slangc binary

if (!(Test-Path $SLANGC_PATH -PathType Leaf)) {
    Write-Host "Error: Slang compiler not found at $SLANGC_PATH"
    exit 1
}

New-Item -ItemType Directory -Force -Path Compiled | Out-Null

# Compile vertex shaders
Get-ChildItem -Path Vertex\*.vert.slang | ForEach-Object {
    $shaderName = $_.Name
    Write-Host "Compiling vertex shader: $shaderName"
    & $SLANGC_PATH -o "Compiled\$($shaderName -replace '\.vert\.slang$', '.spv')" $_.FullName
}

# Compile fragment shaders
Get-ChildItem -Path Fragment\*.frag.slang | ForEach-Object {
    $shaderName = $_.Name
    Write-Host "Compiling fragment shader: $shaderName"
    & $SLANGC_PATH -allow-glsl -o "Compiled\$($shaderName -replace '\.frag\.slang$', '.spv')" $_.FullName
}

# Compile utility shaders
Get-ChildItem -Path Utils\*.slang | ForEach-Object {
    $shaderName = $_.Name
    Write-Host "Compiling utility shader: $shaderName"
    & $SLANGC_PATH -allow-glsl -o "Compiled\$($shaderName -replace '\.slang$', '.spv')" $_.FullName
}

# Compile compute shaders
Get-ChildItem -Path Compute\*.comp | ForEach-Object {
    $shaderName = $_.Name
    Write-Host "Compiling compute shader: $shaderName"
    & $SLANGC_PATH -o "Compiled\$($shaderName -replace '\.comp$', '.spv')" $_.FullName
}

Write-Host "Compilation complete."
