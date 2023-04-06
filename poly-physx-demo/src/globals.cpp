#include "globals.hpp"
#include <string>
#include <unordered_map>

namespace glob
{
    const char *generate_name(const std::size_t id)
    {
        static std::unordered_map<std::size_t, std::string> names;
        if (names.find(id) != names.end())
            return names.at(id).c_str();

        const char syllables[6] = "aeiou",
                   consonants[22] = "bcdfghjklmnpqrstvwxyz";
        const std::size_t mm = 3, mx = 8;
        char name[mx + 1];

        std::size_t index = 0;
        const std::uint8_t is_pair = rand() % 2;
        while (index < mx)
        {
            const bool is_syllable = index % 2 == is_pair;
            const std::size_t idx = rand() % (is_syllable ? 5 : 21);
            if (index >= mm && (is_syllable ? (idx == 4) : (idx == 20)))
                break;
            name[index++] = is_syllable ? syllables[idx] : consonants[idx];
        }
        name[0] = toupper(name[0]);
        name[index] = '\0';
        names[id] = name;
        return names.at(id).c_str();
    }
}