#include <Dialogs/FnfDefineEngine.h>

#include <Simfile/FNFFormats/List.h>

#include "Editor/Common.h"

namespace Vortex {
namespace Fnf {

DialogDefineEngine::~DialogDefineEngine() = default;

DialogDefineEngine::DialogDefineEngine()
{
	setTitle("DEFINE FNF ENGINE");
	myCreateWidgets();
	onChanges(VCM_ALL_CHANGES);
}

void DialogDefineEngine::myCreateWidgets()
{
	myLayout.row().col(250);
	
	WgDroplist* test = myLayout.add<WgDroplist>("Please select the correct FNF engine.");
	for (auto format : formats)
		test->addItem(format.second);
}

}
}
