#include "mmcore/CoreInstance.h"
#include "mmcore/MegaMolGraph.h"

#include "GenericRenderAPI.hpp"

int main() {
    megamol::core::CoreInstance core;
    core.Initialise();

    const megamol::core::factories::ModuleDescriptionManager& moduleProvider = core.GetModuleDescriptionManager();
    const megamol::core::factories::CallDescriptionManager& callProvider = core.GetCallDescriptionManager();

    std::unique_ptr<megamol::render_api::AbstractRenderAPI> api =
        std::make_unique<megamol::render_api::GenericRenderAPI>();
    std::string api_name = "generic";

	megamol::render_api::GenericRenderAPI::Config genericConfig;
    api->initAPI(&genericConfig);

    megamol::core::MegaMolGraph graph(core, moduleProvider, callProvider, std::move(api), api_name);

	// TODO: verify valid input IDs/names in graph instantiation methods - dont defer validation until executing the changes
    graph.QueueModuleInstantiation("View3D", api_name + "::view");
    //graph.QueueModuleInstantiation("SphereOutlineRenderer", api_name + "::spheres");
    //graph.QueueModuleInstantiation("TestSpheresDataSource", api_name + "::datasource");
    //graph.QueueCallInstantiation("CallRender3D", api_name + "::view::rendering", api_name + "::spheres::rendering");
    //graph.QueueCallInstantiation(
    //    "MultiParticleDataCall", api_name + "::spheres::getdata", api_name + "::datasource::getData");

	graph.ExecuteGraphUpdates();

	while (true) {
        graph.RenderNextFrame();
	}

	// clean up modules, calls, graph

	// TODO: implement graph destructor

    return 0;
}