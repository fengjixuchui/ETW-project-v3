#pragma once

#include <boost/property_tree/ptree.hpp>
#include <unordered_map>


using namespace std;

class Manifest_parser {
public:
	Manifest_parser();
	Manifest_parser(string manifest_file);

	~Manifest_parser();

	// get the manifest ptree from manifest file
	void get_manifest_ptree(string manifest_file);
	// parser the manifest ptree and get the data struct we need to parser the etw trace blob
	void parser_manifest_ptree();

private:
	boost::property_tree::ptree manifest_ptree;

	//unordered_map<string, 
};
