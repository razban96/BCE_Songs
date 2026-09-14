# defines the project folders
$projectRoot = Split-Path -Parent $PSScriptRoot
$titlesFile = Join-Path $PSScriptRoot "titles.txt"
$songsJsonFile = Join-Path $projectRoot "songs.json"

# reads the titles from the file
$titles = Get-Content -Path $titlesFile -Encoding utf8 | ForEach-Object { $_.Trim() } | Where-Object { $_ -ne "" }

# builds the song collection so more song properties can be added later
$songs = [System.Collections.ArrayList]::new()
foreach ($title in $titles) {
	[void]$songs.Add([ordered]@{
		title = $title
		tonality = ""
		verses = @()
		tags = @()
	})
}

$songs | ConvertTo-Json -Depth 4 | Set-Content -Path $songsJsonFile -Encoding utf8
