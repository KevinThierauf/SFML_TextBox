//
// Created by Computer on 3/16/2021.
//

#include "Pos.hpp"
#include <ostream>

namespace sftb {
    std::ostream &operator<<(std::ostream &out, const Pos &position) {
        return out << "(" << position.line << ", " << position.position << ")";
    }
}