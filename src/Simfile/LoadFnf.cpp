// -- fnf formats --
#include "Simfile/FNFFormats/Psych1X.h"

// -- other includes --
#include <json.hpp>
#include <optional>
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
	json json = json::parse(File::getText(path, &success).str());
	
	// PSYCH 1.X
	if(
		// Older Psych charts had the entire song in an "song" object instead of direct, so we check for that.
		!json["song"].is_object()
		// Psych 1.X charts contains a 'format' value, with it always being "psych_v1" for this format...
		&& json.contains("format")
		// ... so we check for it.
		&& json["format"].get<string>() == "psych_v1")
		return SIM_FNF_PSYCH1X;

	// If none of the checks go thru, simply return "UNKNOWN" and hope to God that shit doesn't hit the fan.
	return SIM_NONE;
}

bool LoadFnf(StringRef path, Simfile* sim, SimFormat format)
{
	if (format == SIM_NONE)
	{
		// TODO: prompt user for "what format" menu
		return false;
	}
	
	bool success = false;
	json json_file = json::parse(File::getText(path, &success).str());

	switch(format)
	{
	case SIM_FNF_PSYCH1X:
		return Psych1X::Load(path, sim);

	default:
		return false;
	}
}

} // namespace Fnf
} // namespace Vortex
