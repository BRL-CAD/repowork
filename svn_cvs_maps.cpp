/*             S V N _ C V S _ M A P S . C P P
 * BRL-CAD
 *
 * Published in 2020 by the United States Government.
 * This work is in the public domain.
 *
 */
/** @file svn_cvs_maps.cpp
 *
 * Utility functions
 *
 */

#include <iostream>
#include <sstream>
#include <locale>

#include "repowork.h"

int
git_map_svn_committers(git_fi_data *s, std::string &svn_map)
{
    // read map
    std::ifstream infile(svn_map, std::ifstream::binary);
    if (!infile.good()) {
	std::cerr << "Could not open svn_map file: " << svn_map << "\n";
	exit(-1);
    }

    // Create mapping of ids to svn committers 
    std::map<std::string, std::string> svn_committer_map;
    std::string line;
    while (std::getline(infile, line)) {
	// Skip empty lines
	if (!line.length()) {
	    continue;
	}

	size_t spos = line.find_first_of(" ");
	if (spos == std::string::npos) {
	    std::cerr << "Invalid svn map line!: " << line << "\n";
	    exit(-1);
	}

	std::string id = line.substr(0, spos);
	std::string committer = line.substr(spos+1, std::string::npos);

	svn_committer_map[id] = committer;
    }

    // Iterate over the commits and assign committers.
    for (size_t i = 0; i < s->commits.size(); i++) {
	git_commit_data *c = &(s->commits[i]);
	if (!c->svn_id.length()) {
	    continue;
	}
	if (svn_committer_map.find(c->svn_id) != svn_committer_map.end()) {
	    std::string svncommitter = svn_committer_map[c->svn_id];
	    //std::cerr << "Found SVN commit \"" << c->svn_id << "\" with committer \"" << svncommitter << "\"\n";
	    c->svn_committer = svncommitter;
	}
    }

    return 0;
}

void
read_key_sha1_map(git_fi_data *s, std::string &keysha1file)
{
    std::ifstream infile(keysha1file, std::ifstream::binary);
    if (!infile.good()) {
        std::cerr << "Could not open file: " << keysha1file << "\n";
        exit(-1);
    }
    std::string line;
    while (std::getline(infile, line)) {
        if (!line.length()) continue;
        size_t cpos = line.find_first_of(":");
        std::string key = line.substr(0, cpos);
        std::string sha1 = line.substr(cpos+1, std::string::npos);
	s->sha12key[sha1] = key;
	s->key2sha1[key] = sha1;
    }
    infile.close();
}

void
read_key_cvsbranch_map(
	git_fi_data *s,
        std::string &branchfile)
{
    std::map<std::string, std::string> key2branch;
    std::ifstream infile(branchfile, std::ifstream::binary);
    if (!infile.good()) {
        std::cerr << "Could not open file: " << branchfile << "\n";
        exit(-1);
    }
    std::string line;
    while (std::getline(infile, line)) {
        if (!line.length()) continue;
        size_t cpos = line.find_first_of(":");
        std::string key = line.substr(0, cpos);
        std::string branch = line.substr(cpos+1, std::string::npos);
        if (key2branch.find(key) != key2branch.end()) {
            std::string oldbranch = key2branch[key];
            if (oldbranch != branch) {
                std::cout << "WARNING: non-unique key maps to both branch " << oldbranch << " and branch "  << branch << ", overriding\n";
            }
        }
	if (s->key2sha1.find(key) != s->key2sha1.end()) {
	    s->key2cvsbranch[key] = branch;
	}
    }
    infile.close();
}

void
read_key_cvsauthor_map(	git_fi_data *s, std::string &authorfile)
{
    std::map<std::string, std::string> key2author;
    std::ifstream infile(authorfile, std::ifstream::binary);
    if (!infile.good()) {
        std::cerr << "Could not open file: " << authorfile << "\n";
        exit(-1);
    }
    std::string line;
    while (std::getline(infile, line)) {
        if (!line.length()) continue;
        size_t cpos = line.find_first_of(":");
        std::string key = line.substr(0, cpos);
        std::string author = line.substr(cpos+1, std::string::npos);
        if (key2author.find(key) != key2author.end()) {
            std::string oldauthor = key2author[key];
            if (oldauthor != author) {
                std::cout << "WARNING: non-unique key maps to both author " << oldauthor << " and author "  << author << ", overriding\n";
            }
        }
	if (s->key2sha1.find(key) != s->key2sha1.end()) {
	    s->key2cvsauthor[key] = author;
	}
    }
    infile.close();
}

// Local Variables:
// tab-width: 8
// mode: C++
// c-basic-offset: 4
// indent-tabs-mode: t
// c-file-style: "stroustrup"
// End:
// ex: shiftwidth=4 tabstop=8
