# defines the project folders
$projectRoot = Split-Path -Parent $PSScriptRoot
$songsFolder = Join-Path $projectRoot "Cantari tineret"
$titlesFile = Join-Path $PSScriptRoot "titles.txt"

# for each song file, inclduging subfolders, it strips the extension and saves the unique titles to a file
Get-ChildItem -Path $songsFolder -File -Recurse |
	ForEach-Object { $_.BaseName -replace '(?i)\.pptx?$', '' } |
	Sort-Object -Unique |
	Set-Content -Path $titlesFile -Encoding utf8
