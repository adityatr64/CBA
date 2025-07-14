# Get script directory (absolute path)
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# Ensure bin/ exists
if (!(Test-Path "$scriptDir/bin")) {
    New-Item -ItemType Directory -Path "$scriptDir/bin" | Out-Null
}

# Build paths
$src     = "$scriptDir/src/kernel.s"
$elf     = "$scriptDir/bin/kernel.elf"
$gba     = "$scriptDir/bin/kernel.gba"
$linker  = "$scriptDir/link.ld"

# Compile and link
arm-none-eabi-gcc -mthumb-interwork -T"$linker" -nostartfiles -nostdlib -o "$elf" "$src"

# Convert to binary
arm-none-eabi-objcopy -O binary "$elf" "$gba"

# HexDump (first 10 lines)
function HexDump ($path) {
    $bytes = [System.IO.File]::ReadAllBytes((Resolve-Path $path))
    for ($i = 0; $i -lt $bytes.Length; $i += 16) {
        $chunk = $bytes[$i..([Math]::Min($i + 15, $bytes.Length - 1))]
        $hex = ($chunk | ForEach-Object { "{0:X2}" -f $_ }) -join ' '
        $ascii = ($chunk | ForEach-Object {
            if ($_ -ge 32 -and $_ -le 126) { [char]$_ } else { '.' }
        }) -join ''
        "{0:X8}  {1,-47}  |{2}|" -f $i, $hex, $ascii
    }
}

HexDump "$gba" | Select-Object -First 10
