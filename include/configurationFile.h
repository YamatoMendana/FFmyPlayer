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

// ��ȡ����ģ�壬�� ptree �е��������Ϊ��������
template<typename T>
T ConfigureFile::read(const char* key)
{
	return pt.get<T>(key);
}

// ģ�庯������ȡ INI �ļ��е�ֵ
template <typename T>
T ConfigureFile::readValue(const std::string& section, const std::string& key, const T& defaultValue) const {
	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(strFilePath, pt);
	return pt.get<T>(section + "." + key, defaultValue);
}


// ģ�庯����д�� INI �ļ��е�ֵ
template <typename T>
void ConfigureFile::writeValue(const std::string& section, const std::string& key, const T& value) const {
	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(strFilePath, pt);
	pt.put(section + "." + key, value);
	boost::property_tree::write_ini(strFilePath, pt);
}


// ģ�庯�����޸� INI �ļ��е�ֵ
template <typename T>
void ConfigureFile::modifyValue(const std::string& section, const std::string& key, const T& newValue) const {
	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(strFilePath, pt);
	pt.put(section + "." + key, newValue);
	boost::property_tree::write_ini(strFilePath, pt);
}


// ģ�庯����ɾ�� INI �ļ��е�ֵ
void ConfigureFile::deleteValue(const std::string& section, const std::string& key) const {
	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(strFilePath, pt);
	pt.erase(section + "." + key);
	boost::property_tree::write_ini(strFilePath, pt);
}


#endif//__CONFIGURATION_FILE_H__




