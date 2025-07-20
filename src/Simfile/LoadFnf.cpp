#include <iostream>

#include "json.hpp"
#include "Editor/Common.h"
#include "Editor/Editor.h"
#include "Managers/StyleMan.h"
#include "Simfile/Chart.h"
#include "Simfile/SegmentGroup.h"
#include "Simfile/Segments.h"
#include "Simfile/Simfile.h"
#include "Simfile/Tempo.h"
#include "Simfile/TimingData.h"
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

Song ParseJson(json j)
{
	Song o;

	o.charter = j.contains("charter") ? j["charter"].get<std::string>().data() : "Unknown";
	o.artist = j.contains("artist") ? j["artist"].get<std::string>().data() : "Unknown";
	o.scrollspeed = j.contains("speed") ? j["speed"].get<float>() : 2.0;
	o.stage = j.contains("stage") ? j["stage"].get<std::string>().data() : "stage";
	o.player1 = j.contains("player1") ? j["player1"].get<std::string>().data() : "bf";
	o.player2 = j.contains("player2") ? j["player2"].get<std::string>().data() : "dad";
	o.gf_version = j.contains("gfVersion") ? j["gfVersion"].get<std::string>().data() : "gf";
	o.bpm = j.contains("bpm") ? j["bpm"].get<float>() : -1;
	o.name = j.contains("song") ? j["song"].get<std::string>().data() : "!!**!! UNKNOWN SONG !!**!!";

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
		}
	}

	return o;
}

static bool LessThan(const Vector<float> a, const Vector<float> b)
{
	// [0] = time
	// [1] = direction
	if (a[0] < b[0])
		HudWarning("LessThan sorting happened: %d, %d", a[0], b[0]);
		// std::cout << "LessThan: a: " << a[0] << " b: " << b.data()[0] << " res: " << (a[0] < b[0]) << "\n";
	return a[0] < b[0];
}

/**
 * Loads a Psych Engine, version 1.X, formatted file.
 * @param json_file A parsed .json file
 * @param sim A pointer to a Simfile, used for the Editor itself
 * @return Whether or not the conversion process succeeded
 */
bool Load(StringRef path, json json_file, Simfile* sim)
{
	Song song = ParseJson(json_file["song"]);

	if(!std::is_sorted(song.notes.begin(), song.notes.end(), LessThan))
	{
		std::sort(song.notes.begin(), song.notes.end(), LessThan);
	}

	// check: bpm failed?
	if(song.bpm < 0.0) return false;
	// check: song name failed? (this should never realistically happen)
	if(song.name == "!!**!! UNKNOWN SONG !!**!!") return false;

	sim->artist = song.artist;
	sim->title = song.name;
	sim->format = SIM_FNF_PSYCH1X;
	sim->genre = "FNF";

	// FIXME: not having music be loaded with the chart causes weird off-sync
	// issues, so this should be done before a release happens.
	sim->music = R"(D:\Games\FNF\Engine - Psych\assets\songs\2hot\All.ogg)"; // hardcoded for now lol

	BpmChange initialBpm;
	initialBpm.bpm = song.bpm;
	sim->tempo->segments->append(initialBpm);
	sim->tempo->offset = 0;

	// notes
	Chart* c = new Chart;
	c->artist = song.charter;
	c->difficulty = DIFF_EDIT;
	c->meter = 1;
	// "dance-routine" hides the colors for player nums, so.. yeah.
	// "dance-double" it is.
	c->style = gStyle->findStyle("dance-double");

	TimingData timing;
	timing.update(sim->tempo);
	TempoRowTracker tracker(timing);

	for(auto& n : song.notes)
	{
		const double t = n[0];
		const int d = n[1];
		const double slen = n[2];
		const bool opp = d > 3;

		uint quant = 192;
		int row = tracker.lookAhead(std::ceil(t) / 1000);
		if(slen > 0)
		{
			int endrow = timing.timeToRow(std::ceil(t) / 1000.0 + std::ceil(slen) / 1000.0);
			c->notes.append({row, endrow, static_cast<uint>(opp ? d % 4 : d % 4 + 4), /*(uint)(opp ? 0 : 1)*/0, NOTE_STEP_OR_HOLD, quant, true});
		}
		else
		{
			c->notes.append({row, row, static_cast<uint>(opp ? d % 4 : d % 4 + 4), /*(uint)(opp ? 0 : 1)*/0, NOTE_STEP_OR_HOLD, quant, true});
		}
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
	success = Psych1X::Load(path, data, sim);
	

	return success;
}
}
}
