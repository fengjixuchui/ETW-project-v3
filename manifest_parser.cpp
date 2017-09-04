#include <fstream>
#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "manifest_parser.h"


using namespace std;

using namespace boost::property_tree;
using namespace boost::property_tree::xml_parser;

Manifest_parser::Manifest_parser() {
	
}

Manifest_parser::Manifest_parser(string manifest_file) {
	get_manifest_ptree(manifest_file);
	parser_manifest_ptree();
}

Manifest_parser::~Manifest_parser() {

}

void Manifest_parser::get_manifest_ptree(string manifest_file) {
	ifstream manifest_file_stream;
	manifest_file_stream.open(manifest_file);

	read_xml(manifest_file_stream, manifest_ptree);

	manifest_file_stream.close();
}

void Manifest_parser::parser_manifest_ptree() {
	try{
		manifest_ptree.get_child("instrumentationManifest.instrumentation");
	}
	catch (...){
		cout << "get_child() failed!" << endl;
		return;
	}
	cout << "get_child() successful!" << endl;

	for (ptree::const_iterator it = manifest_ptree.begin(); it != manifest_ptree.end(); it++) {
		cout << it->first << "-" << it->second.data() << endl;
	}

	return;
}