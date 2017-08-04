/**
 * @file   loss_functions.cpp
 * @author Michael Gauland <michael.gauland@canterbury.ac.nz>
 * @date   Mon Nov 16 15:29:29 2015
 * 
 * @brief  Classes representing loss functions.
 * 
 * This file part of SLAT (the Seismic Loss Assessment Tool).
 *
 * ©2015 Canterbury University
 */
#include "loss_functions.h"
#include <vector>
#include <stdexcept>

namespace SLAT {

    LossFn::LossFn(std::vector<LogNormalDist> distributions)
    {
        if (distributions.size() == 0) {
            throw std::invalid_argument("distributions");
        } else {
            loss_functions = distributions;
        }
    }

    std::size_t LossFn::n_states(void)
    {
        return loss_functions.size();
    }

    LogNormalDist LossFn::CalculateLoss(std::vector<double> probabilities, int count)
    {
        LogNormalDist result = LogNormalDist::AddWeightedDistributions(loss_functions,
                                                                       probabilities);
        return result.ScaleDistribution(count);
    }

    std::ostream& operator<<(std::ostream& out, LossFn& o)
    {
        out << "LossFn: ";
        for (size_t i=0; i < o.n_states(); i++) {
            out << o.loss_functions[i] << "; ";
        }
        out << std::endl;
        return out;
    }
}
