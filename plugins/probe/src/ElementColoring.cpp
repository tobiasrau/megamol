/*
 * ElementColoring.cpp
 * Copyright (C) 2021 by MegaMol Team
 * Alle Rechte vorbehalten.
 */

#include "ElementColoring.h"
#include "probe/CallKDTree.h"
#include "probe/ProbeCalls.h"
#include "adios_plugin/CallADIOSData.h"
#include "glm/glm.hpp"
#include "mmcore/param/EnumParam.h"
#include "mmcore/param/FlexEnumParam.h"



namespace megamol {
namespace probe {

ElementColoring::ElementColoring()
    : Module()
    , _version(0)
    , _probe_rhs_slot("getProbes", "")
    , _elements_rhs_slot("getElements", "")
    , _mesh_lhs_slot("deployMesh", "")

    {

    this->_probe_rhs_slot.SetCompatibleCall<probe::CallProbesDescription>();
    this->MakeSlotAvailable(&this->_probe_rhs_slot);
    this->_probe_rhs_slot.SetNecessity(megamol::core::AbstractCallSlotPresentation::SLOT_REQUIRED);

    this->_elements_rhs_slot.SetCompatibleCall<mesh::CallMeshDescription>();
    this->MakeSlotAvailable(&this->_elements_rhs_slot);
    this->_elements_rhs_slot.SetNecessity(megamol::core::AbstractCallSlotPresentation::SLOT_REQUIRED);

    this->_mesh_lhs_slot.SetCallback(
        mesh::CallMesh::ClassName(), mesh::CallMesh::FunctionName(0), &ElementColoring::getData);
    this->_mesh_lhs_slot.SetCallback(
        mesh::CallMesh::ClassName(), mesh::CallMesh::FunctionName(1), &ElementColoring::getMetaData);
    this->MakeSlotAvailable(&this->_mesh_lhs_slot);

}

ElementColoring::~ElementColoring() { this->Release(); }

bool ElementColoring::create() {
    return true;
}

void ElementColoring::release() {}

bool ElementColoring::getData(core::Call& call) {

    auto cp = this->_probe_rhs_slot.CallAs<probe::CallProbes>();
    if (cp == nullptr)
        return false;

    auto celements = this->_elements_rhs_slot.CallAs<mesh::CallMesh>();
    if (celements == nullptr)
        return false;

    mesh::CallMesh* cmesh = dynamic_cast<mesh::CallMesh*>(&call);
    if (cmesh == nullptr)
        return false;

    auto meta_data = cp->getMetaData();

    if (!(*cp)(0)) return false;
    if (!(*celements)(0)) return false;
    const bool cp_dirty = cp->hasUpdate();
    const bool celements_dirty = celements->hasUpdate();
    if (cp_dirty || celements_dirty) {
        ++_version;
        _mesh_collection_copy = *celements->getData();

        // get dimensions for elements
        int shell_max = 0;
        int shell_element_max = 0;
        auto meshes = celements->getData()->accessMeshes();
        for (auto& mesh: meshes) {
            auto tmp_string_vec = this->split(mesh.first, '_');
            auto index_vec = this->split(tmp_string_vec[tmp_string_vec.size()-1], ',');
            auto shell = std::stoi(index_vec[0]);
            auto shell_element = std::stoi(index_vec[1]);
            shell_max = std::max(shell, shell_max);
            shell_element_max = std::max(shell_element, shell_element_max);
        }
        shell_max += 1;
        shell_element_max += 1;

        auto const probe_count = cp->getData()->getProbeCount();
        std::vector<int> cluster_ids(probe_count);
        for (auto i = 0; i < probe_count; i++) {
            auto generic_probe = cp->getData()->getGenericProbe(i);

            int cluster_id;

            auto visitor = [&cluster_id](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, probe::FloatProbe>) {
                    cluster_id = arg.m_cluster_id;
                } else if constexpr (std::is_same_v<T, probe::IntProbe>) {
                    cluster_id = arg.m_cluster_id;
                } else if constexpr (std::is_same_v<T, probe::Vec4Probe>) {
                    cluster_id = arg.m_cluster_id;
                } else if constexpr (std::is_same_v<T, probe::FloatDistributionProbe>) {
                    cluster_id = arg.m_cluster_id;
                } else {
                    // unknown probe type, throw error? do nothing?
                }
            };

            std::visit(visitor, generic_probe);

            cluster_ids[i] = cluster_id;
        }

        auto cluster_ids_sorted = cluster_ids;
        std::sort(cluster_ids_sorted.begin(), cluster_ids_sorted.end());
        auto unique_cluster_ids = cluster_ids_sorted;
        auto non_unique_items = std::unique(unique_cluster_ids.begin(), unique_cluster_ids.end());
        unique_cluster_ids.erase(non_unique_items, unique_cluster_ids.end());
        auto num_clusters = unique_cluster_ids.size();
        int lowes_cluster_id = unique_cluster_ids[0];


        _colors.clear();
        _colors.resize(shell_max);
        _mesh_copy.resize(meshes.size());
        for (int i = 0; i < shell_max; ++i) {
            _colors[i].resize(shell_element_max);
            for (int j = 0; j < shell_element_max; ++j) {
                auto flat_id = shell_max * j;
                std::string access_str = "element_mesh_" + std::to_string(i) + "," + std::to_string(j);
                auto current_id = cluster_ids[j];
                auto current_color = hsvSpiralColor(current_id - lowes_cluster_id, num_clusters); // id can be -1

                int num_verts = 0;
                auto& attribs = celements->getData()->accessMesh(access_str).attributes;
                for (auto& attr :attribs) {
                    if (attr.semantic == mesh::MeshDataAccessCollection::POSITION) {
                        num_verts = attr.byte_size / sizeof(std::array<float, 3>);
                    }
                }

                _colors[i][j].resize(num_verts);
                for (int v = 0; v < num_verts; ++v) {
                    _colors[i][j][v] = {current_color.x, current_color.y, current_color.z, 1.0f};
                }

                auto col_attr = mesh::MeshDataAccessCollection::VertexAttribute();
                col_attr.component_type = mesh::MeshDataAccessCollection::ValueType::FLOAT;
                col_attr.byte_size = _colors[i][j].size() * sizeof(std::array<float, 4>);
                col_attr.component_cnt = 4;
                col_attr.stride = sizeof(std::array<float, 4>);
                col_attr.offset = 0;
                col_attr.data = reinterpret_cast<uint8_t*>(_colors[i][j].data());
                col_attr.semantic = mesh::MeshDataAccessCollection::COLOR;
                // put data into mesh
                _mesh_copy[flat_id] = _mesh_collection_copy.accessMesh(access_str);
                _mesh_copy[flat_id].attributes.emplace_back(col_attr);
                _mesh_collection_copy.deleteMesh(access_str);
                _mesh_collection_copy.addMesh(access_str, _mesh_copy[flat_id].attributes, _mesh_copy[flat_id].indices);
            }
        }
    } 
    cmesh->setData(std::make_shared<mesh::MeshDataAccessCollection>(_mesh_collection_copy), _version);

