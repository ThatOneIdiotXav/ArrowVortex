#include "Psych1X.h"

#include <iostream>
#include <json.hpp>
#include <string>

#include "Core/String.h"
#include "Core/Vector.h"
#include "Managers/StyleMan.h"
#include "Simfile/Chart.h"
#include "Simfile/SegmentGroup.h"
#include "Simfile/Segments.h"
#include "Simfile/Simfile.h"
#include "Simfile/Tempo.h"
#include "Simfile/TimingData.h"
#include "System/File.h"

using namespace std;
using json = nlohmann::json;

namespace Vortex {
namespace Fnf {

Psych1X::Song Psych1X::Parse(json json)
{
	Song song = Song();

	// get shit (or use default)
	song.splash_skin = json.contains("splashSkin") ? json["splashSkin"].get<string>() : song.splash_skin;
	song.game_over_char = json.contains("gameOverChar") ? json["gameOverChar"].get<string>() : song.game_over_char;
	song.game_over_sound = json.contains("gameOverSound") ? json["gameOverSound"].get<string>() : song.game_over_sound;
	song.game_over_end = json.contains("gameOverEnd") ? json["gameOverEnd"].get<string>() : song.game_over_end;
	song.game_over_loop = json.contains("gameOverLoop") ? json["gameOverLoop"].get<string>() : song.game_over_loop;
	song.arrow_skin = json.contains("arrowSkin") ? json["arrowSkin"].get<string>() : song.arrow_skin;
	song.disable_note_rgb = json.contains("disableNoteRGB") ? json["disableNoteRGB"].get<bool>() : song.disable_note_rgb;
	song.speed = json.contains("speed") ? json["speed"].get<float>() : song.speed;
	song.stage = json.contains("stage") ? json["stage"].get<string>() : song.stage;
	song.player1 = json.contains("player1") ? json["player1"].get<string>() : song.player1;
	song.player2 = json.contains("player2") ? json["player2"].get<string>() : song.player2;
	song.gf_version = json.contains("gfVersion") ? json["gfVersion"].get<string>() : song.gf_version;
	song.bpm = json.contains("bpm") ? json["bpm"].get<float>() : song.bpm;

	// notes
	for(auto& json_section : json["notes"])
	{
		Section section;

		auto& notes = json_section["sectionNotes"];
		if(!notes.empty())
		{
			for(auto note : notes)
			{
				NoteEntry entry = {
					note[0],
					note[1],
					note[2],
					note.size() == 4 ? note[3] : ""
				};

				section.section_notes.push_back(entry);
			}
		}
		section.section_beats = json_section["sectionBeats"];
		section.gf_section = json_section["gfSection"];
		section.alt_anim = json_section["altAnim"];
		section.must_hit_section = json_section["mustHitSection"];
		section.change_bpm = json_section["changeBPM"];
		section.bpm = json_section["bpm"];

		song.notes->push_back(section);
	}

	// events
	if(!json["events"].empty())
	{
		for(auto& json_events : json["events"])
		{
			Vector<EventGroupEntry::Event> deets;

			for(auto& json_event : json_events[1])
			{
				EventGroupEntry::Event d = {
					json_event[0], json_event[1], json_event[2],
				};
				deets.push_back(d);
			}

			EventGroupEntry event =
			{
				json_events[0],
				deets,
			};
		}
	}

	return song;
}

bool Psych1X::ConvertToSim(StringRef path, Song s, Simfile* sim)
{
	if(s.bpm < 0.0f)
	{
		return false;
	}

	sim->artist = "Unknown";
	sim->title = s.song.c_str();
	sim->format = SIM_FNF_PSYCH1X;
	
	std::string musicPath = GetMusicFile(path);
	sim->music = musicPath.c_str();

	BpmChange initialBpm;
	HudInfo("%f", s.bpm);
	initialBpm.bpm = s.bpm;
	sim->tempo->segments->append(initialBpm);
	sim->tempo->offset = s.offset;

	// notes
	Chart* c = new Chart;
	c->artist = "Unknown";
	c->difficulty = DIFF_EDIT;
	c->meter = 1;
	c->skip_unsorted = true;
	c->style = gStyle->findStyle("dance-double");

	TimingData timing;
	timing.update(sim->tempo);
	TempoRowTracker tracker(timing);

	// forloop sections
	for(int i = 0; i < s.notes->size(); i++)
	{
		Section section = s.notes->at(i);

		// forloop notes within section
		for(NoteEntry& entry : section.section_notes)
		{
			NoteType type = NOTE_STEP_OR_HOLD;
			if(entry.note_type == "Hurt Note") type = NOTE_MINE;

			uint quant = 192;
			int row = tracker.advance(ceill(entry.time) / 1000.l);
			if(entry.sustain_length > 0)
			{
				int endrow = timing.timeToRow(ceill(entry.time) / 1000.l + ceill(entry.sustain_length) / 1000.l);
				c->notes.append({row, endrow, (uint)entry.direction, 0, (uint)type, quant});
			}
			else
				c->notes.append({row, row, (uint)entry.direction, 0, (uint)type, quant});
		}
	}

	sim->charts.push_back(c);

	return true;
}

std::string Psych1X::GetMusicFile(Path path)
{
	if(path.filename().empty()) throw exception("given path is empty");

	// get song title (we'll need it)
	std::string title = path.topdir().str();
	title = title.substr(0, title.size()-1);
	HudInfo("Trying to get Inst.ogg for %s...", title.c_str());
	
	// drop two folders (removing the file too on the first go)
	path.dropFolder(true); // would be [engine root]/assets/[shared (if src)/]data/
	path.dropFolder(); // would be [engine root]/assets/[shared (if src)/]

	if (std::strcmp(path.topdir().str(), "shared\\") == 0)
	{
		HudInfo("shared folder detected - going back one more.");
		// source assets folder, so drop another folder
		path.dropFolder();
	}

	// append the path to reach the Inst.ogg file
	path.push("songs", true);
	path.push(title.c_str(), true);
	path.push("Inst.ogg", false);
	
	HudInfo("Attempting to load song from assumed path: %s", path.str.str());

	// return path to Inst.ogg file
	return path.str.str();
}

bool Psych1X::Load(StringRef path, Simfile* sim)
{
	// parse json
	bool success = false;
	String file = File::getText(path, &success);
	if(!success) throw exception("wasn't able to load the json contents");
	json json_file = json::parse(file.str());

	// parse chart data
	Song song = Parse(json_file);

	return ConvertToSim(path, song, sim);
}

}
}
