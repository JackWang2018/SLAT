/**
 * @file   structure.h
 * @author Michael Gauland <michael.gauland@canterbury.ac.nz>
 * @date   Wed Feb 03 10:04:50 NZDT 2016
 *
 * @brief  Classes representing structures (composed on component groups).
 * 
 * This file part of SLAT (the Seismic Loss Assessment Tool).
 *
 * ©2015 Canterbury University
 */
#ifndef _STRUCTURE_H_
#define _STRUCTURE_H_

#include "comp_group.h"
#include "lognormaldist.h"
#include "maq.h"

namespace SLAT {
    class Structure {
    public:
        Structure(std::string name) {
            this->name = name;
        };
        ~Structure() {};

        void AddCompGroup(std::shared_ptr<CompGroup> cg);

        Integration::IntegrationSettings local_settings;
        static Integration::IntegrationSettings class_settings;

        LogNormalDist Loss(double im, bool consider_collapse);
        std::pair<LogNormalDist, LogNormalDist> DeaggregatedLoss(double im);
        void setRebuildCost(LogNormalDist dist) { rebuild_cost = dist; };
        LogNormalDist getRebuildCost(void) { return rebuild_cost; };
    private:
        std::string name;
        LogNormalDist LossNC(double im);
        std::vector<std::shared_ptr<CompGroup>> components;
        std::shared_ptr<IM> im;
        LogNormalDist rebuild_cost;
    };
}
#endif
