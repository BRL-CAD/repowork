/*               S V N _ C V S _ M S G S . C P P
 * BRL-CAD
 *
 * Published in 2020 by the United States Government.
 * This work is in the public domain.
 *
 */
/** @file repowork.cpp
 *
 * Utility functions and main processing loop
 *
 */

#include <iostream>
#include <sstream>
#include <locale>

#include "repowork.h"

// https://stackoverflow.com/a/5607650
struct schars: std::ctype<char> {
    schars(): std::ctype<char>(get_table()) {}
    static std::ctype_base::mask const* get_table() {
        static const std::ctype<char>::mask *const_table= std::ctype<char>::classic_table();
        static std::ctype<char>::mask cmask[std::ctype<char>::table_size];
        std::memcpy(cmask, const_table, std::ctype<char>::table_size * sizeof(std::ctype<char>::mask));
        cmask[';'] = std::ctype_base::space;
        return &cmask[0];
    }
};

void
parse_cvs_svn_info(git_commit_data *c, std::string &str)
{
    std::stringstream ss(str);
    std::string cline;
    std::regex svnrevline("^svn:revision:([0-9]+).*");
    std::regex svnbranchline("^svn:branch:(.*)");
    std::regex svntagline("^svn:tag:(.*)");
    std::regex svncommitterline("^svn:account:(.*)");
    std::regex cvsbranchline("^cvs:branch:(.*)");
    std::regex cvscommitterline("^cvs:account:(.*)");

    c->svn_branches.clear();
    c->cvs_branches.clear();

    while (std::getline(ss, cline, '\n')) {
	if (std::regex_match(cline, svnrevline)) {
	    c->svn_id = cline.substr(13, std::string::npos);
	    continue;
	}
	if (std::regex_match(cline, svnbranchline)) {
	    std::string nbranch = cline.substr(11, std::string::npos);
	    if (nbranch == std::string("master")) {
		nbranch = std::string("trunk");
	    }
	    c->svn_branches.insert(nbranch);
	    continue;
	}
	if (std::regex_match(cline, svntagline)) {
	    std::string ntag = cline.substr(8, std::string::npos);
	    if (ntag == std::string("master")) {
		ntag = std::string("trunk");
	    }
	    c->svn_tags.insert(ntag);
	    continue;
	}
	if (std::regex_match(cline, svncommitterline)) {
	    c->svn_committer = cline.substr(12, std::string::npos);
	    continue;
	}
	if (std::regex_match(cline, cvsbranchline)) {
	    std::string nbranch = cline.substr(11, std::string::npos);
	    if (nbranch == std::string("master")) {
		nbranch = std::string("trunk");
	    }
	    c->cvs_branches.insert(nbranch);
	    continue;
	}
	if (std::regex_match(cline, cvscommitterline)) {
	    c->cvs_committer = cline.substr(12, std::string::npos);
	    continue;
	}
    }
}

void
update_commit_msg(git_commit_data *c)
{
    // First, get a version of the commit message without any svn or cvs info.
    std::set<std::string>::iterator s_it;
    std::regex cvsline("^cvs:.*");
    std::regex svnline("^svn:.*");
    std::string cline;
    std::string nmsg;
    std::stringstream css(c->commit_msg);
    while (std::getline(css, cline, '\n')) {
	bool smatch = std::regex_match(cline, svnline);
	bool cmatch = std::regex_match(cline, cvsline);
	if (smatch || cmatch) {
	    // If we do already have CVS/SVN info, don't write the last blank
	    // spacer line - it was inserted to separate it from the actual
	    // message.
	    nmsg.pop_back();
	    break;
	}
	nmsg.append(cline);
	nmsg.append("\n");
    }

    // If we have any SVN or CVS info, insert a blank line:
    if ((c->svn_id.length() && c->svn_id != std::string("-1")) || c->svn_branches.size() || c->svn_tags.size() || c->svn_committer.length() || c->cvs_branches.size() || c->cvs_committer.length() ) {
	nmsg.append("\n");
    }

    // Add all the info we have
    if (c->svn_id.length() && c->svn_id != std::string("-1")) {
	std::string ninfo = std::string("svn:revision:") + c->svn_id;
	nmsg.append(ninfo);
	nmsg.append("\n");
    }
    for (s_it = c->svn_branches.begin(); s_it != c->svn_branches.end(); s_it++) {
	std::string ninfo = std::string("svn:branch:") + *s_it;
	nmsg.append(ninfo);
	nmsg.append("\n");
    }
    for (s_it = c->svn_tags.begin(); s_it != c->svn_tags.end(); s_it++) {
	std::string ninfo = std::string("svn:tag:") + *s_it;
	nmsg.append(ninfo);
	nmsg.append("\n");
    }
    if (c->svn_committer.length()) {
	std::string ninfo = std::string("svn:account:") + c->svn_committer;
	nmsg.append(ninfo);
	nmsg.append("\n");
    }
    for (s_it = c->cvs_branches.begin(); s_it != c->cvs_branches.end(); s_it++) {
	std::string ninfo = std::string("cvs:branch:") + *s_it;
	nmsg.append(ninfo);
	nmsg.append("\n");
    }
    if (c->cvs_committer.length()) {
	std::string ninfo = std::string("cvs:account:") + c->cvs_committer;
	nmsg.append(ninfo);
	nmsg.append("\n");
    }

    c->commit_msg = nmsg;
}


