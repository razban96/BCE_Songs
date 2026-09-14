#include "json.hpp"
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;
using json = nlohmann::json;

// defines the song struct as represented in the JSON file
struct song {
  string title;
  string tonality;
  vector<string> verses;
  vector<string> tags;
};

// load the song database from JSON
vector<song> load_database(const string &filepath) {
  ifstream file(filepath);
  json data = json::parse(file);
  vector<song> database;

  for (const auto &item : data) {
    song s;
    s.title = item.at("title").get<string>();
    s.tonality = item.at("tonality").get<string>();
    s.verses = item.at("verses").get<vector<string>>();
    s.tags = item.at("tags").get<vector<string>>();
    database.push_back(s);
  }
  return database;
}

// generate a sermon by selecting a random subset of songs from the pool
string generate_sermon(vector<song> &songsPool, const int count,
                       const string sermonName, mt19937 &rng) {

  // umap for keeping track of the gamma counts
  const int max_tonality_count = 5;
  unordered_map<string, int> tonality_count;
  vector<pair<song, string>> selected;

  // iterates until song pool finished or selected the songs
  auto it = songsPool.begin();
  while (it != songsPool.end() &&
         selected.size() < static_cast<size_t>(count)) {

    // checks appearances of tonality
    if (tonality_count[it->tonality] < max_tonality_count) {
      tonality_count[it->tonality]++;

      // selects random a verse from the songs' verse pool
      uniform_int_distribution<size_t> verse_dist(0, it->verses.size() - 1);
      string selected_verse = it->verses[verse_dist(rng)];

      selected.push_back({*it, selected_verse});
      it = songsPool.erase(it);
    } else
      ++it;
  }

  if (selected.size() < static_cast<size_t>(count)) {
    cerr << "Insufficient no. of songs in the pool to satisfy tonality "
            "requirement for "
         << sermonName << '\n';
  }
  ostringstream out;
  out << sermonName << '\n';
  for (size_t i = 0; i < selected.size(); ++i) {
    out << (i + 1) << ". " << selected[i].first.title << " ("
        << selected[i].first.tonality << ")\n"
        << "  - " << selected[i].second << '\n';
  }
  return out.str();
}
int main() {
  random_device rd;
  mt19937 rng(rd());

  // load the song database and shuffle it
  vector<song> pool = load_database("songs.json");
  shuffle(pool.begin(), pool.end(), rng);

  const int morning_songs_count = 6;
  const int afternoon_songs_count = 7;

  // generate the morning and afternoon sermons
  string morning_sermon =
      generate_sermon(pool, morning_songs_count, "Morning Sermon", rng);
  string afternoon_sermon =
      generate_sermon(pool, afternoon_songs_count, "Afternoon Sermon", rng);

  return 0;
}
