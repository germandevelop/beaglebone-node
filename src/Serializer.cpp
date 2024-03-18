/************************************************************
 *   Author : German Mundinger
 *   Date   : 2019
 ************************************************************/

#include "Serializer.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>


namespace Serializer
{
    boost::property_tree::ptree writePropertyTree (const DataArray &data);
    DataArray readPropertyTree (const boost::property_tree::ptree &tree);
}

std::string Serializer::Ini::serialize (const Serializer::DataArray &data)
{
    boost::property_tree::ptree propertyTree = Serializer::writePropertyTree(data);

    std::ostringstream stringStream;
    boost::property_tree::ini_parser::write_ini(stringStream, propertyTree);

    std::string rawData = stringStream.str();

    return rawData;
}

Serializer::DataArray Serializer::Ini::deserialize (const std::string &rawData)
{
    std::istringstream stringStream { rawData };

    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(stringStream, propertyTree);

    Serializer::DataArray data = Serializer::readPropertyTree(propertyTree);

    return data;
}

void Serializer::Ini::save (const Serializer::DataArray &data, const std::filesystem::path &file)
{
    boost::property_tree::ptree propertyTree = Serializer::writePropertyTree(data);

    boost::property_tree::ini_parser::write_ini(file.string(), propertyTree);

    return;
}

Serializer::DataArray Serializer::Ini::load (const std::filesystem::path &file)
{
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(file.string(), propertyTree);

    Serializer::DataArray data = Serializer::readPropertyTree(propertyTree);

    return data;
}


std::string Serializer::Json::serialize (const Serializer::DataArray &data, bool isPretty)
{
    boost::property_tree::ptree propertyTree = Serializer::writePropertyTree(data);

    std::ostringstream stringStream;
    boost::property_tree::json_parser::write_json(stringStream, propertyTree, isPretty);

    std::string rawData = stringStream.str();

    return rawData;
}

Serializer::DataArray Serializer::Json::deserialize (const std::string &rawData)
{
    std::istringstream stringStream { rawData };

    boost::property_tree::ptree propertyTree;
    boost::property_tree::json_parser::read_json(stringStream, propertyTree);

    Serializer::DataArray data = Serializer::readPropertyTree(propertyTree);

    return data;
}

void Serializer::Json::save (const Serializer::DataArray &data, const std::filesystem::path &file)
{
    boost::property_tree::ptree propertyTree = Serializer::writePropertyTree(data);

    boost::property_tree::json_parser::write_json(file.string(), propertyTree);

    return;
}

Serializer::DataArray Serializer::Json::load (const std::filesystem::path &file)
{
    boost::property_tree::ptree propertyTree;
    boost::property_tree::json_parser::read_json(file.string(), propertyTree);

    Serializer::DataArray data = Serializer::readPropertyTree(propertyTree);

    return data;
}



boost::property_tree::ptree Serializer::writePropertyTree (const Serializer::DataArray &data)
{
    boost::property_tree::ptree propertyTree;

    for (auto itr = std::cbegin(data); itr != std::cend(data); ++itr)
    {
        const auto &[key, value] = *itr;

        if (std::holds_alternative<int>(value) == true)
        {
            propertyTree.put(key, std::get<int>(value));
        }
        else if (std::holds_alternative<float>(value) == true)
        {
            propertyTree.put(key, std::get<float>(value));
        }
        else if (std::holds_alternative<std::string>(value) == true)
        {
            propertyTree.put(key, std::get<std::string>(value));
        }
    }
    return propertyTree;
}

Serializer::DataArray Serializer::readPropertyTree (const boost::property_tree::ptree &tree)
{
    Serializer::DataArray dataArray;

    for (auto itr = boost::const_begin(tree); itr != boost::const_end(tree); ++itr)
    {
        const auto &[key, value] = *itr;

        Serializer::Data data;

        bool isCasted = false;

        if (isCasted != true)
        {
            try
            {
                data = boost::lexical_cast<int>(value.data());
                isCasted = true;
            }
            catch (const boost::bad_lexical_cast &exp)
            {
                // Do nothing
            }
        }

        if (isCasted != true)
        {
            try
            {
                data = boost::lexical_cast<float>(value.data());
                isCasted = true;
            }
            catch (const boost::bad_lexical_cast &exp)
            {
                // Do nothing
            }
        }

        if (isCasted != true)
        {
            try
            {
                data = boost::lexical_cast<std::string>(value.data());
            }
            catch (const boost::bad_lexical_cast &exp)
            {
                // Do nothing
            }
        }

        dataArray.emplace(key, data);
    }

    return dataArray;
}
