#ifndef WILDCARD_H
#define WILDCARD_H

#include <string>
#include <vector>

// ============================================================================
// Wildcard Expansion
// ============================================================================

// Check if pattern contains wildcards
bool has_wildcards(const std::string& pattern);

// Expand a wildcard pattern to matching files
// Returns original pattern if no matches found
std::vector<std::string> expand_glob(const std::string& pattern);

// Match a string against a pattern with * and ?
bool match_pattern(const std::string& pattern, const std::string& str);

#endif // WILDCARD_H
