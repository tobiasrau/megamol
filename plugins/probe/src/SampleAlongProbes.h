/*
 * SampleAlongProbes.h
 * Copyright (C) 2009-2017 by MegaMol Team
 * Alle Rechte vorbehalten.
 */


#ifndef SAMPLE_ALONG_PROBES_H_INCLUDED
#define SAMPLE_ALONG_PROBES_H_INCLUDED

#include "mmcore/CalleeSlot.h"
#include "mmcore/CallerSlot.h"
#include "mmcore/Module.h"

#include "ProbeCollection.h"
#include "mmcore/param/ParamSlot.h"
#include "kdtree.h"
#include "mmcore/param/IntParam.h"
#include "adios_plugin/CallADIOSData.h"

namespace megamol {
namespace probe {

class SampleAlongPobes : public core::Module {
public:
    /**
     * Answer the name of this module.
     *
     * @return The name of this module.
     */
    static const char* ClassName() { return "SampleAlongProbes"; }

    /**
     * Answer a human readable description of this module.
     *
     * @return A human readable description of this module.
     */
    static const char* Description() { return "..."; }

    /**
     * Answers whether this module is available on the current system.
     *
     * @return 'true' if the module is available, 'false' otherwise.
     */
    static bool IsAvailable(void) { return true; }

    SampleAlongPobes();
    virtual ~SampleAlongPobes();

protected:
    virtual bool create();
    virtual void release();

    core::CalleeSlot _probe_lhs_slot;

    core::CallerSlot _probe_rhs_slot;
    size_t _probe_cached_hash;

    core::CallerSlot _adios_rhs_slot;
    size_t _adios_cached_hash;

    core::CallerSlot _full_tree_rhs_slot;
    size_t _full_tree_cached_hash;

    core::param::ParamSlot _parameter_to_sample_slot;
    core::param::ParamSlot _num_samples_per_probe_slot;

private:
    template <typename T>
    void doSampling(const std::shared_ptr<pcl::KdTreeFLANN<pcl::PointXYZ>>& tree, std::vector<T>& data);
    bool getData(core::Call& call);

    bool getMetaData(core::Call& call);

    std::shared_ptr<ProbeCollection> _probes;

    size_t _old_datahash;
    bool _trigger_recalc;
    bool paramChanged(core::param::ParamSlot& p);
};


template <typename T>
void SampleAlongPobes::doSampling(const std::shared_ptr<pcl::KdTreeFLANN<pcl::PointXYZ>>& tree, std::vector<T>& data) {

    const int samples_per_probe = this->_num_samples_per_probe_slot.Param<core::param::IntParam>()->Value();

    for (int i = 0; i < _probes->getProbeCount(); i++) {

        auto probe = _probes->getProbe<FloatProbe>(i);
        auto samples = probe.getSamplingResult();
        // samples = std::make_shared<FloatProbe::SamplingResult>();

        auto sample_step = probe.m_end / static_cast<float>(samples_per_probe);
        auto radius = sample_step / 2.0f;

        float min_value = std::numeric_limits<float>::max();
        float max_value = std::numeric_limits<float>::min();
        float avg_value = 0.0f;
        samples->samples.resize(samples_per_probe);

        for (int j = 0; j < samples_per_probe; j++) {

            pcl::PointXYZ sample_point;
            sample_point.x = probe.m_position[0] + j * sample_step * probe.m_direction[0];
            sample_point.y = probe.m_position[1] + j * sample_step * probe.m_direction[1];
            sample_point.z = probe.m_position[2] + j * sample_step * probe.m_direction[2];

            std::vector<uint32_t> k_indices;
            std::vector<float> k_distances;

            auto num_neighbors = tree->radiusSearch(sample_point, radius, k_indices, k_distances);
            if (num_neighbors == 0) {
                num_neighbors = tree->nearestKSearch(sample_point, 1, k_indices, k_distances);
            }


            // accumulate values
            float value = 0;
            for (int n = 0; n < num_neighbors; n++) {
                value += data[k_indices[n]];
            } // end num_neighbors
            value /= num_neighbors;
            samples->samples[j] = value;
            min_value = std::min(min_value, value);
            max_value = std::max(max_value, value);
            avg_value += value;
        } // end num samples per probe
        avg_value /= samples_per_probe;
        samples->average_value = avg_value;
        samples->max_value = max_value;
        samples->min_value = min_value;
    } // end for probes
}


} // namespace probe
} // namespace megamol


#endif // !SAMPLE_ALONG_PROBES_H_INCLUDED