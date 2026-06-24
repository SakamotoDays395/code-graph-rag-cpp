///////////////////////////////////////////////////////////////////////////////
/// @file cache_manager.h
/// @brief LRU (Least Recently Used) cache for database query results
///
/// MAPS TO ORIGINAL: src/storage/cache-manager.ts
///
/// WHY: If the AI asks the same question 3 times in a row, we don't want
/// to hit the database 3 times. The cache returns the stored answer instantly.
///
/// WHAT TO LEARN:
///   - LRU Cache algorithm: https://leetcode.com/problems/lru-cache/
///   - std::list + std::unordered_map = O(1) LRU cache
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <list>
#include <unordered_map>
#include <optional>
#include <cstdint>
#include <chrono>

namespace codegraph {

template<typename T>
class CacheManager {
public:
    /// @param maxSize Maximum number of entries to keep
    /// @param ttlMs Time-to-live in milliseconds (entries expire after this)
    CacheManager(size_t maxSize = 1000, int64_t ttlMs = 300000);

    /// Get a cached value (returns nullopt if not found or expired)
    std::optional<T> get(const std::string& key);

    /// Store a value in the cache
    void put(const std::string& key, const T& value);

    /// Remove a specific entry
    void remove(const std::string& key);

    /// Clear all entries
    void clear();

    /// Get cache statistics
    struct Stats {
        size_t size;
        int    hits;
        int    misses;
        double hitRate;
    };
    Stats getStats() const;

private:
    struct CacheEntry {
        std::string key;
        T           value;
        int64_t     timestamp;
    };

    size_t  maxSize_;
    int64_t ttlMs_;
    int     hits_   = 0;
    int     misses_ = 0;

    // The LRU data structures:
    // - list: ordered by access time (front = most recent, back = least recent)
    // - map: key → iterator into the list (for O(1) lookup)
    std::list<CacheEntry> accessList_;
    std::unordered_map<std::string, typename std::list<CacheEntry>::iterator> lookupMap_;

    /// Evict the least recently used entry
    void evict();

    /// Get current timestamp in milliseconds
    int64_t now() const;
};

// ── Template implementation (must be in header for C++ templates) ───────────

template<typename T>
CacheManager<T>::CacheManager(size_t maxSize, int64_t ttlMs)
    : maxSize_(maxSize), ttlMs_(ttlMs) {}

template<typename T>
std::optional<T> CacheManager<T>::get(const std::string& key) {
    auto it = lookupMap_.find(key);
    if (it == lookupMap_.end()) {
        misses_++;
        return std::nullopt;
    }

    auto& entry = *(it->second);

    // Check TTL expiry
    if (now() - entry.timestamp > ttlMs_) {
        // Expired — remove it
        accessList_.erase(it->second);
        lookupMap_.erase(it);
        misses_++;
        return std::nullopt;
    }

    // Move to front (most recently used)
    accessList_.splice(accessList_.begin(), accessList_, it->second);
    hits_++;
    return entry.value;
}

template<typename T>
void CacheManager<T>::put(const std::string& key, const T& value) {
    // If key already exists, update it
    auto it = lookupMap_.find(key);
    if (it != lookupMap_.end()) {
        accessList_.erase(it->second);
        lookupMap_.erase(it);
    }

    // Evict if at capacity
    if (accessList_.size() >= maxSize_) {
        evict();
    }

    // Insert at front (most recently used)
    accessList_.push_front({key, value, now()});
    lookupMap_[key] = accessList_.begin();
}

template<typename T>
void CacheManager<T>::remove(const std::string& key) {
    auto it = lookupMap_.find(key);
    if (it != lookupMap_.end()) {
        accessList_.erase(it->second);
        lookupMap_.erase(it);
    }
}

template<typename T>
void CacheManager<T>::clear() {
    accessList_.clear();
    lookupMap_.clear();
    hits_ = 0;
    misses_ = 0;
}

template<typename T>
typename CacheManager<T>::Stats CacheManager<T>::getStats() const {
    int total = hits_ + misses_;
    return {
        accessList_.size(),
        hits_,
        misses_,
        total > 0 ? static_cast<double>(hits_) / total : 0.0
    };
}

template<typename T>
void CacheManager<T>::evict() {
    if (accessList_.empty()) return;
    // Remove the BACK (least recently used)
    auto& last = accessList_.back();
    lookupMap_.erase(last.key);
    accessList_.pop_back();
}

template<typename T>
int64_t CacheManager<T>::now() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

} // namespace codegraph
