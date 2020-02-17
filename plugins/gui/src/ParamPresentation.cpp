/*
 * PopUp.cpp
 *
 * Copyright (C) 2019 by Universitaet Stuttgart (VIS).
 * Alle Rechte vorbehalten.
 */

#include "stdafx.h"
#include "GUIUtils.h"

#include "vislib/UTF8Encoder.h"

#include <imgui_stdlib.h>
#include <vector>


using namespace megamol::gui;


GUIUtils::GUIUtils(void) : tooltipTime(0.0f), tooltipId(-1), searchFocus(false), searchString() {}


void GUIUtils::HoverToolTip(const std::string& text, ImGuiID id, float time_start, float time_end) {
    assert(ImGui::GetCurrentContext() != nullptr);
    ImGuiIO& io = ImGui::GetIO();

    if (ImGui::IsItemHovered()) {
        bool show_tooltip = false;
        if (time_start > 0.0f) {
            if (this->tooltipId != id) {
                this->tooltipTime = 0.0f;
                this->tooltipId = id;
            } else {
                if ((this->tooltipTime > time_start) && (this->tooltipTime < (time_start + time_end))) {
                    show_tooltip = true;
                }
                this->tooltipTime += io.DeltaTime;
            }
        } else {
            show_tooltip = true;
        }

        if (show_tooltip) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(text.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    } else {
        if ((time_start > 0.0f) && (this->tooltipId == id)) {
            this->tooltipTime = 0.0f;
        }
    }
}


void GUIUtils::HelpMarkerToolTip(const std::string& text, std::string label) {
    assert(ImGui::GetCurrentContext() != nullptr);

    if (!text.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled(label.c_str());
        this->HoverToolTip(text);
    }
}


float GUIUtils::TextWidgetWidth(const std::string& text) const {
    assert(ImGui::GetCurrentContext() != nullptr);
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    ImVec2 pos = ImGui::GetCursorPos();
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.0f);
    ImGui::Text(text.c_str());
    ImGui::PopStyleVar();
    ImGui::SetCursorPos(pos);

    return ImGui::GetItemRectSize().x;
}


bool GUIUtils::Utf8Decode(std::string& str) const {
    vislib::StringA dec_tmp;
    if (vislib::UTF8Encoder::Decode(dec_tmp, vislib::StringA(str.c_str()))) {
        str = std::string(dec_tmp.PeekBuffer());
        return true;
    }
    return false;
}


bool GUIUtils::Utf8Encode(std::string& str) const {
    vislib::StringA dec_tmp;
    if (vislib::UTF8Encoder::Encode(dec_tmp, vislib::StringA(str.c_str()))) {
        str = std::string(dec_tmp.PeekBuffer());
        return true;
    }
    return false;
}

void megamol::gui::GUIUtils::StringSearch(const std::string& label, const std::string& help) {
    ImGuiStyle& style = ImGui::GetStyle();

    std::string help_label = "(?)";

    if (ImGui::Button("Clear")) {
        this->searchString = "";
    }
    ImGui::SameLine();

    // Set keyboard focus when hotkey is pressed
    if (this->searchFocus) {
        ImGui::SetKeyboardFocusHere();
        this->searchFocus = false;
    }

    auto width = ImGui::GetContentRegionAvailWidth() - ImGui::GetCursorPosX() + 4.0f * style.ItemInnerSpacing.x -
                 this->TextWidgetWidth(label + help_label);
    const int min_width = 50.0f;
    width = (width < min_width) ? (min_width) : width;
    ImGui::PushItemWidth(width);

    /// XXX: UTF8 conversion and allocation every frame is horrific inefficient.
    this->Utf8Encode(this->searchString);
    ImGui::InputText("###Search Parameters", &this->searchString, ImGuiInputTextFlags_AutoSelectAll);
    this->Utf8Decode(this->searchString);

    ImGui::PopItemWidth();

    ImGui::SameLine();
    ImGui::Text(label.c_str());
    this->HelpMarkerToolTip(help, help_label);
}
