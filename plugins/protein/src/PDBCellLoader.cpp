/*
 * PDBCellLoader.cpp
 *
 * Copyright (C) 2019 by University of Stuttgart (VISUS).
 * All rights reserved.
 */


#include "stdafx.h"
#include "PDBCellLoader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "PDBLoader.h"
#include "json.hpp"
#include "mmcore/param/BoolParam.h"
#include "mmcore/param/EnumParam.h"
#include "mmcore/param/FilePathParam.h"
#include "mmcore/param/StringParam.h"
#include "protein_calls/CellDataCall.h"
#include "vislib/sys/Log.h"
#include "vislib/sys/sysfunctions.h"

namespace megamol {
namespace protein {


PDBCellLoader::PDBCellLoader(void)
    : Module()
    , jsonFileSlot("jsonFile", "Contains transformations and molecule infos (usually *.apr.json)")
    , pdbFileSlot("pdbFiles", "Separate each pdb with a \";\". In case of included file names just provide path.")
    , cellDataOutSlot("dataout", "The slot providing the loaded data")
    , transKindSlot("transformation", "What kind of transformation is used in jsonFile")
    , dataHash(0) {

    // jsonFileSlot
    this->jsonFileSlot << new core::param::FilePathParam("");
    this->MakeSlotAvailable(&this->jsonFileSlot);

    // pdbFileSlot
    this->pdbFileSlot << new core::param::StringParam("");
    this->MakeSlotAvailable(&this->pdbFileSlot);

    // transKindSlot
    enum kind { MATRIX, QUATERNION };
    megamol::core::param::EnumParam* transKind = new megamol::core::param::EnumParam(kind::MATRIX);
    transKind->SetTypePair(kind::MATRIX, "MATRIX");
    transKind->SetTypePair(kind::QUATERNION, "QUATERNION");
    this->transKindSlot << transKind;
    this->MakeSlotAvailable(&this->transKindSlot);

    // data out slot for molecular data
    this->cellDataOutSlot.SetCallback(protein_calls::CellDataCall::ClassName(),
        protein_calls::CellDataCall::FunctionName(0), &PDBCellLoader::getData);
    this->cellDataOutSlot.SetCallback(protein_calls::CellDataCall::ClassName(),
        protein_calls::CellDataCall::FunctionName(1), &PDBCellLoader::getExtent);
    this->MakeSlotAvailable(&this->cellDataOutSlot);
}


PDBCellLoader::~PDBCellLoader(void) { this->Release(); }

bool PDBCellLoader::create(void) {
    // nothing to do
    return true;
}

void PDBCellLoader::release(void) {
    // clear data arrays
    this->pdb.clear();
}

bool PDBCellLoader::getExtent(core::Call& call) {
    protein_calls::CellDataCall* dc = dynamic_cast<protein_calls::CellDataCall*>(&call);
    if (dc == NULL) return false;

    this->getData(call);
    return true;
}

bool PDBCellLoader::getData(core::Call& caller) {
    protein_calls::CellDataCall* dc = dynamic_cast<protein_calls::CellDataCall*>(&caller);
    if (dc == NULL) return false;

    // load all transformations
    if (this->jsonFileSlot.IsDirty() || this->pdbFileSlot.IsDirty()) {
        this->jsonFileSlot.ResetDirty();
        this->pdbFileSlot.ResetDirty();


        // clear data arrays
        this->pdb.clear();

        // load json file
        std::ifstream file;
        file.open(this->jsonFileSlot.Param<core::param::FilePathParam>()->Value());
        nlohmann::json content = nlohmann::json::parse(file);

        if (this->transKindSlot.Param<core::param::EnumParam>()->ValueString() == "QUATERNION") {
            for (nlohmann::json::iterator components = content.begin(); components != content.end(); ++components) {
                if (components.key() == "recipe") continue;
                // auto compartments = content.at("compartments");
                if (components->is_object()) {
                    vislib::sys::Log::DefaultLog.WriteInfo("number of recepies: %d", components->size());
                    for (nlohmann::json::iterator it = components->begin(); it != components->end(); ++it) {
                        // pack
                        std::string pack = it.key();
                        vislib::sys::Log::DefaultLog.WriteInfo("packs: %s", it.key().c_str());
                        for (nlohmann::json::iterator iit = it->begin(); iit != it->end(); ++iit) {
                            // interior
                            auto tmp_iit = iit;
                            vislib::sys::Log::DefaultLog.WriteInfo("interior: %s", iit.key().c_str());
                            for (nlohmann::json::iterator iiit = tmp_iit->begin(); iiit != tmp_iit->end(); ++iiit) {
                                // ingredients
                                vislib::sys::Log::DefaultLog.WriteInfo("ingredients: %s", iiit.key().c_str());
                                for (nlohmann::json::iterator iiiit = iiit->begin(); iiiit != iiit->end(); ++iiiit) {
                                    // molecules

                                    std::string key;
                                    std::string structureName;
                                    if (iiiit->is_object()) {
                                        key = pack + iiiit.key();
                                        structureName = iiiit.key();
                                    } else {
                                        continue;
                                    }
                                    vislib::sys::Log::DefaultLog.WriteInfo("molName: %s", key.c_str());
                                    this->data[key].structureName = structureName;
                                    this->data[key].componentName = pack;

                                    // get source entery
                                    auto sourceObj = iiiit->at("source");
                                    if (sourceObj.is_object()) {
                                        auto pdbName = sourceObj.at("pdb");
                                        if (pdbName.is_string()) {
                                            std::string tmp_name;
                                            pdbName.get_to(tmp_name);
                                            if (tmp_name == "3irl") continue;

                                            this->data[key].pdbIdentifier = tmp_name;
                                        }
                                        auto transformObj = sourceObj.at("transform");
                                        if (transformObj.is_object()) {
                                            for (nlohmann::json::iterator trans_it = transformObj.begin();
                                                 trans_it != transformObj.end(); ++trans_it) {

                                                if (trans_it.key() == "translate") {
                                                    auto translateObj = transformObj.at("translate");
                                                    if (translateObj.is_array()) {
                                                        translateObj.get_to(this->data[key].transformTranslate);
                                                    }
                                                }
                                                if (trans_it.key() == "rotate") {
                                                    auto rotateObj = transformObj.at("rotate");
                                                    if (rotateObj.is_array()) {
                                                        rotateObj.get_to(this->data[key].transformRotate);
                                                    }
                                                }
                                                if (trans_it.key() == "center") {
                                                    auto centerObj = transformObj.at("center");
                                                    if (centerObj.is_boolean()) {
                                                        centerObj.get_to(this->data[key].transformCenter);
                                                    }
                                                }
                                            }
                                        }
                                    } else {
                                        vislib::sys::Log::DefaultLog.WriteWarn(
                                            "No \"source\" object found. No name mapping.");
                                    }

                                    // get result entery
                                    auto resObj = iiiit->at("results");
                                    if (resObj.is_array()) {
                                        this->data[key].transformations.reserve(resObj.size());
                                        vislib::sys::Log::DefaultLog.WriteInfo(
                                            "number of transformations: %d", resObj.size());
                                        for (nlohmann::json::iterator arrayIt = resObj.begin(); arrayIt != resObj.end();
                                             ++arrayIt) {
                                            auto& arrays = *arrayIt;
                                            // vislib::sys::Log::DefaultLog.WriteInfo("number of encapsulated arrays:
                                            // %d", arrays.size());
                                            bool posFilled = false;
                                            bool vecFilled = false;
                                            for (nlohmann::json::iterator numberIt = arrays.begin();
                                                 numberIt != arrays.end(); ++numberIt) {
                                                auto& number = *numberIt;
                                                if (number.is_array()) {
                                                    // vislib::sys::Log::DefaultLog.WriteInfo("size: %d",
                                                    // number.size());
                                                    protein_calls::transform tmp_trans;
                                                    if (number.size() > 3) {
                                                        std::array<float, 4> quat;
                                                        number.get_to(quat);

                                                        // if (!(this->data[key].transformRotate[0] == 1.0f &&
                                                        //        this->data[key].transformRotate[1] == 0.0f &&
                                                        //        this->data[key].transformRotate[2] == 0.0f &&
                                                        //        this->data[key].transformRotate[3] == 0.0f)) {
                                                        // std::array<float, 4> globalQuat =
                                                        //    this->data[key].transformRotate;

                                                        // quat[0] = quat[0] * globalQuat[1] +
                                                        //          globalQuat[0] * quat[1] +
                                                        //          quat[2] * globalQuat[3] - quat[3] * globalQuat[2];
                                                        // quat[1] = quat[0] * globalQuat[2] +
                                                        //          globalQuat[0] * quat[2] +
                                                        //          quat[3] * globalQuat[1] - quat[1] * globalQuat[3];
                                                        // quat[2] = quat[0] * globalQuat[3] +
                                                        //          globalQuat[0] * quat[3] +
                                                        //          quat[1] * globalQuat[2] - quat[2] * globalQuat[1];
                                                        // quat[3] =
                                                        //    quat[0] * globalQuat[0] -
                                                        //    (quat[1] * globalQuat[1] + quat[2] * globalQuat[2] +
                                                        //        quat[3] * globalQuat[3]);
                                                        //}

                                                        // suggesting indices qr=0 qi=1 qj=2 qk=3
                                                        // columns first rows later
                                                        float s = 1;
                                                        tmp_trans.v[0][0] =
                                                            1 - 2 * s * (quat[2] * quat[2] + quat[3] * quat[3]);
                                                        tmp_trans.v[0][1] =
                                                            2 * s * (quat[1] * quat[2] - quat[3] * quat[0]);
                                                        tmp_trans.v[0][2] =
                                                            2 * s * (quat[1] * quat[3] + quat[2] * quat[0]);
                                                        tmp_trans.v[1][0] =
                                                            2 * s * (quat[1] * quat[2] + quat[3] * quat[0]);
                                                        tmp_trans.v[1][1] =
                                                            1 - 2 * s * (quat[1] * quat[1] + quat[3] * quat[3]);
                                                        tmp_trans.v[1][2] =
                                                            2 * s * (quat[2] * quat[3] - quat[1] * quat[0]);
                                                        tmp_trans.v[2][0] =
                                                            2 * s * (quat[1] * quat[3] - quat[2] * quat[0]);
                                                        tmp_trans.v[2][1] =
                                                            2 * s * (quat[2] * quat[3] + quat[1] * quat[0]);
                                                        tmp_trans.v[2][2] =
                                                            1 - 2 * s * (quat[1] * quat[1] + quat[2] * quat[2]);
                                                        vecFilled = true;

                                                    } else {
                                                        std::array<float, 3> pos;
                                                        number.get_to(pos);
                                                        tmp_trans.pos[0] =
                                                            pos[0] + this->data[key].transformTranslate[0];
                                                        tmp_trans.pos[1] =
                                                            pos[1] + this->data[key].transformTranslate[1];
                                                        tmp_trans.pos[2] =
                                                            pos[2] + this->data[key].transformTranslate[2];
                                                        posFilled = true;
                                                    }
                                                    if (posFilled && vecFilled) {
                                                        this->data[key].transformations.push_back(tmp_trans);
                                                        posFilled = false;
                                                        vecFilled = false;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else {

            vislib::sys::Log::DefaultLog.WriteInfo("number of recepies: %d", content.size());
            for (nlohmann::json::iterator it = content.begin(); it != content.end(); ++it) {
                auto& item = *it;
                std::string pack = it.key();
                vislib::sys::Log::DefaultLog.WriteInfo("number of molecules: %d", item.size());
                for (nlohmann::json::iterator iit = item.begin(); iit != item.end(); ++iit) {
                    auto& iitem = *iit;
                    std::string key;
                    std::string structureName;
                    if (iit->is_object()) {
                        key = pack + iit.key();
                    } else {
                        continue;
                    }
                    vislib::sys::Log::DefaultLog.WriteInfo("molName: %s", key.c_str());
                    this->data[key].structureName = structureName;
                    this->data[key].componentName = pack;
                    auto resObj = iitem.at("results");
                    if (resObj.is_array()) {
                        this->data[key].transformations.reserve(resObj.size());
                        vislib::sys::Log::DefaultLog.WriteInfo("number of transformations: %d", resObj.size());
                        for (nlohmann::json::iterator arrayIt = resObj.begin(); arrayIt != resObj.end(); ++arrayIt) {
                            auto& arrays = *arrayIt;
                            // vislib::sys::Log::DefaultLog.WriteInfo("number of encapsulated arrays: %d",
                            // arrays.size());
                            bool posFilled = false;
                            bool vecFilled = false;
                            for (nlohmann::json::iterator numberIt = arrays.begin(); numberIt != arrays.end();
                                 ++numberIt) {
                                auto& number = *numberIt;
                                if (number.is_array()) {
                                    // vislib::sys::Log::DefaultLog.WriteInfo("size: %d", number.size());
                                    protein_calls::transform tmp_trans;
                                    if (number.size() > 3) {
                                        std::array<std::array<float, 4>, 4> mx;
                                        number.get_to(mx);
                                        tmp_trans.v[0][0] = mx[0][0];
                                        tmp_trans.v[0][1] = mx[0][1];
                                        tmp_trans.v[0][2] = mx[0][2];
                                        tmp_trans.v[1][0] = mx[1][0];
                                        tmp_trans.v[1][1] = mx[1][1];
                                        tmp_trans.v[1][2] = mx[1][2];
                                        tmp_trans.v[2][0] = mx[2][0];
                                        tmp_trans.v[2][1] = mx[2][1];
                                        tmp_trans.v[2][2] = mx[2][2];
                                        vecFilled = true;

                                    } else {
                                        std::array<float, 3> pos;
                                        number.get_to(pos);
                                        tmp_trans.pos[0] = pos[0];
                                        tmp_trans.pos[1] = pos[1];
                                        tmp_trans.pos[2] = pos[2];
                                        posFilled = true;
                                    }
                                    if (posFilled && vecFilled) {
                                        this->data[key].transformations.push_back(tmp_trans);
                                        posFilled = false;
                                        vecFilled = false;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (this->transKindSlot.Param<core::param::EnumParam>()->ValueString() == "QUATERNION") {
            std::string path(std::string(pdbFileSlot.Param<core::param::StringParam>()->Value()));

            for (auto mol : this->data) {
                std::stringstream fullFile;
                std::string identifier = mol.second.pdbIdentifier;
                if (identifier.find("pdb") == std::string::npos) {
                    identifier = identifier + ".pdb";
                }
                fullFile << path << "\\" << identifier;
                this->data[mol.first].pdbFile = fullFile.str();
            }
        } else {

            std::istringstream pdbs(std::string(pdbFileSlot.Param<core::param::StringParam>()->Value()));
            std::string singleFile;

            vislib::sys::Log::DefaultLog.WriteError("should be fixed first");
            return false;

            // TODO:
            // while (std::getline(pdbs, singleFile, ';')) {
            //    this->data[singleFile] = this->getTransformationKey(singleFile);
            //}
        }


        if (!this->data.empty()) {
            // TODO test if this is really a valid file!
            // for each line in the file, try to load the pdb file

            std::mt19937 rnd;
            // rnd.seed(std::random_device()());
            rnd.seed();
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);

            for (auto mol : this->data) {
                this->pdb[mol.first] = new PDBLoader();
                this->data[mol.first].call = new protein_calls::MolecularDataCall();
                this->pdb[mol.first]->pdbFilenameSlot.Param<core::param::FilePathParam>()->SetValue(
                    mol.second.pdbFile.c_str(), true);
                this->pdb[mol.first]->strideFlagSlot.Param<core::param::BoolParam>()->SetValue(false, true);
                this->data[mol.first].call->SetFrameID(0);
                this->data[mol.first].call->SetFrameCount(1);
                this->data[mol.first].color = {dist(rnd), dist(rnd), dist(rnd), 1.0f};

                if (this->pdb[mol.first]->getExtent(*this->data[mol.first].call)) {
                    this->pdb[mol.first]->getData(*this->data[mol.first].call);
                    vislib::sys::Log::DefaultLog.WriteInfo(
                        "Data and extends of molecule %s (file: %s) successfully loaded.", mol.first.c_str(), mol.second.pdbFile.c_str());

                    auto center = this->data[mol.first].call->GetBoundingBoxes().ObjectSpaceBBox().CalcCenter();
                    this->data[mol.first].center = {center.GetX(), center.GetY(), center.GetZ()};
                }
            }
        }

        dc->data = std::make_shared<decltype(this->data)>(this->data);
        this->dataHash++;
    }
    // if (this->pdb.empty()) return false;

    dc->SetDataHash(this->dataHash);

    return true;
}

// std::string PDBCellLoader::getTransformationKey(std::string mol) {

//    std::string molName = mol;
//    const size_t pdbIndex = molName.find_last_of(".");
//    if (pdbIndex != std::numeric_limits<size_t>::max()) {
//        molName = mol.substr(0, pdbIndex);
//    }
//    const size_t directoryIndex = molName.find_last_of("\\");
//    if (directoryIndex != std::numeric_limits<size_t>::max()) {
//        molName = molName.substr(directoryIndex + 1, -1);
//    }

//    std::string transformationKey;
//    for (auto& trans : this->results) {
//        if (trans.first.find(molName) != std::string::npos) {
//            transformationKey = trans.first;
//            break;
//        }
//    }
//    return transformationKey;
//}


} // namespace protein
} // namespace megamol