    return true;
}

bool ElementColoring::getMetaData(core::Call& call) {

    auto cp = this->_probe_rhs_slot.CallAs<probe::CallProbes>();
    if (cp == nullptr) return false;

    auto celements = this->_elements_rhs_slot.CallAs<mesh::CallMesh>();
    if (celements == nullptr)
        return false;

    mesh::CallMesh* cmesh = dynamic_cast<mesh::CallMesh*>(&call);
    if (cmesh == nullptr) return false;

    auto probe_meta_data = cp->getMetaData();
    auto elements_meta_data = celements->getMetaData();
    auto meta_data = cmesh->getMetaData();

    probe_meta_data.m_frame_ID = meta_data.m_frame_ID;
    elements_meta_data.m_frame_ID = meta_data.m_frame_ID;

    if (!(*cp)(1)) return false;
    if (!(*celements)(1)) return false;

    probe_meta_data = cp->getMetaData();
    elements_meta_data = celements->getMetaData();

    meta_data.m_frame_cnt = elements_meta_data.m_frame_cnt;
    meta_data.m_bboxs = elements_meta_data.m_bboxs;

    // put metadata in mesh call
    cmesh->setMetaData(meta_data);

    return true;
}

std::vector<std::string> ElementColoring::split(const std::string text, const char sep) {
    std::vector<std::string> result;
    std::istringstream f(text);
    std::string s;
    while (std::getline(f, s, sep)) {
        result.push_back(s);
    }
    return result;
}

glm::vec3 ElementColoring::hsvSpiralColor(int color_idx, int total_colors) {

    float orbit_cnt = 1.0 + log(total_colors);

    float idx_normalized = float(color_idx) / float(total_colors);

    float h = glm::mod(idx_normalized * orbit_cnt, 1.0f);
    float s = 0.1 + 0.9 * idx_normalized;
    float v = 1.0 - 0.9 * idx_normalized;

    glm::vec3 c = glm::vec3(h, s, v);

    return this->hsv2rgb(c);
}

glm::vec3 ElementColoring::hsv2rgb(glm::vec3 c) {
    // From https://stackoverflow.com/questions/31835027/glsl-hsv-shader
    glm::vec4 K = glm::vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    glm::vec3 p =
        abs(glm::fract(glm::vec3(c.x, c.x, c.x) + glm::vec3(K.x, K.y, K.z)) * 6.0f - glm::vec3(K.w, K.w, K.w));
    glm::vec3 rgb = c.z * glm::mix(glm::vec3(K.x, K.x, K.x), glm::clamp(p - glm::vec3(K.x, K.x, K.x), 0.0f, 1.0f), c.y);
    return rgb;
}

} // namespace probe
} // namespace megamol