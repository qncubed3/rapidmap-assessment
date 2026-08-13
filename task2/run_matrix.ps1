# Run the pipeline over many settings
# Edit the lists below, then run:
#   .\run_matrix.ps1

# Go to this script's folder
Set-Location $PSScriptRoot

# ---------- settings you can change ----------

# set to $true to only print commands (do not run them)
$DryRun = $false

# feature types to test (add "polygons" later)
$Types = @("polygons")

# how many features
$Counts = @(100000, 500000, 1000000, 5000000, 10000000)

# image size (N x N)
$Dims = @(4096, 8192, 16384)

# extra flags passed to every run.exe (same as solo runs)
$Compact = $false       # --compact
$Optimised1 = $true    # --optimised1 (polygons only; harmless if unused)

# ---------- end of settings ----------

# Build the program first
Write-Host "Building..."
mingw32-make
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed"
    exit 1
}

# Count how many runs we will do
$total = $Types.Count * $Counts.Count * $Dims.Count
$n = 0
$failed = 0

Write-Host "Will run $total jobs"
Write-Host ""

# Triple loop over type, count, and grid size
foreach ($type in $Types) {
    foreach ($count in $Counts) {
        foreach ($dim in $Dims) {
            $n = $n + 1
            $extra = @()
            if ($Compact) { $extra += "--compact" }
            if ($Optimised1) { $extra += "--optimised1" }
            $extraText = if ($extra.Count -gt 0) { " " + ($extra -join " ") } else { "" }

            Write-Host "==== [$n / $total] type=$type count=$count dim=$dim$extraText ===="

            if ($DryRun) {
                Write-Host ".\run.exe --type $type --dim $dim --count $count$extraText"
            }
            else {
                & .\run.exe --type $type --dim $dim --count $count @extra
                if ($LASTEXITCODE -ne 0) {
                    Write-Host "FAILED (exit code $LASTEXITCODE)"
                    $failed = $failed + 1
                }
            }
        }
    }
}

Write-Host ""
Write-Host "Finished. Failed: $failed / $total"
Write-Host "See performance_log.csv for timings"