// This is the first step of the note correction process - run it first
int
git_update_svn_revs(git_fi_data *s, std::string &svn_rev_map)
{
    if (!s->have_sha1s) {
	std::cerr << "Fatal - sha1 SVN note updating requested, but don't have original sha1 ids - redo fast-export with the --show-original-ids option.\n";
	exit(1);
    }
    // read maps
    std::ifstream infile_revs(svn_rev_map, std::ifstream::binary);
    if (svn_rev_map.length() && !infile_revs.good()) {
	std::cerr << "Could not open svn_rev_map file: " << svn_rev_map << "\n";
	exit(-1);
    }
    std::map<std::string, int> rmap;
    if (infile_revs.good()) {
	std::string line;
	while (std::getline(infile_revs, line)) {
	    // Skip empty lines
	    if (!line.length()) {
		continue;
	    }

	    size_t spos = line.find_first_of(";");
	    if (spos == std::string::npos) {
		std::cerr << "Invalid sha1;rev map line!: " << line << "\n";
		exit(-1);
	    }

	    std::string id1 = line.substr(0, spos);
	    std::string id2 = line.substr(spos+1, std::string::npos);
	    int rev = (id2.length()) ? std::stoi(id2) : -1;
	    rmap[id1] = rev;
	}

	infile_revs.close();
    }

    for (size_t i = 0; i < s->commits.size(); i++) {

	if (!s->commits[i].id.sha1.length()) {
	    continue;
	}
	if (rmap.find(s->commits[i].id.sha1) == rmap.end()) {
	    continue;
	}

	long nrev =  rmap[s->commits[i].id.sha1];
	s->commits[i].svn_id = std::to_string(nrev);

	if (nrev > 0) {
	    std::cout << "Assigning new SVN rev " << nrev << " to " << s->commits[i].id.sha1 << "\n";
	    // Note:  this isn't guaranteed to be unique...  setting it mostly for
	    // the cases where it is.
	    s->rev_to_sha1[s->commits[i].svn_id] = s->commits[i].id.sha1;
	    update_commit_msg(&s->commits[i]);
	}

    }

    return 0;
}


