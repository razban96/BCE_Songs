# BCE Songs

BCE Songs is a small C++17 utility for building randomized song sets for church services. It maintains a catalog of songs in JSON, then selects songs for a morning sermon and an afternoon sermon while limiting how many songs use the same tonality.

The repository also contains PowerShell helper scripts for turning the existing song presentation files into the initial JSON catalog.

## What the project contains

```text
BCE_Songs/
|-- CMakeLists.txt                 CMake build definition
|-- main.cpp                       C++ program and selection algorithm
|-- songs.json                     Song catalog consumed by the program
|-- Helper/
|   |-- songTitleParses.ps1       Extracts titles from presentation filenames
|   |-- titles.txt                Intermediate list of song titles
|   `-- convertSongsTitlesToJson.ps1
|                                  Converts titles.txt to songs.json
|-- build/                         Local CMake build output and dependencies
`-- README.md                      Project documentation
```

The presentation files are the source material for the catalog, but the C++ application does not read `.ppt` or `.pptx` files. It reads only `songs.json` at runtime. The `build` directory is generated output and normally should not be edited by hand.

## Current catalog

The checked-in catalog contains 589 song records. Most records have a tonality and tags; the `verses` arrays are currently empty. Some records are placeholders with an empty tonality and no tags, such as entries whose title is `"1"` or `"2"`.

The available populated tonalities currently include `Do`, `Re`, `Mi`, `Fa`, `Sol`, and `La`.

## JSON format

`songs.json` is a top-level JSON array. Each array item represents one song and has four properties:

| Property   | Type             | Meaning                                                                                                                         |
| ---------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| `title`    | string           | The song's display title. This is normally derived from a presentation filename with its file extension removed.                |
| `tonality` | string           | The song's key or tonality, for example `"Re"` or `"Sol"`. It may be empty when the song has not been completed in the catalog. |
| `verses`   | array of strings | Bible Verses. The program randomly chooses one string from this array when selecting the song.                                  |
| `tags`     | array of strings | Search or classification labels such as `"Lauda"`, `"Inchinare"`, or `"Inviere"`.                                               |

Example:

```json
{
  "title": "10000 motive",
  "tonality": "Sol",
  "verses": [""],
  "tags": ["Lauda", "Binecuvantare", "Inchinare", "Recunostinta"]
}
```

The C++ loader expects all four properties to exist and to have the types shown above. In particular, `verses` and `tags` must be arrays, even when they are empty:

```json
{
  "title": "Un cantec nou",
  "tonality": "Re",
  "verses": [],
  "tags": []
}
```

## C++ program

### Loading the database

`load_database()` opens the JSON file, parses it with [nlohmann/json](https://github.com/nlohmann/json), and converts every object into a `song` structure:

```cpp
struct song {
	string title;
	string tonality;
	vector<string> verses;
	vector<string> tags;
};
```

The executable loads `songs.json` using a path relative to its current working directory. CMake copies the catalog next to the executable after a successful build so that the normal build output can be run directly.

### Selecting a sermon

`generate_sermon()` receives a mutable song pool, the requested number of songs, a sermon name, and a Mersenne Twister random-number generator. It then:

1. Tracks how many selected songs use each tonality in an `unordered_map`.
2. Walks through the already-shuffled pool.
3. Accepts a song only if its tonality has been selected fewer than five times.
4. Chooses one random string from that song's `verses` array.
5. Adds the song and selected verse to the result.
6. Erases selected songs from the pool so they cannot appear in the next sermon.
7. Stops when the requested count is reached or the pool has been exhausted.

The current `main()` requests:

| Sermon           | Requested songs | Selection order                        |
| ---------------- | --------------: | -------------------------------------- |
| Morning Sermon   |               6 | First selection from the shuffled pool |
| Afternoon Sermon |               7 | Selection from the remaining pool      |

The tonality limit is five songs per tonality within each call to `generate_sermon()`. Because the same tonality counter is local to each call, the morning and afternoon sermons can each contain up to five songs in the same tonality. The two sermons cannot share a song because the morning selections are erased from the shared pool.

The random generator is seeded from `std::random_device`, so a new run normally produces a different order and different verse choices.

### Current implementation notes

The program currently creates `morning_sermon` and `afternoon_sermon` but does not print or save either string. As a result, a successful run currently produces no visible sermon output.

Also, all checked-in `verses` arrays are empty. The selection code assumes each selected song has at least one verse and constructs a random distribution over that array. Before using the generator with the current catalog, verses need to be added or the selection code needs an empty-verse fallback.

If the pool cannot satisfy the requested count while respecting the tonality limit, the function writes a diagnostic to standard error and returns the songs it was able to select.

## PowerShell helpers

The helper scripts are intended to be run from the repository root or from any directory; each script derives the project root from its own location.

### `Helper/songTitleParses.ps1`

This script creates the intermediate title list:

1. Recursively scans Songs Folder for files.
2. Takes each file's base name, removing the `.ppt` or `.pptx` extension.
3. Sorts the titles and removes duplicates.
4. Writes the result as UTF-8 text to `Helper/titles.txt`.

It processes all files under the source folder, including files in subfolders. Because it scans files rather than reading slide contents, the presentation filename is the catalog title. Temporary PowerPoint lock files such as `~$...` can therefore appear as titles if they are present during the scan and may need to be removed from `titles.txt` before conversion.

Run it with:

```powershell
.\Helper\songTitleParses.ps1
```

### `Helper/convertSongsTitlesToJson.ps1`

This script converts `Helper/titles.txt` into the initial `songs.json` catalog:

1. Reads non-empty, trimmed lines from `titles.txt` as song titles.
2. Creates one ordered object per title.
3. Initializes `tonality` to an empty string.
4. Initializes `verses` and `tags` to empty arrays.
5. Serializes the objects as JSON and writes the result to the repository root.

Run it with:

```powershell
.\Helper\convertSongsTitlesToJson.ps1
```

The usual refresh sequence is:

```powershell
.\Helper\songTitleParses.ps1
.\Helper\convertSongsTitlesToJson.ps1
```

After conversion, populate tonalities, verses, and tags in `songs.json` as needed. Re-running the conversion script recreates those fields with empty values, so manual catalog enrichment should be backed up or performed after the conversion step.

## Build and run

Requirements:

- CMake 3.20 or newer
- A C++17 compiler
- PowerShell, only if regenerating the catalog
- Internet access on the first CMake configure, because CMake downloads nlohmann/json 3.11.3

Configure and build with CMake from the repository root:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
```

For the existing Ninja build, the executable is normally:

```powershell
.\build\BCE_Songs.exe
```

If a multi-configuration generator was used, the executable may instead be under a configuration directory such as `build\Debug\BCE_Songs.exe`. Run it from the directory containing the copied `songs.json`, or provide that file in the process working directory, because the program opens the relative path `songs.json`.

## Data flow

```text
PowerPoint filenames
				|
				v
songTitleParses.ps1
				|
				v
Helper/titles.txt
				|
				v
convertSongsTitlesToJson.ps1
				|
				v
songs.json --(CMake copies it beside the executable)-->
				|
				v
main.cpp -> load, shuffle, select morning and afternoon songs
```

The source presentations remain useful as the original song material, while `songs.json` is the application-facing database used by the C++ program.
