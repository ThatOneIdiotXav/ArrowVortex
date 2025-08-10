// -- fnf formats --
#include "Simfile/FNFFormats/Psych1X.h"

// -- other includes --
#include <json.hpp>
#include "System/File.h"

#include "Simfile.h"
#include "Core/Vector.h"

using json = nlohmann::json;
using namespace std;

namespace Vortex {
namespace Fnf {

/**
 * Attempts to automatically detect the chart format, based on a few quirks of each format.
 * @param path The path to the chart to be detected.
 * @return The format, or SM_NONE.
 */
SimFormat DetectFormat(StringRef path)
{
	bool success = false;
	String song = File::getText(path, &success);
	if (!success) throw exception("file was not able to be read (Vortex::Fnf::DetectFormat)");
	json json_file = json::parse(song.str());

	// PSYCH 1.X
	if(
			// The older Psych chart format has the entire song in
			// a "song" object instead of directly available, so we check for that.
		!json_file["song"].is_object()
		// Psych 1.X charts contains a 'format' value, with it always being "psych_v1" for this format...
		&& json_file.contains("format")
		// ... so we check for it.
		&& json_file["format"].get<string>() == "psych_v1")
		return SIM_FNF_PSYCH1X;

	// If none of the checks go thru, simply return "UNKNOWN" and hope to God that shit doesn't hit the fan.
	return SIM_NONE;
}

bool LoadFnf(StringRef path, Simfile* sim, SimFormat format)
{
	switch(format)
	{
	case SIM_FNF_PSYCH1X:
		return Psych1X::Load(path, sim);

	case SIM_NONE: // TODO: prompt user to select engine.
	default: // In case an engine *is* defined, but isn't properly implemented (yet).
		return false;

	}
}

} // namespace Fnf
} // namespace Vortex
