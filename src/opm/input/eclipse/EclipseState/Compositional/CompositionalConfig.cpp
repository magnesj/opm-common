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
#include <opm/input/eclipse/Parser/ParserKeywords/B.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/C.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/E.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/M.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/N.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/P.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/S.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/T.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/V.hpp>

#include <opm/common/utility/OpmInputError.hpp>
#include <opm/common/OpmLog/OpmLog.hpp>

#include <fmt/format.h>

#include <stdexcept>
#include <string>
#include <unordered_map>

namespace Opm {

CompositionalConfig::CompositionalConfig(const Deck& deck, const Runspec& runspec) {
    if (!DeckSection::hasPROPS(deck)) return;

    const PROPSSection props_section {deck};
    const bool comp_mode_runspec = runspec.compositionalMode(); // TODO: the way to use comp_mode_runspec should be refactored
    if (!comp_mode_runspec) {
        warningForExistingCompKeywords(props_section);
        return; // not processing compositional props keywords
    }

    // we are under compositional mode now
    this->num_comps = runspec.numComps();

    if (props_section.hasKeyword<ParserKeywords::NCOMPS>()) {
        // NCOMPS might be present within multiple included files
        // We check all the input NCOMPS until testing proves that we can not have multiple of them
        const auto& keywords = props_section.get<ParserKeywords::NCOMPS>();
        for (const auto& kw : keywords) {
            const auto& item = kw.getRecord(0).getItem<ParserKeywords::NCOMPS::NUM_COMPS>();
            const auto ncomps = item.get<int>(0);
            if (size_t(ncomps) != this->num_comps) {
                const std::string msg = fmt::format("NCOMPS is specified with {}, which is different from the number specified in COMPS {}",
                                                    ncomps, this->num_comps);
                throw OpmInputError(msg, kw.location());
            }
        }
    }

    if ( !props_section.hasKeyword<ParserKeywords::CNAMES>() ) {
        throw std::logic_error("CNAMES is not specified for compositional simulation");
    } else {
        comp_names.resize(num_comps);
        const auto& keywords = props_section.get<ParserKeywords::CNAMES>();
        if (keywords.size() > 1) {
            throw OpmInputError("there are multiple CNAMES keyword specification", keywords.begin()->location());
        }
        const auto& kw = keywords.back();
        const auto& item = kw.getRecord(0).getItem<ParserKeywords::CNAMES::data>();
        const auto names_size = item.getData<std::string>().size();
        if (names_size != num_comps) {
            const auto msg = fmt::format("in keyword CNAMES, {} values are specified, which is different from the number of components {}",
                                         names_size, num_comps);
            throw OpmInputError(msg, kw.location());
        }
        for (size_t c = 0; c < num_comps; ++c) {
            comp_names[c] = item.getTrimmedString(c);
        }
    }


    if (props_section.hasKeyword<ParserKeywords::STCOND>()) {
        const auto& keywords = props_section.get<ParserKeywords::STCOND>();
        if (keywords.size() > 1) {
            throw OpmInputError("there are multiple STCOND keyword specification", keywords.begin()->location());
        }
        const auto& kw = keywords.back();
        const auto& temp_item = kw.getRecord(0).getItem<ParserKeywords::STCOND::TEMPERATURE>();
        this->standard_temperature = temp_item.getSIDouble(0);
        const auto& pres_item = kw.getRecord(0).getItem<ParserKeywords::STCOND::PRESSURE>();
        this->standard_pressure = pres_item.getSIDouble(0);
    }

    const Tabdims tabdims{deck};
    const size_t num_eos_res = tabdims.getNumEosRes();
    // EOS keyword can also be in RUNSPEC section, we also parse the EOS in the RUNSPEC section here for simplicity
    // might be suggested to handle in the RUNSPEC section though
    eos_types.resize(num_eos_res, EOSType::PR);
    {
        const RUNSPECSection runsec_section {deck};
        using KWEOS = ParserKeywords::EOS;
        if (props_section.hasKeyword<KWEOS>() || runsec_section.hasKeyword<KWEOS>()) {
            // we are not allowing EOS specified in both places
            if (props_section.hasKeyword<KWEOS>() && runsec_section.hasKeyword<KWEOS>()) {
                throw std::logic_error("EOS is specified in both RUNSPEC and PROP sections");
            }

            // we do not allow multiple input of the keyword EOS unless proven otherwise
            // only one section has EOS defined when we reach here
            const auto& keywords = props_section.hasKeyword<KWEOS>() ? props_section.get<KWEOS>() : runsec_section.get<KWEOS>();
            if (keywords.size() > 1) {
                throw OpmInputError("there are multiple EOS keyword specification", keywords.begin()->location());
            }
            const auto& kw = keywords.back();
            if (kw.size() > num_eos_res) {
                const auto msg = fmt::format(" {} equations of state are specified in keyword EOS, which is more than the number of"
                                             " of equation of state regions of {}.", kw.size(), num_eos_res);
                throw OpmInputError(msg, kw.location());
            }
            for (size_t i = 0; i < kw.size(); ++i) {
                const auto& item = kw.getRecord(i).getItem<KWEOS::EQUATION>();
                const auto& equ_str = item.getTrimmedString(0);
                eos_types[i] = eosTypeFromString(equ_str);
            }
        }
    }

    CompositionalConfig::processKeyword<ParserKeywords::MW>(props_section, this->molecular_weights,
                                                            num_eos_res, this->num_comps, "MW");
    CompositionalConfig::processKeyword<ParserKeywords::ACF>(props_section, this->acentric_factors,
                                                            num_eos_res, this->num_comps, "ACF");
    CompositionalConfig::processKeyword<ParserKeywords::PCRIT>(props_section, this->critical_pressure,
                                                              num_eos_res, this->num_comps, "PCRIT");
    CompositionalConfig::processKeyword<ParserKeywords::TCRIT>(props_section, this->critical_temperature,
                                                              num_eos_res, this->num_comps, "TCRIT");
    CompositionalConfig::processKeyword<ParserKeywords::VCRIT>(props_section, this->critical_volume,
                                                              num_eos_res, this->num_comps, "VCRIT");

    const std::size_t bic_size = this->num_comps * (this->num_comps - 1) / 2;
    CompositionalConfig::processKeyword<ParserKeywords::BIC>(props_section, this->binary_interaction_coefficient,
                                                             num_eos_res, bic_size, "ACF", 0.);
}

bool CompositionalConfig::operator==(const CompositionalConfig& other) const {
    return this->num_comps == other.num_comps &&
           this->standard_temperature == other.standard_temperature &&
           this->standard_pressure == other.standard_pressure &&
           this->comp_names ==other.comp_names &&
           this->eos_types == other.eos_types &&
           this->molecular_weights == other.molecular_weights &&
           this->acentric_factors == other.acentric_factors &&
           this->critical_pressure == other.critical_pressure &&
           this->critical_temperature == other.critical_temperature &&
           this->critical_volume == other.critical_volume &&
           this->binary_interaction_coefficient == other.binary_interaction_coefficient;
}


CompositionalConfig CompositionalConfig::serializationTestObject() {
    CompositionalConfig result;

    result.num_comps = 3;
    result.standard_temperature = 5.;
    result.standard_pressure = 1e5;
    result.comp_names = {"C1", "C10"};
    result.eos_types = {2, EOSType::SRK};
    result.molecular_weights = {2, std::vector<double>(result.num_comps, 16.)};
    result.acentric_factors = {2,  std::vector<double>(result.num_comps, 1.)};
    result.critical_pressure = {2, std::vector<double>(result.num_comps, 2.)};
    result.critical_temperature = {2, std::vector<double>(result.num_comps, 3.)};
    result.critical_volume = {2, std::vector<double>(result.num_comps, 5.)};
    result.binary_interaction_coefficient = {2, std::vector<double>(result.num_comps * (result.num_comps - 1) / 2, 6.)};

    return result;
}

CompositionalConfig::EOSType CompositionalConfig::eosTypeFromString(const std::string& str) {
    if (str == "PR") return EOSType::PR;
    if (str == "RK") return EOSType::RK;
    if (str == "SRK") return EOSType::SRK;
    if (str == "ZJ") return EOSType::ZJ;
    throw std::invalid_argument("Unknown string for EOSType");
}

std::string CompositionalConfig::eosTypeToString(Opm::CompositionalConfig::EOSType eos) {
    switch (eos) {
        case EOSType::PR: return "PR";
        case EOSType::RK: return "RK";
        case EOSType::SRK: return "SRK";
        case EOSType::ZJ: return "ZJ";
        default: throw std::invalid_argument("Unknown EOSType");
    }
}

void CompositionalConfig::warningForExistingCompKeywords(const PROPSSection& props_section) {
    bool any_comp_prop_kw = false;
    std::string msg {" COMPS is not specified, the following keywords related to compositional simulation in PROPS section will be ignored:\n"};

    static const std::unordered_map<std::string, std::function<bool(const PROPSSection&)>> keywordCheckers = {
        {"NCOMPS", [](const PROPSSection& section) -> bool { return section.hasKeyword<ParserKeywords::NCOMPS>(); }},
        {"CNAMES", [](const PROPSSection& section) -> bool { return section.hasKeyword<ParserKeywords::CNAMES>(); }},
        {"EOS",    [](const PROPSSection& section) -> bool { return section.hasKeyword<ParserKeywords::EOS>(); }},
        {"STCOND", [](const PROPSSection& section) -> bool { return section.hasKeyword<ParserKeywords::STCOND>(); }},
        {"PCRIT",  [](const PROPSSection& section) -> bool { return section.hasKeyword<ParserKeywords::PCRIT>(); }},
        {"TCRIT",  [](const PROPSSection& section) -> bool { return section.hasKeyword<ParserKeywords::TCRIT>(); }},
        {"VCRIT",  [](const PROPSSection& section) -> bool { return section.hasKeyword<ParserKeywords::VCRIT>(); }},
        {"ACF",    [](const PROPSSection& section) -> bool { return section.hasKeyword<ParserKeywords::ACF>(); }},
        {"BIC",    [](const PROPSSection& section) -> bool { return section.hasKeyword<ParserKeywords::BIC>(); }},
    };

    for (const auto& [kwname, checker] : keywordCheckers) {
        if (checker(props_section)) {
            any_comp_prop_kw = true;
            fmt::format_to(std::back_inserter(msg),  " {}", kwname);
        }
    }

    if (any_comp_prop_kw) {
        OpmLog::warning(msg);
    }
}

double CompositionalConfig::standardTemperature() const {
    return this->standard_temperature;
}

double CompositionalConfig::standardPressure() const {
    return this->standard_pressure;
}

const std::vector<std::string>& CompositionalConfig::compName() const {
    return this->comp_names;
}

CompositionalConfig::EOSType CompositionalConfig::eosType(size_t eos_region) const {
    return this->eos_types[eos_region];
}

const std::vector<double>& CompositionalConfig::molecularWeights(std::size_t eos_region) const {
    return this->molecular_weights[eos_region];
}

const std::vector<double>& CompositionalConfig::acentricFactors(size_t eos_region) const {
    return this->acentric_factors[eos_region];
}

const std::vector<double>& CompositionalConfig::criticalPressure(size_t eos_region) const {
    return this->critical_pressure[eos_region];
}

const std::vector<double>& CompositionalConfig::criticalTemperature(size_t eos_region) const {
    return this->critical_temperature[eos_region];
}

const std::vector<double>& CompositionalConfig::criticalVolume(size_t eos_region) const {
    return this->critical_volume[eos_region];
}

const std::vector<double>& CompositionalConfig::binaryInteractionCoefficient(size_t eos_region) const {
    return this->binary_interaction_coefficient[eos_region];
}

}