#include <iostream>

#include "json.hpp"
#include <string>
#include <Simfile/Simfile.h>

#include "Chart.h"
#include "SegmentGroup.h"
#include "Segments.h"
#include "Tempo.h"
#include "TimingData.h"
#include "Editor/Editor.h"
#include "Managers/StyleMan.h"
#include "System/File.h"

using json = nlohmann::json;

namespace Vortex {
namespace Fnf {

namespace Psych1X {

struct Song
{
	String charter;		// charter -> diff credit
	String artist;		// artist -> artist
	float scrollspeed;	// scrollspeed -> ?
	String stage;		// stage -> ?
	String player1;		// player -> ?
	String player2;		// opponent -> ?
	String gf_version;	// spectator -> ?
	float bpm;			// bpm -> bpm
	String name;		// name -> title

	Vector<Vector<float>> notes; // notes -> simfile notes
	// Vector<Event> events; // ignored.
};

Song Parse(json j)
{
	Song o;

	if(j.contains("charter"))
		o.charter = j["charter"].get<std::string>().data();
	else
		o.charter = "Unknown";

	if(j.contains("artist"))
		o.artist = j["artist"].get<std::string>().data();
	else
		o.artist = "Unknown";

	o.scrollspeed = j["speed"].get<float>();
	o.stage = j["stage"].get<std::string>().data();
	o.player1 = j["player1"].get<std::string>().data();
	o.player2 = j["player2"].get<std::string>().data();
	o.gf_version = j["gfVersion"].get<std::string>().data();
	o.bpm = j["bpm"].get<float>();
	o.name = j["song"].get<std::string>().data();

	std::cout << "Parsed metadata for " << o.name.str() << "\n";

	for(auto& s : j["notes"])
	{
		auto& n = s["sectionNotes"];
		if(n.empty()) continue; // check for no notes in section
		// int b = s["sectionBeats"].get<int>();

		for(auto note : n)
		{
			float time = note[0].get<float>();
			int dir = note[1].get<int>();
			float slen = note[2].get<float>();

			Vector<float> no;
			no.push_back(time);
			no.push_back(dir);
			no.push_back(slen);

			o.notes.push_back(no);

			std::cout << "[ note ] time: " << time << " * dir: " << dir << " * suslen: " << slen << "\n";
		}
	}

	return o;
}

static bool LessThan(const Vector<float> a, const Vector<float> b)
{
	if(a[0] != b[0]) return a[0] < b[0];  // NOLINT(clang-diagnostic-float-equal)
	return a[0] < b[0];
}

/**
 * Loads a Psych Engine, version 1.X, formatted file.
 * @param json_file A parsed .json file
 * @param sim A pointer to a Simfile, used for the Editor itself
 * @return Whether or not the conversion process succeeded
 */
bool Load(json json_file, Simfile* sim)
{
	Song song = Parse(json_file["song"]);

	sim->artist = song.artist;
	sim->title = song.name;
	sim->format = SIM_FNF_PSYCH1X;
	sim->genre = "FNF";
	// TODO: find song automatically (or implement multi-file playback)
	sim->music = R"(D:\Games\FNF\Engine - Psych\assets\songs\milf\Inst.ogg)";

	BpmChange initial;
	initial.bpm = song.bpm;
	sim->tempo->segments->append(initial);

	// notes
	Chart* c = new Chart;
	c->artist = song.charter;
	c->difficulty = DIFF_EDIT;
	c->meter = 1;
	c->style = gStyle->findStyle("dance-couple", 8, 1);

	/*if (!std::is_sorted(song.notes.begin(), song.notes.end(), LessThan))
	{
		std::sort(song.notes.begin(), song.notes.end(), LessThan);
	}*/

	TimingData timing;
	timing.update(sim->tempo);
	TempoRowTracker tracker(timing);

	for(auto& n : song.notes)
	{
		float t = n[0];
		int d = n[1];
		float slen = n[2];

		uint dir = d + 4;

		if(d >= 4 && d <= 7)
		{
			dir = d - 4;
		}

		int quant = 192;
		int row = tracker.advance(t / 1000);
		if(slen > 0)
		{
			int endrow = timing.timeToRow(t / 1000 + slen / 1000);
			c->notes.append({row, endrow, dir, 0, NOTE_STEP_OR_HOLD, (uint)quant});
		}
		else
		{
			c->notes.append({row, row, dir, 0, NOTE_STEP_OR_HOLD, (uint)quant});
		}

		tracker.advance(-t);

	}

	sim->charts.push_back(c);

	return true;
}
}

/**
 * In the case of FNF, this is a wrapper to load other sub-formats.
 * @param path The path to the chart
 * @param sim A pointer to a Simfile
 * @return true if the chart successfully loaded
 */
bool LoadFnf(StringRef path, Simfile* sim)
{
	HudInfo("Support for Friday Night Funkin' charts is experimental!\nExpect bugs or non-functional stuff.");

	// Read JSON file.
	bool success = false;
	String file = File::getText(path, &success);
	json data = json::parse(file.str());
	if(!success)
	{
		HudError("An error occured while loading the JSON file.");
		return false;
	}

	// TODO: handle other formats that Psych 1.X
	return Psych1X::Load(data, sim);

	// return true;
}

}
}
