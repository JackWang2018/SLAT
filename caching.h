/**
 * @file   caching.h
 * @author Michael Gauland <michael.gauland@canterbury.ac.nz>
 * @date   Mon Nov 16 15:29:29 2015
 * 
 * @brief  Facilitates for caching function results.
 * 
 * This file part of SLAT (the Seismic Loss Assessment Tool).
 *
 * ©2015 Canterbury University
 */
#ifndef _CACHING_H_
#define _CACHING_H_

#include <unordered_map>
#include <functional>
#include <iostream>
#include <iomanip>
#include <string>
#include <omp.h>

namespace SLAT {
    namespace Caching {
        void Add_Cache(void *cache, std::function<void (void)> clear_func);
        void Remove_Cache(void *cache);
        void Clear_Caches(void);
            
        template <class T, class V> class CachedFunction {
        public:
            omp_lock_t lock;
            std::function<T (V)> func;
            std::unordered_map<V, T> cache;
            bool cache_active;
        public:
            CachedFunction() {
                name = "Anonymous";
                hits = 0;
                total_calls = 0;
                omp_init_lock(&lock);
            };
            CachedFunction(std::function<T (V)> base_func, std::string name="Anonymous", bool activate_cache=true) { 
                omp_init_lock(&lock);
                this->name = name;
                cache_active = activate_cache;
                hits = 0;
                total_calls = 0;
                func = base_func; 
                Add_Cache(this, [this] (void) { 
                        this->ClearCache(); });
            };
            ~CachedFunction() {
                std::cout << std::setw(50) << name
                          << std::setw(10) << hits
                          << std::setw(10) << total_calls 
                          << std::endl;
                Remove_Cache(this);
            }
            T operator()(V v) { 
                omp_set_lock(&lock);
                total_calls++;
                if (cache_active) {
                    if (cache.count(v) == 0) {
                        cache[v] = func(v); 
                    } else {
                        hits++;
                    }
                    omp_unset_lock(&lock);
                    return cache[v];
                } else {
                    omp_unset_lock(&lock);
                    return func(v);
                }
            }
            void ClearCache(void) {
                cache.clear(); 
            };
        private:
            std::string name;
            int hits;
            int total_calls;
        };

        template <class T> class CachedValue {
        private:
            std::function<T (void)> func;
            T cached_value;
            bool cache_valid;
            int hits, total_calls;
            std::string name;
        public:
        CachedValue() : cache_valid(false) { 
                name = "Anonymous";
                total_calls = 0;
                hits = 0;
            };
            CachedValue(std::function<T (void)> base_func, std::string name = "Anonymous") { 
                this->name = name;
                cache_valid = false;
                total_calls = 0;
                hits = 0;
                func = base_func; 
                Add_Cache(this, [this] (void) { 
                        this->ClearCache(); });
            };
            ~CachedValue() {
                std::cout << std::setw(50) << name
                          << std::setw(10) << hits
                          << std::setw(10) << total_calls 
                          << std::endl;
            };
            T operator()(void) { 
                total_calls++;
                if (!cache_valid) {
                    cached_value = func();
                    cache_valid = true;
                } else {
                    hits++;
                }
                return cached_value;
            }
            void ClearCache(void) {
                cache_valid = false;
            };
        };
    }
}
#endif
