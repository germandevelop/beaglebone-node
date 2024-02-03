/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include "ConfigParser.hpp"

#include <boost/variant.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>


using BoostGeneralData = boost::variant<bool, unsigned int, int, double, std::string>;

class BoostGeneralVisitor : public boost::static_visitor<>
{
public:
    BoostGeneralVisitor(GeneralData& generalData) : generalValue{ generalData }
    {
        return;
    }

    template <typename T>
    void operator()(T &value) const
    {
        this->generalValue = value;

        return;
    }

private:
    GeneralData &generalValue;
};


ConfigList ConfigParser::readInitialization(std::string fileName)
{
    ConfigList configList;

    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(fileName, propertyTree);

    for (auto itr = propertyTree.begin(); itr != propertyTree.end(); ++itr) {

        Config config;

        for (auto jtr = itr->second.begin(); jtr != itr->second.end(); ++jtr) {

            // Convert boost::variant into std::variant
            GeneralData generalValue;
            BoostGeneralData boostGeneralValue = jtr->second.data();
            boost::apply_visitor(BoostGeneralVisitor{ generalValue }, boostGeneralValue);

            config.emplace(jtr->first, generalValue);
        }
        configList.emplace(itr->first, std::move(config));
    }

    return configList;
}

void ConfigParser::writeInitialization(std::string fileName, ConfigList configList)
{
    boost::property_tree::ptree propertyTree;

    for (auto itr = std::cbegin(configList); itr != std::cend(configList); ++itr) {

        for (auto jtr = std::cbegin(itr->second); jtr != std::cend(itr->second); ++jtr) {

            // Convert std::variant into boost::variant
            BoostGeneralData generalValue;
            std::visit([&generalValue] (auto const& value) { generalValue = value; }, jtr->second);

            propertyTree.put(itr->first + "." + jtr->first, generalValue);
        }
    }

    boost::property_tree::ini_parser::write_ini(fileName, propertyTree);

    return;
}
