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
#include <signal.h>
#include <cmath>

namespace SLAT {
    namespace Caching {
        namespace Private {
            /*
             * Don't use anything from this namespace directly from outside this module:
             */
            void Init_Caching(void);
            void Add_Cache(void *cache, std::function<void (void)> clear_func);
            void Remove_Cache(void *cache);
        };

        /*
         * This can be used safely anwywhere, to clear all caches:
         */
        void Clear_Caches(void);
            
        template <class T, class V> class CachedFunction {
        public:
            std::function<T (V)> func;
            std::unordered_map<V, T> cache;
            bool cache_active;
        public:
            CachedFunction()  {
                Caching::Private::Init_Caching();
                name = "Anonymous";
                hits = 0;
                total_calls = 0;
                omp_init_lock(&lock);
                func = [this] (V v) {
                    throw std::runtime_error("Cached function not provided");
                    return T();
                };
            };
            CachedFunction(std::function<T (V)> base_func, std::string name="Anonymous", bool activate_cache=true) { 
                Caching::Private::Init_Caching();
                omp_init_lock(&lock);
                this->name = name;
                cache_active = activate_cache;
                hits = 0;
                total_calls = 0;
                func = base_func; 
                Caching::Private::Add_Cache(this, [this] (void) { 
                        this->ClearCache(); });
            };
            ~CachedFunction() {
                Caching::Private::Remove_Cache(this);
            }
            T operator()(V v) { 
                omp_set_lock(&lock);
#pragma omp flush 
                total_calls++;
                if (cache_active && !std::isnan(v)) {
                    bool cached = cache.count(v) != 0;
                    if (!cached) {
                        bool happening = in_process.count(v) != 0;
                        if (happening) {
                            in_process[v].count++;
#pragma omp flush 
                            omp_lock_t *calc_lock = &in_process[v].lock;
                            omp_unset_lock(&lock);
                            omp_set_lock(calc_lock);
                            omp_set_lock(&lock);
#pragma omp flush 
                            in_process[v].count--;
                            if (in_process[v].count > 0) {
                                omp_unset_lock(&in_process[v].lock);
                            } else {
                                in_process.erase(v);
                            }
                        } else {
                            omp_init_lock(&in_process[v].lock);
                            omp_set_lock(&in_process[v].lock);
                            in_process[v].count = 0;
#pragma omp flush 
                            omp_unset_lock(&lock);
                            T result = func(v); 
                            omp_set_lock(&lock);
#pragma omp flush 
                            if (cache.count(v) != 0) {
                                std::cout << "DUPLICATE (f): " << name << " at " << v << std::endl;
                                std::cout << in_process[v].count << std::endl;
                                raise(SIGINT);
                            }
                            cache[v] = result;
                            if (in_process[v].count == 0) {
                                in_process.erase(v);
                            } else {
                                omp_unset_lock(&in_process[v].lock);
                            }
                        }
                    } else {
                        hits++;
                    }
                    T result = cache[v];
#pragma omp flush 
                    omp_unset_lock(&lock);
                    return result;
                } else {
                    omp_unset_lock(&lock);
                    return func(v);
                }
            }
            void ClearCache(void) {
                cache.clear(); 
            };
        private:
            struct WAITING {omp_lock_t lock; int count; };
            std::unordered_map<V, struct WAITING> in_process;
            omp_lock_t lock;
            std::string name;
            int hits;
            int total_calls;
        };

        template <class T> class CachedValue {
        private:
            bool in_process;
            omp_lock_t waiting_lock;
            omp_lock_t lock;
            std::function<T (void)> func;
            T cached_value;
            bool cache_valid;
            int hits, total_calls;
            std::string name;
        public:
        CachedValue() : cache_valid(false) { 
                Caching::Private::Init_Caching();
                name = "Anonymous";
                total_calls = 0;
                hits = 0;
                omp_init_lock(&lock);
                omp_init_lock(&waiting_lock);
                omp_set_lock(&waiting_lock);
                in_process = false;
                func = [this] (void) {
                    throw std::runtime_error("Function for CachedValue not provided");
                    return T();
                };
            };
            CachedValue(std::function<T (void)> base_func, std::string name = "Anonymous") { 
                Caching::Private::Init_Caching();
                omp_init_lock(&lock);
                omp_init_lock(&waiting_lock);
                omp_set_lock(&waiting_lock);
                in_process = false;
                this->name = name;
                cache_valid = false;
                total_calls = 0;
                hits = 0;
                func = base_func; 
                Caching::Private::Add_Cache(this, [this] (void) { 
                        this->ClearCache(); });
            };
            ~CachedValue() {
                Caching::Private::Remove_Cache(this);
            };
            T operator()(void) { 
                omp_set_lock(&lock);
#pragma omp flush 
                total_calls++;
                if (!cache_valid) {
                    if (in_process) {
#pragma omp flush 
                        omp_unset_lock(&lock);
                        omp_set_lock(&waiting_lock);
                        omp_unset_lock(&waiting_lock); // Release for anyone else who's waiting
                        omp_set_lock(&lock);
#pragma omp flush 
                    } else {
                        in_process = true;
#pragma omp flush 
                        omp_unset_lock(&lock);
                        T result = func();
                        omp_set_lock(&lock);
                        if (cache_valid) {
                            std::cout << "DUPLICATE (v): " << name << std::endl;
                            raise(SIGINT);
                        }
                        cached_value = result;
                        cache_valid = true;
                        in_process = false;
#pragma omp flush 
                        omp_unset_lock(&waiting_lock);
                    }                        
                } else {
                    hits++;
                }
                T result = cached_value;
                omp_unset_lock(&lock);
                return result;
            }
            void ClearCache(void) {
                cache_valid = false;
            };
        };
    }
}
#endif
