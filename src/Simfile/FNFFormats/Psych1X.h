#pragma once

#include <json.hpp>

#include "Core/Vector.h"
#include "Simfile/Simfile.h"
using json = nlohmann::json;
using namespace std;

namespace Vortex {
namespace Fnf {

class Psych1X
{

public:
	struct NoteEntry
	{
		float time = 0.f;
		int direction = 0;
		float sustain_length = 0.f;
		string note_type = "";
	};
	struct EventGroupEntry
	{
		struct Event
		{
			string name, param1, param2 = "";
		};
		float time = 0.f; // the time
		Vector<Event> events = Vector<Event>(); // events at that time
	};
	struct Section
	{
		bool gf_section = false; // gfSection: whether or not the opponent notes should be sang by the spectator character
		bool alt_anim = false; // altAnim: whether or not to use the alt animations for characters when singing
		Vector<NoteEntry> section_notes = Vector<NoteEntry>(); // section notes here
		float bpm = 60.f; // bpm: the song's bpm at that section
		int section_beats = 4; // sectionBeats: time measure, i.e. (brackets = thing): [4]/4
		bool change_bpm = false; // changeBPM: should be true if bpm is different and *should* be applied
		bool must_hit_section = false; // mustHitSection: only used for camera point positioning, and does not affect the actual chart
	};
	struct Song
	{
		string splash_skin = "note_splash"; // splashSkin: what skin to use for note splashes
		string game_over_char = "bf-dead"; // gameOverChar: what character to use when failing
		string game_over_sound = "death"; // gameOverSound: what sound file to play when failing
		string game_over_end = "retry"; // gameOverEnd: what sound file to play when restarting after fail
		string game_over_loop = "loop"; // gameOverLoop: what sound file to play when in the fail loop
		string arrow_skin = "note_tex"; // arrowSkin: what skin to use for notes
		bool disable_note_rgb = false; // disableNoteRGB: disable the note colouring shader if true
		float speed = 1.f; // speed: at what speed the notes scroll by
		string stage = "stage"; // stage: what stage to use for the song
		string player1 = "bf"; // player1 (player): what character to use, i.e.: "bf"
		string player2 = "dad"; // player2 (opponent): what character to use, i.e.: "dad"
		Vector<EventGroupEntry>* events = new Vector<EventGroupEntry>(); // events: gee i wonder
		Vector<Section>* notes = new Vector<Section>();// notes: actually sections but okay
		string gf_version = "gf"; // gfVersion (spectator): what character to use, i.e.: "gf"
		const string format = "psych_v1"; // format: chart format (will always be "psych_v1" for this)
		float bpm = -1.f; // bpm: initial bpm at song start
		bool needs_voices = true; // needsVoices: whether or not Voices[-name].ogg files need to be read as well
		string song = ""; // song: the actual song name
		float offset = .0f; // offset: song offset?
	};

	#define CONV_ARGS Song s, Simfile* sim


	static Song Parse(json json);
	static bool ConvertToSim(CONV_ARGS);
	static bool ConvertToFnf(CONV_ARGS);
	static bool Load(StringRef path, Simfile* sim);

private:
};

}
}
