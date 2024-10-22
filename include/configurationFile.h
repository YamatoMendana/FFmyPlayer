#pragma once
#ifndef __CONFIGURATION_FILE_H__
#define __CONFIGURATION_FILE_H__

#include <iostream>
#include <stdio.h>
#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace std;

class ConfigureFile
{
public:
	ConfigureFile(const std::string& filePath) : strFilePath(filePath) {}
	template<typename T> T read(const char* key);
	template <typename T> T readValue(const std::string& section, const std::string& key, const T& defaultValue) const;
	template <typename T> void writeValue(const std::string& section, const std::string& key, const T& value) const;
	template <typename T> void modifyValue(const std::string& section, const std::string& key, const T& newValue) const;
	void deleteValue(const std::string& section, const std::string& key) const;
private:
	boost::property_tree::ptree pt;
	string strFilePath;
};

// 读取函数模板，将 ptree 中的数据输出为任意类型
template<typename T>
T ConfigureFile::read(const char* key)
{
	return pt.get<T>(key);
}

// 模板函数，读取 INI 文件中的值
template <typename T>
T ConfigureFile::readValue(const std::string& section, const std::string& key, const T& defaultValue) const {
	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(strFilePath, pt);
	return pt.get<T>(section + "." + key, defaultValue);
}


// 模板函数，写入 INI 文件中的值
template <typename T>
void ConfigureFile::writeValue(const std::string& section, const std::string& key, const T& value) const {
	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(strFilePath, pt);
	pt.put(section + "." + key, value);
	boost::property_tree::write_ini(strFilePath, pt);
}


// 模板函数，修改 INI 文件中的值
template <typename T>
void ConfigureFile::modifyValue(const std::string& section, const std::string& key, const T& newValue) const {
	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(strFilePath, pt);
	pt.put(section + "." + key, newValue);
	boost::property_tree::write_ini(strFilePath, pt);
}


// 模板函数，删除 INI 文件中的值
void ConfigureFile::deleteValue(const std::string& section, const std::string& key) const {
	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(strFilePath, pt);
	pt.erase(section + "." + key);
	boost::property_tree::write_ini(strFilePath, pt);
}


#endif//__CONFIGURATION_FILE_H__




