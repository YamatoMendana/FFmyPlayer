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

	boost::property_tree::ptree pt;
	// ��ȡ����ģ�壬�� ptree �е��������Ϊ��������
	template<typename T>
	T ConfigureFile::read(const char* key)
	{
		return pt.get<T>(key);
	}

	// ģ�庯������ȡ INI �ļ��е�ֵ
	template <typename T>
	T readValue(const std::string& section, const std::string& key, const T& defaultValue) const {
		boost::property_tree::ptree pt;
		boost::property_tree::read_ini(filePath, pt);
		return pt.get<T>(section + "." + key, defaultValue);
	}

	// ģ�庯����д�� INI �ļ��е�ֵ
	template <typename T>
	void writeValue(const std::string& section, const std::string& key, const T& value) const {
		boost::property_tree::ptree pt;
		boost::property_tree::read_ini(filePath, pt);
		pt.put(section + "." + key, value);
		boost::property_tree::write_ini(filePath, pt);
	}

	// ģ�庯�����޸� INI �ļ��е�ֵ
	template <typename T>
	void modifyValue(const std::string& section, const std::string& key, const T& newValue) const {
		boost::property_tree::ptree pt;
		boost::property_tree::read_ini(filePath, pt);
		pt.put(section + "." + key, newValue);
		boost::property_tree::write_ini(filePath, pt);
	}

	// ģ�庯����ɾ�� INI �ļ��е�ֵ
	void deleteValue(const std::string& section, const std::string& key) const {
		boost::property_tree::ptree pt;
		boost::property_tree::read_ini(filePath, pt);
		pt.erase(section + "." + key);
		boost::property_tree::write_ini(filePath, pt);
	}

private:
	string strFilePath;
};




#endif//__CONFIGURATION_FILE_H__




