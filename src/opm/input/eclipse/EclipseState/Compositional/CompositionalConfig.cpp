/*
Copyright (C) 2024 SINTEF Digital, Mathematics and Cybernetics.

This file is part of the Open Porous Media project (OPM).

      OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckSection.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/EclipseState/Compositional/CompositionalConfig.hpp>
#include <opm/input/eclipse/EclipseState/Tables/Tabdims.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/A.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/E.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/N.hpp>

#include <opm/common/utility/OpmInputError.hpp>
#include <opm/common/OpmLog/OpmLog.hpp>

#include <fmt/format.h>

#include <stdexcept>
#include <string>

namespace Opm {

CompostionalConfig::CompostionalConfig(const Deck& deck, const Runspec& runspec) {
    if (!DeckSection::hasPROPS(deck)) return;

    if (!runspec.compostionalMode()) return; // or we should go head to give more diagnostic messages.

    const bool comp_mode_runspec = runspec.compostionalMode(); // TODO: the way to use comp_mode_runspec should be refactored
    if (comp_mode_runspec) {
        this->num_comps = runspec.numComps();
    }

    const PROPSSection props_section {deck};
    if (props_section.hasKeyword<ParserKeywords::NCOMPS>()) {
        if (comp_mode_runspec) {
            const auto& keywords = props_section.get<ParserKeywords::NCOMPS>();
            for (const auto& kw : keywords) {
                const auto& item = kw.getRecord(0).getItem<ParserKeywords::NCOMPS::NUM_COMPS>();
                const auto ncomps = item.get<int>(0);
                if (ncomps != this->num_comps) {
                    const std::string msg = fmt::format("NCOMPS is specified with {}, which is different from the number specified in COMPS {}",
                                                         num_comps, this->num_comps);
                    throw OpmInputError(msg, kw.location());
                }
            }
        } else {
            OpmLog::warning("NCOMPS will be ignored since there is no COMPS specified in RUNSPEC");
        }
    }

    const auto tabdims = Tabdims(deck);
    const auto num_eos_res = tabdims.getNumEosRes();
    eos_types.resize(num_eos_res, EOSType::PR);
    if (props_section.hasKeyword<ParserKeywords::EOS>()) {
        if (comp_mode_runspec) {
            // TODO: should we use the last one directly?
            // TODO: how should we handle multiple EOS input, new one overwrite the old one?
            const auto& keywords = props_section.get<ParserKeywords::EOS>();
            for (const auto& kw : keywords) {
                for (size_t i = 0; i < kw.size(); ++i) {
                    const auto& item = kw.getRecord(i).getItem<ParserKeywords::EOS::EQUATION>();
                    const auto& equ_str = item.getTrimmedString(0);
                    eos_types[i] = CompostionalConfig::eosTypeFromString(equ_str); // this
                }
            }
        } else {
            OpmLog::warning("EOS keywords will be ignored since there is no COMPS specified in RUNSPEC");
        }
    }
    acentric_factors.resize(num_eos_res, std::vector<double>(this->num_comps, 0.));
    if (props_section.hasKeyword<ParserKeywords::ACF>()) {
        if (comp_mode_runspec) {
            const auto& keywords = props_section.get<ParserKeywords::ACF>();
            for (const auto& kw : keywords) {
                for (size_t i = 0; i < kw.size(); ++i) {
                    const auto& item = kw.getRecord(i).getItem<ParserKeywords::ACF::DATA>();
                    const auto data = item.getData<double>();
                    if (data.size() > this->num_comps) { // size_t vs int
                        const auto msg = fmt::format("in keyword ACF, {} values are specified, which is bigger than the number of components {}",
                                                      data.size(), this->num_comps);
                        throw OpmInputError(msg, kw.location());
                    }
                    std::copy(data.begin(), data.end(), acentric_factors[i].begin());
                }
            }
        } else {
            OpmLog::warning("ACF keywords will be ignored since there is no COMPS specified in RUNSPEC");
        }
    }

    // PCRIT
    // VCRIT
    // TCRIT

    // BIC a little different
};

bool CompostionalConfig::operator==(const CompostionalConfig& other) const {
    return this->num_comps == other.num_comps &&
           this->eos_types == other.eos_types;
}


CompostionalConfig CompostionalConfig::serializationTestObject() {
    CompostionalConfig result;

    result.num_comps = 3;
    result.eos_types = {2, EOSType::SRK};

    return result;
}

CompostionalConfig::EOSType CompostionalConfig::eosTypeFromString(const std::string& str) {
    if (str == "PR") return EOSType::PR;
    if (str == "RK") return EOSType::RK;
    if (str == "SRK") return EOSType::SRK;
    if (str == "ZJ") return EOSType::ZJ;
    throw std::invalid_argument("Unknown string for EOSType");
}

std::string CompostionalConfig::eosTypeToString(Opm::CompostionalConfig::EOSType eos) {
    switch (eos) {
        case EOSType::PR: return "PR";
        case EOSType::RK: return "RK";
        case EOSType::SRK: return "SRK";
        case EOSType::ZJ: return "ZJ";
        default: throw std::invalid_argument("Unknown EOSType");
    }
}

}