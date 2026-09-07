$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$build = Join-Path $root 'build'

if (-not $env:AMS2_ROOT) {
    $defaultAms2 = 'C:\Program Files (x86)\Steam\steamapps\common\Automobilista 2'
    if (Test-Path -LiteralPath $defaultAms2) {
        $env:AMS2_ROOT = $defaultAms2
    }
}

cmake -S $root -B $build -A x64
if ($LASTEXITCODE) { throw "CMake configuration failed: $LASTEXITCODE" }

cmake --build $build --config Release
if ($LASTEXITCODE) { throw "Build failed: $LASTEXITCODE" }

ctest --test-dir $build -C Release --output-on-failure
if ($LASTEXITCODE) { throw "Tests failed: $LASTEXITCODE" }
