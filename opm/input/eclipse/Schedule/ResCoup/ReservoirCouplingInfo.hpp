/*
  Copyright 2024 Equinor ASA.

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
#ifndef RESERVOIR_COUPLING_INFO_HPP
#define RESERVOIR_COUPLING_INFO_HPP

#include <opm/input/eclipse/Schedule/ResCoup/Slaves.hpp>
#include <opm/input/eclipse/Schedule/ResCoup/GrupSlav.hpp>
#include <opm/input/eclipse/Schedule/ResCoup/MasterGroup.hpp>
#include <opm/io/eclipse/rst/group.hpp>
#include <opm/io/eclipse/rst/well.hpp>

#include <map>
#include <string>

namespace Opm::ReservoirCoupling {

class CouplingInfo {
public:
    enum class CouplingFileFlag {
        NONE,
        FORMATTED,
        UNFORMATTED
    };

    CouplingInfo() = default;

    static CouplingInfo serializationTestObject();
    bool operator==(const CouplingInfo& other) const;

    void couplingFileFlag(CouplingFileFlag flag) {
        m_coupling_file_flag = flag;
    }
    CouplingFileFlag couplingFileFlag() const {
        return m_coupling_file_flag;
    }

    const GrupSlav& grupSlav(const std::string& name) const {
        return m_grup_slavs.at(name);
    }
    const std::map<std::string, GrupSlav>& grupSlavs() const {
        return this->m_grup_slavs;
    }
    std::map<std::string, GrupSlav>& grupSlavs() {
        return this->m_grup_slavs;
    }
    int grupSlavCount() const {
        return this->m_grup_slavs.size();
    }

    bool hasGrupSlav(const std::string& name) const {
        return m_grup_slavs.find(name) != m_grup_slavs.end();
    }

    bool hasMasterGroup(const std::string& name) const {
        return m_master_groups.find(name) != m_master_groups.end();
    }
    bool hasSlave(const std::string& name) const {
        return m_slaves.find(name) != m_slaves.end();
    }

    const std::map<std::string, MasterGroup>& masterGroups() const {
        return this->m_master_groups;
    }
    std::map<std::string, MasterGroup>& masterGroups() {
        return this->m_master_groups;
    }
    const MasterGroup& masterGroup(const std::string& name) const {
        return m_master_groups.at(name);
    }

    int masterGroupCount() const {
        return m_master_groups.size();
    }

    double masterMinTimeStep() const {
        return m_master_min_time_step;
    }
    void setMasterMinTimeStep(double tstep) {
        m_master_min_time_step = tstep;
    }

    const std::map<std::string, Slave>& slaves() const {
        return this->m_slaves;
    }
    std::map<std::string, Slave>& slaves() {
        return this->m_slaves;
    }
    const Slave& slave(const std::string& name) const {
        return m_slaves.at(name);
    }
    int slaveCount() const {
        return m_slaves.size();
    }


    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_slaves);
        serializer(m_master_groups);
        serializer(m_grup_slavs);
        serializer(m_master_min_time_step);
        serializer(m_coupling_file_flag);
    }

private:
    std::map<std::string, Slave> m_slaves;
    std::map<std::string, MasterGroup> m_master_groups;
    std::map<std::string, GrupSlav> m_grup_slavs;
    double m_master_min_time_step{0.0};
    CouplingFileFlag m_coupling_file_flag{CouplingFileFlag::NONE};
};

} // namespace Opm::ReservoirCoupling

#endif