// This function is intended to handle map files with either sha1 or svn rev keys,
// and multiple branches mapping to one key either on multiple lines or by semicolon
// separated lists.
//
// If update_mode == 0, clear branch lines from the message when we don't have map
// information.  If update_mode == 1, leave intact anything not explicitly in the map.
int
git_assign_branch_labels(git_fi_data *s, std::string &svn_branch_map, int update_mode)
{
    int key_type = -1;

    if (!s->have_sha1s) {
	std::cerr << "Fatal - sha1 SVN note updating requested, but don't have original sha1 ids - redo fast-export with the --show-original-ids option.\n";
	exit(1);
    }

    std::ifstream infile_branches(svn_branch_map, std::ifstream::binary);
    if (svn_branch_map.length() && !infile_branches.good()) {
	std::cerr << "Could not open svn_branch_map file: " << svn_branch_map << "\n";
	exit(-1);
    }
    std::map<std::string, std::set<std::string>> bmap;
    if (infile_branches.good()) {
	std::string line;
	while (std::getline(infile_branches, line)) {
	    // Skip empty lines
	    if (!line.length()) {
		continue;
	    }

	    size_t spos = line.find_first_of(":");
	    if (spos == std::string::npos) {
		std::cerr << "Invalid sha1;branch map line!: " << line << "\n";
		exit(-1);
	    }

	    std::string id1 = line.substr(0, spos);
	    std::string id2 = line.substr(spos+1, std::string::npos);

	    if (key_type < 0) {
		key_type = (id1.length() == 40) ? 1 : 2;
	    }

	    std::cout << "key: \"" << id1 << "\" -> branch: \"" << id2 << "\n";

	    // Split into a vector, since there may be more than one branch
	    std::stringstream ss(id2);
	    std::ostringstream oss;
	    ss.imbue(std::locale(std::locale(), new schars()));
	    std::istream_iterator<std::string> b_begin(ss);
	    std::istream_iterator<std::string> b_end;
	    std::vector<std::string> branches_array(b_begin, b_end);
	    std::copy(branches_array.begin(), branches_array.end(), std::ostream_iterator<std::string>(oss, "\n"));
	    for (size_t i = 0; i < branches_array.size(); i++) {
		bmap[id1].insert(branches_array[i]);
	    }
	}

	infile_branches.close();
    }

    // Iterate over the commits looking for relevant commits, and update msg.
    for (size_t i = 0; i < s->commits.size(); i++) {
	long int rev = -1;
	if (s->commits[i].svn_id.length()) {
	    rev = std::stol(s->commits[i].svn_id);
	}
	if (update_mode == 0 && s->commits[i].svn_id.length()) {
	    // If we're in overwrite mode, don't go beyond the CVS era commits - 
	    // SVN era commits were assigned branches in the original process,
	    // and any alterations to them should be corrections.  The CVS era
	    // assignments were unreliable, and so should be removed.
	    if (rev > 29886) {
		continue;
	    }
	}

	std::set<std::string> sbranches;
	if (key_type == 1) {
	    if (update_mode == 1 && !s->commits[i].id.sha1.length())
		continue;
	    if (update_mode == 1 && bmap.find(s->commits[i].id.sha1) == bmap.end())
		continue;
	    sbranches = bmap[s->commits[i].id.sha1];
	}

	if (key_type == 2) {
	    if (update_mode == 1 && !s->commits[i].svn_id.length())
		continue;
	    if (update_mode == 1 && bmap.find(s->commits[i].svn_id) == bmap.end())
		continue;
	    sbranches = bmap[s->commits[i].svn_id];
	}

	if (rev > 29886) {
	    s->commits[i].svn_branches.clear();
	    s->commits[i].svn_branches = sbranches;
	} else {
	    s->commits[i].cvs_branches.clear();
	    s->commits[i].cvs_branches = sbranches;
	}

	// Update the message
	update_commit_msg(&s->commits[i]);
    }

    return 0;
}

int
git_set_tag_labels(git_fi_data *s, std::string &tag_list)
{
    if (!s->have_sha1s) {
	std::cerr << "Fatal - sha1 SVN note updating requested, but don't have original sha1 ids - redo fast-export with the --show-original-ids option.\n";
	exit(1);
    }

    std::ifstream infile_tag_list(tag_list, std::ifstream::binary);
    if (tag_list.length() && !infile_tag_list.good()) {
	std::cerr << "Could not open tag_list file: " << tag_list << "\n";
	exit(-1);
    }
    std::set<std::string> tag_sha1s;
    if (infile_tag_list.good()) {
	std::string line;
	while (std::getline(infile_tag_list, line)) {
	    // Skip anything the wrong length
	    if (line.length() != 40)
		continue;
	    tag_sha1s.insert(line);
	    std::cout << "tag sha1: " << line << "\n";
	    bool valid = (s->sha1_to_mark.find(line) != s->sha1_to_mark.end());
	    if (!valid) {
		std::cout << "INVALID sha1 supplied for tag!\n";
	    }
	    git_commit_data &c = s->commits[s->mark_to_index[s->sha1_to_mark[line]]];
	    c.svn_tags = c.svn_branches;
	    c.svn_branches.clear();
	    update_commit_msg(&c);
	}

	infile_tag_list.close();
    }
    return 0;
}


// Local Variables:
// tab-width: 8
// mode: C++
// c-basic-offset: 4
// indent-tabs-mode: t
// c-file-style: "stroustrup"
// End:
// ex: shiftwidth=4 tabstop=8
