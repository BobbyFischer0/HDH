#include "wildcard.h"

#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include <algorithm>

// ============================================================================
// Check for Wildcards
// ============================================================================

bool has_wildcards(const std::string& pattern) {
    return pattern.find('*') != std::string::npos ||
           pattern.find('?') != std::string::npos;
}

// ============================================================================
// Pattern Matching (supports * and ?)
// ============================================================================

bool match_pattern(const std::string& pattern, const std::string& str) {
    size_t p = 0, s = 0;
    size_t star_p = std::string::npos;
    size_t star_s = std::string::npos;
    
    while (s < str.size()) {
        if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == str[s])) {
            // Match single character or exact match
            p++;
            s++;
        }
        else if (p < pattern.size() && pattern[p] == '*') {
            // Star found - remember position
            star_p = p;
            star_s = s;
            p++;
        }
        else if (star_p != std::string::npos) {
            // Mismatch after star - backtrack
            p = star_p + 1;
            star_s++;
            s = star_s;
        }
        else {
            // No match
            return false;
        }
    }
    
    // Check remaining pattern (should only be stars)
    while (p < pattern.size() && pattern[p] == '*') {
        p++;
    }
    
    return p == pattern.size();
}

// ============================================================================
// Expand Wildcard Pattern
// ============================================================================

std::vector<std::string> expand_glob(const std::string& pattern) {
    std::vector<std::string> results;
    
    // Find directory and file pattern
    size_t last_slash = pattern.rfind('/');
    std::string dir_path;
    std::string file_pattern;
    
    if (last_slash != std::string::npos) {
        dir_path = pattern.substr(0, last_slash + 1);
        file_pattern = pattern.substr(last_slash + 1);
    } else {
        dir_path = ".";
        file_pattern = pattern;
    }
    
    // Open directory
    DIR* dir = opendir(dir_path.c_str());
    if (dir == nullptr) {
        // Return original pattern if directory can't be opened
        results.push_back(pattern);
        return results;
    }
    
    // Read directory entries
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        
        // Skip hidden files unless pattern starts with .
        if (name[0] == '.' && file_pattern[0] != '.') {
            continue;
        }
        
        // Skip . and ..
        if (name == "." || name == "..") {
            continue;
        }
        
        // Check if name matches pattern
        if (match_pattern(file_pattern, name)) {
            if (dir_path != ".") {
                results.push_back(dir_path + name);
            } else {
                results.push_back(name);
            }
        }
    }
    
    closedir(dir);
    
    // Sort results
    std::sort(results.begin(), results.end());
    
    // If no matches, return original pattern
    if (results.empty()) {
        results.push_back(pattern);
    }
    
    return results;
}
