/**
 * @file   relationships.h
 * @author Michael Gauland <michael.gauland@canterbury.ac.nz>
 * @date   Mon Nov 16 15:29:29 2015
 * 
 * @brief  Classes representing relationships involving mathematical functions.
 * 
 * This file part of SLAT (the Seismic Loss Assessment Tool).
 *
 * ©2015 Canterbury University
 */
#ifndef _RELATIONSHIPS_H_
#define _RELATIONSHIPS_H_

#include <unordered_map>
#include "functions.h"
#include "maq.h"
#include "replaceable.h"
#include "caching.h"

#include <iostream>

namespace SLAT {

/**
 * @brief Rate Relationship
 * 
 * A 'Rate Relationship' represents the probability of a function exceeding a
 * given value. This is an abstract class.
 */
    class RateRelationship : public Replaceable<RateRelationship>
    {
    public:
        virtual ~RateRelationship() {
        };   /**< Default destructor; does nothing. */
    protected:
        RateRelationship(bool activate_cache);
        Integration::IntegrationSettings local_settings;
        static Integration::IntegrationSettings class_settings;
        int callback_id;

        /** 
         * Returns the probability of exceedence at a given value.
         * 
         * @param x The value for which we want to know the probability.
         * 
         * @return The probability the of exceedence.
         */
        virtual double calc_lambda(double x) {
            return NAN;
        };

        
    public:
        Caching::CachedFunction<double, double> lambda;
        
        /** 
         * Returns the derivative of lambda at the given point.
         * 
         * @param x The value at which we want to know the derivative.
         * 
         * @return The derivative of the relationship at x.
         */
        virtual double DerivativeAt(double x) ;

        /**
         * Returns class integration settings:
         */
        static Integration::IntegrationSettings &Get_Class_Integration_Settings(void);

        /**
         * Returns object integration settings:
         */
        Integration::IntegrationSettings &Get_Integration_Settings(void);

        virtual std::string ToString(void) const;
            
        friend std::ostream& operator<<(std::ostream& out, const RateRelationship& o);
    };

/**
 * @brief Simple Rate Relationship
 * 
 * A simple rate relationship is defined by a single, deterministic
 * function. The function should describe the probability that a value will
 * exceed a given value. This should be 1 at 0 (i.e., the value is guaranteed to
 * be at least zero), monotonically decreasing to 0.
 */
    class SimpleRateRelationship : public RateRelationship
    {
    public:
        /** 
         * Construct a SimpleRateRelationship given a shared pointer to a
         * deterministic function that describes the rate relationship.
         * 
         * @param func A shared pointer to the function defining the relationship.
         */
        SimpleRateRelationship(std::shared_ptr<DeterministicFn> func);
        ~SimpleRateRelationship() {
            f->remove_callbacks(callback_id);
        }; /**< Destructor; uninstall callbacks */
        /** 
         * Return the probability of exceeding a given value.
         * 
         * @param x  The value at which we want to know the probability.
         * 
         * @return The probability that the value will exceed x.
         */
        virtual double calc_lambda(double x);

        virtual std::string ToString(void) const;
    protected:
        std::shared_ptr<DeterministicFn> f; /**< Function describing the
                                                   * relationship. */
    };

/**
 * @brief Comound Rate Relationship
 * 
 * A compound rate relationship is defined by a rate relationship, and a
 * probabilistic function. For example, given an rate relationship for IM, and a
 * probabilistic function describing the relationship between IM and EDP, a
 * compound relationship will combine them to describe the rate relationship for
 * EDP.
 */
    class CompoundRateRelationship : public RateRelationship
    {
    public:
        /** 
         * Set the default tolerance and evaluations limit used by the
         * integrator when calculating lambda for CompoundRateRelationships.
         * 
         * @param tol   Tolerance
         * @param evals Maximum number of evaluations
         */
        static void SetDefaultIntegrationParameters(double tol, unsigned int evals);

        /** 
         * Get the default tolerance and evaluations limit used by the
         * integrator when calculating lambda for CompoundRateRelationships.
         * 
         * @param tol    Returned with the tolerance.
         * @param evals  Returned with the evaluations limit.
         */
        static void GetDefaultIntegrationParameters(double &tol, unsigned int &evals);

        /** 
         * Create a compound relationship given shared pointers to a rate
         * relationship and a probabilistic function.
         * 
         * @param base_rate       A shared pointer to a rate relationship
         * @param dependent_rate  A shared pointer to a probabilistic function.
         * 
         * @return 
         */
        CompoundRateRelationship(std::shared_ptr<RateRelationship> base_rate,
                                 std::shared_ptr<ProbabilisticFn> dependent_rate);

        ~CompoundRateRelationship() {
            base_rate->remove_callbacks(base_rate_callback_id);
            dependent_rate->remove_callbacks(dependent_rate_callback_id);
        }; /**< Destructor; unregister callbacks */


        /** 
         * Return the probability of exceeding a given value in the complex
         * relationship. This is calculated by looking at all values of the base
         * rate, and determing 1) how likely that value is to be encountered, and 2)
         * given that base rate, how likely is the dependent function to be
         * exceeded?
         * 
         * @param x  The value at which we want to know the probability.
         * 
         * @return The probability that the value will exceed x.
         */
        virtual double calc_lambda(double x);

        virtual std::string ToString(void) const;

        double P_exceedence(double base_value, double min_dependent_value) const;
        
        std::shared_ptr<RateRelationship> Base_Rate(void) { return base_rate; };
    protected:
        std::shared_ptr<RateRelationship> base_rate; /**< Base rate relationship */
        std::shared_ptr<ProbabilisticFn> dependent_rate; /**< Dependent function */

        double tolerance;
        unsigned int max_evaluations;

        int base_rate_callback_id, dependent_rate_callback_id;

        static double default_tolerance;
        static unsigned int default_max_evaluations;
    };
}
#endif
