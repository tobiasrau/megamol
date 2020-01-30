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
    genericConfig.windowPlacement.w = 1920;
    genericConfig.windowPlacement.h = 1080;
    api->initAPI(&genericConfig);

    megamol::core::MegaMolGraph graph(core, moduleProvider, callProvider, std::move(api), api_name);

	// TODO: verify valid input IDs/names in graph instantiation methods - dont defer validation until executing the changes
    graph.QueueModuleInstantiation("View3D_2", api_name + "::view");
    graph.QueueModuleInstantiation("OSPRayRenderer", api_name + "::rnd");
    graph.QueueModuleInstantiation("AmbientLight", api_name + "::light");
    graph.QueueModuleInstantiation("TestSpheresDataSource", api_name + "::datasource");
    graph.QueueModuleInstantiation("OSPRaySphereGeometry", api_name + "::geo");
    graph.QueueModuleInstantiation("OSPRayOBJMaterial", api_name + "::mat");

    graph.QueueCallInstantiation("CallRender3D_2", api_name + "::view::rendering", api_name + "::rnd::rendering");
    graph.QueueCallInstantiation("MultiParticleDataCall", api_name + "::geo::getdata", api_name + "::datasource::getData");
    graph.QueueCallInstantiation(
        "CallLight", api_name + "::rnd::lights", api_name + "::light::deployLightSlot");
    graph.QueueCallInstantiation("CallOSPRayStructure", api_name + "::rnd::getStructure", api_name + "::geo::deployStructureSlot");
    graph.QueueCallInstantiation(
        "CallOSPRayMaterial", api_name + "::geo::getMaterialSlot", api_name + "::mat::deployMaterialSlot");
    
	graph.ExecuteGraphUpdates();

    std::cout << "Rendering ..." << std::endl;
	while (true) {
        graph.RenderNextFrame();
	}

	// clean up modules, calls, graph

	// TODO: implement graph destructor

    return 0;
}