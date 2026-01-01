param(
    [Parameter(Mandatory=$true)]
    [string]$Arch,
    
    [Parameter(Mandatory=$true)]
    [string]$Binary
)

$ErrorActionPreference = "Stop"

Write-Output "=== fuckfile Test - Windows $Arch ==="
Write-Output "Date: $(Get-Date)"
Write-Output "Binary: $Binary"
Write-Output ""

# Test 1: Help
Write-Output "Test 1: Display help"
& ".\$Binary" | Write-Output
Write-Output ""

# Test 2: Single file
Write-Output "Test 2: Fuck single file (preserve size & time)"
"Hello World" | Out-File -FilePath test1.txt -Encoding ASCII -NoNewline
$OrigSize = (Get-Item test1.txt).Length
$OrigTime = (Get-Item test1.txt).LastWriteTime
& ".\$Binary" -f test1.txt | Write-Output
$NewSize = (Get-Item test1.txt).Length
$NewTime = (Get-Item test1.txt).LastWriteTime
$TimePreserved = if ($OrigTime -eq $NewTime) { "YES" } else { "NO" }
Write-Output "Original size: $OrigSize, New size: $NewSize"
Write-Output "Time preserved: $TimePreserved"
Write-Output ""

# Test 3: File with truncate
Write-Output "Test 3: Fuck file with truncate (-s)"
"Test content" | Out-File -FilePath test2.txt -Encoding ASCII -NoNewline
& ".\$Binary" -f -s test2.txt | Write-Output
$NewSize = (Get-Item test2.txt).Length
Write-Output "New size: $NewSize (should be 0)"
Write-Output ""

# Test 4: Aggressive mode
Write-Output "Test 4: Aggressive mode (-a)"
"Aggressive test" | Out-File -FilePath test3.txt -Encoding ASCII -NoNewline
$OrigTime = (Get-Item test3.txt).LastWriteTime
Start-Sleep -Seconds 1
& ".\$Binary" -f -a test3.txt | Write-Output
$NewTime = (Get-Item test3.txt).LastWriteTime
$NewSize = (Get-Item test3.txt).Length
$TimeChanged = if ($OrigTime -ne $NewTime) { "YES" } else { "NO" }
$SizeZero = if ($NewSize -eq 0) { "YES" } else { "NO" }
Write-Output "Time changed: $TimeChanged"
Write-Output "Size is 0: $SizeZero"
Write-Output ""

# Test 5: Dry run
Write-Output "Test 5: Dry run mode (-d)"
"Dry run test" | Out-File -FilePath test4.txt -Encoding ASCII -NoNewline
$OrigContent = Get-Content test4.txt -Raw
& ".\$Binary" -f -d test4.txt | Write-Output
$NewContent = Get-Content test4.txt -Raw
$ContentUnchanged = if ($OrigContent -eq $NewContent) { "YES" } else { "NO" }
Write-Output "Content unchanged: $ContentUnchanged"
Write-Output ""

# Test 6: Recursive directory
Write-Output "Test 6: Recursive directory (-r)"
if (Test-Path testdir) { Remove-Item testdir -Recurse -Force }
New-Item -ItemType Directory -Force -Path testdir\subdir | Out-Null
"File 1" | Out-File -FilePath testdir\file1.txt -Encoding ASCII -NoNewline
"File 2" | Out-File -FilePath testdir\file2.txt -Encoding ASCII -NoNewline
"File 3" | Out-File -FilePath testdir\subdir\file3.txt -Encoding ASCII -NoNewline
& ".\$Binary" -f -r testdir | Write-Output
Write-Output "Files processed:"
Get-ChildItem -Path testdir -Recurse -File | ForEach-Object {
    Write-Output "  $($_.FullName): $($_.Length) bytes"
}
Write-Output ""

# Test 7: No preserve time (-t)
Write-Output "Test 7: No preserve time (-t)"
"Time test" | Out-File -FilePath test5.txt -Encoding ASCII -NoNewline
$OrigTime = (Get-Item test5.txt).LastWriteTime
Start-Sleep -Seconds 1
& ".\$Binary" -f -t test5.txt | Write-Output
$NewTime = (Get-Item test5.txt).LastWriteTime
$TimeChanged = if ($OrigTime -ne $NewTime) { "YES" } else { "NO" }
Write-Output "Time changed: $TimeChanged"
Write-Output ""

Write-Output "=== All tests completed ==="