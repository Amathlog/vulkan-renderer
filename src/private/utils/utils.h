#pragma once

#include <cstdint>
#include <cstring>
#include <vector>

namespace VulkanRenderer
{
namespace Utils
{
// Validate that all inNames are present in others
// others can be any type (basically any Vulkan structure), with a callback to get the string
// from it.
// Return inNamesCount if all found, otherwise, return the index of the first not found item
template <typename T, typename Func>
int ValidateStrings(const char* const* inNames, uint32_t inNamesCount, const T* others, uint32_t othersCount,
                    Func getStringCallback)
{
    for (uint32_t i = 0; i < inNamesCount; ++i)
    {
        bool found = false;
        for (uint32_t j = 0; j < othersCount; ++j)
        {
            if (std::strcmp(getStringCallback(others[j]), inNames[i]) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            return i;
        }
    }

    return inNamesCount;
}

// Validate that all inNames are present in others
// others can be any type (basically any Vulkan structure), with a callback to get the string
// from it.
// Return inNames.size() if all found, otherwise, return the index of the first not found item
// inNames needs to hold char*, and containers should support ".data" and ".size" (like a std container)
template <typename FirstConstainer, typename SecondContainer, typename Func>
int ValidateStrings(const FirstConstainer& inNames, const SecondContainer& otherNames, Func&& getStringCallback)
{
    return ValidateStrings(inNames.data(), static_cast<uint32_t>(inNames.size()), otherNames.data(),
                           static_cast<uint32_t>(otherNames.size()), std::forward<Func>(getStringCallback));
}
} // namespace Utils
} // namespace VulkanRenderer