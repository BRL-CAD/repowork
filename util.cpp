/*                    U T I L . C P P
 * BRL-CAD
 *
 * Published in 2020 by the United States Government.
 * This work is in the public domain.
 *
 */
/** @file util.cpp
 *
 * Utility functions
 *
 */

#include <iostream>
#include <sstream>
#include <locale>

#include "repowork.h"


int
git_parse_commitish(git_commitish &gc, git_fi_data *s, std::string line)
{
       if (line.c_str()[0] == ':') {
        // If we start with a colon, we have a mark - translate it and zero
        // from_str.
        line.erase(0, 1); // Remove ":" prefix
	long omark = std::stol(line);
        gc.mark = s->mark_old_to_new[omark];
	if (s->mark_to_index.find(gc.mark) != s->mark_to_index.end()) {
	    gc.index = s->mark_to_index[gc.mark];
	    //std::cout << "Mark id :" << line << " -> " << gc.index << "\n";
	} else {
	    std::cerr << "Mark with no index:" << gc.mark << "\n";
	    exit(EXIT_FAILURE);
	}
        return 0;
    }
    if (!ficmp(line, std::string("refs/heads/"))) {
        gc.ref = std::stol(line);
        return 0;
    }
    if (line.length() == 40) {
        // Probably have a SHA1
        gc.sha1 = line;
        gc.index = s->mark_to_index[s->sha1_to_mark[gc.sha1]];
        //std::cout << "SHA1 id :" << gc.sha1 << " -> " << gc.mark << " -> " << gc.index << "\n";
        return 0;
    }

    return -1;
}

int
git_remove_commits(git_fi_data *s, std::string &remove_commits)
{
    if (!s->have_sha1s) {
	std::cerr << "Fatal - commit removal requested, but don't have original sha1 ids - redo fast-export with the --show-original-ids option.\n";
	exit(1);
    }

    std::ifstream infile_remove_commits(remove_commits, std::ifstream::binary);
    if (remove_commits.length() && !infile_remove_commits.good()) {
	std::cerr << "Could not open remove_commits file: " << remove_commits << "\n";
	exit(-1);
    }
    std::set<std::string> remove_sha1s;
    if (infile_remove_commits.good()) {
	std::string line;
	while (std::getline(infile_remove_commits, line)) {
	    // Skip anything the wrong length
	    if (line.length() != 40) {
		continue;
	    }
	    remove_sha1s.insert(line);
	    std::cout << "remove sha1: " << line << "\n";
	    bool valid = false;
	    for (size_t i = 0; i < s->commits.size(); i++) {
		if (s->commits[i].id.sha1 == line) {
		    valid = true;
		    break;
		}
	    }
	    if (!valid) {
		std::cout << "INVALID sha1 supplied for removal!\n";
	    }
	}

	infile_remove_commits.close();
    }


    std::set<std::string>::iterator r_it;
    for (r_it = remove_sha1s.begin(); r_it != remove_sha1s.end(); r_it++) {
	git_commitish rfrom;
	git_commitish rish;
	for (size_t i = 0; i < s->commits.size(); i++) {
	    if (s->commits[i].id.sha1 == *r_it) {
		rfrom = s->commits[i].from;
		rish = s->commits[i].id;
		s->commits[i].skip_commit = true;
		break;
	    }
	}
	// Update any references
	for (size_t i = 0; i < s->commits.size(); i++) {
	    if (s->commits[i].from == rish) {
		std::cout << *r_it << " removal: updating from commit for " << s->commits[i].id.sha1 << "\n";
		s->commits[i].from = rfrom;
	    }
	    for (size_t j = 0; j < s->commits[i].merges.size(); j++) {
		if (s->commits[i].merges[j] == rish) {
		    std::cout << *r_it << " removal: updating merge commit for " << s->commits[i].id.sha1 << "\n";
		    s->commits[i].merges[j] = rfrom;
		}
	    }
	}
    }

    return 0;
}

int
git_map_emails(git_fi_data *s, std::string &email_map)
{
    // read map
    std::ifstream infile(email_map, std::ifstream::binary);
    if (!infile.good()) {
	std::cerr << "Could not open email_map file: " << email_map << "\n";
	exit(-1);
    }

    std::map<std::string, std::string> email_id_map;

    std::string line;
    while (std::getline(infile, line)) {
	// Skip empty lines
	if (!line.length()) {
	    continue;
	}

	size_t spos = line.find_first_of(";");
	if (spos == std::string::npos) {
	    std::cerr << "Invalid email map line!: " << line << "\n";
	    exit(-1);
	}

	std::string id1 = line.substr(0, spos);
	std::string id2 = line.substr(spos+1, std::string::npos);

	std::cout << "id1: \"" << id1 << "\"\n";
	std::cout << "id2: \"" << id2 << "\"\n";
	email_id_map[id1] = id2;
    }

    // Iterate over the commits looking for note commits.  If we find one,
    // find its associated blob with data, read it, find the associated
    // commit, and stash it in a string in that container.
    for (size_t i = 0; i < s->commits.size(); i++) {
	git_commit_data *c = &(s->commits[i]);
	if (email_id_map.find(c->author) != email_id_map.end()) {
	    std::string nauthor = email_id_map[c->author];
	    c->author = nauthor;
	}
	if (email_id_map.find(c->committer) != email_id_map.end()) {
	    std::string ncommitter = email_id_map[c->committer];
	    //std::cerr << "Replaced committer \"" << c->committer << "\" with \"" << ncommitter << "\"\n";
	    c->committer = ncommitter;
	}
    }

    return 0;
}


void
process_ls_tree(std::string &sha1)
{
    // read children
    std::ifstream tfile("tree.txt", std::ifstream::binary);
    if (!tfile.good()) {
	std::cerr << "Could not open tree file tree.txt\n";
	exit(-1);
    }
    std::string sha1tree = std::string("trees/") + sha1 + std::string("-tree.fi");
    std::ofstream ofile(sha1tree, std::ios::out | std::ios::binary);
    ofile << "deleteall\n";

    std::string tline;
    while (std::getline(tfile, tline)) {
	std::string ltree = tline;
	std::regex bregex(" blob ");
	std::string ltree2 = std::regex_replace(ltree, bregex, " ");
	std::regex sregex("^");
	ltree = std::regex_replace(ltree2, sregex , "M ");
	std::regex tregex("\t");
	ltree2 = std::regex_replace(ltree, tregex , " \"");
	ofile << ltree2 << "\"\n";
    }

    ofile.close();

    std::remove("tree.txt");
}

int
git_id_rebuild_commits(git_fi_data *s, std::string &id_file, std::string &repo_path, std::string &child_commits_file)
{
    {
	// read children
	std::ifstream cfile(child_commits_file, std::ifstream::binary);
	if (!cfile.good()) {
	    std::cerr << "Could not open child_commits_file file: " << child_commits_file << "\n";
	    exit(-1);
	}

	std::string rline;
	while (std::getline(cfile, rline)) {
	    // Skip empty lines
	    if (!rline.length()) {
		continue;
	    }

	    // First 40 characters are the key
	    std::string key = rline.substr(0, 40);
	    rline.erase(0,41); // Remove key and space
	    std::set<std::string> vals;
	    while (rline.length() >= 40) {
		std::string val = rline.substr(0, 40);
		vals.insert(val);
		rline.erase(0,41);
	    }
	    if (vals.size()) {
		s->children[key] = vals;
	    }
	}
    }

    {
	// read ids
	std::ifstream infile(id_file, std::ifstream::binary);
	if (!infile.good()) {
	    std::cerr << "Could not open id_file file: " << id_file << "\n";
	    exit(-1);
	}

	std::string line;
	while (std::getline(infile, line)) {
	    // Skip empty lines
	    if (!line.length()) {
		continue;
	    }

	    std::string sha1;
	    if (line.length() < 40) {
		// Given an svn revision - translate it to a sha1
		if (s->rev_to_sha1.find(line) == s->rev_to_sha1.end()) {
		    std::cerr << "SVN revision " << line << " could not be mapped to SHA1.  May need to re-export fast import file with --show-original-ids.\n";
		    exit(1);
		}
		sha1 = s->rev_to_sha1[line];
	    } else {
		sha1 = line;
	    }

	    s->rebuild_commits.insert(sha1);
	    std::cout << "rebuild commit: " << line << " -> " << sha1 << "\n";
	}
    }

    // Children of the rebuilt commits will need to fully define their
    // contents, unless they are also being rebuilt (in which case their
    // children will need to reset themselves.)
    std::set<std::string> rbc = s->rebuild_commits;
    while (rbc.size()) {
	std::string rb = *rbc.begin();
	rbc.erase(rb);
	std::cout << "Finding reset commit(s) for: " << rb << "\n";
	if (s->children.find(rb) == s->children.end()) {
	    // No child commits - no further work needed.
	    std::cout << "Leaf commit: " << rb << "\n";
	    continue;
	}
	std::set<std::string>::iterator c_it;
	std::set<std::string> rc = s->children[rb];
	while (rc.size()) {
	    std::string rcs = *rc.begin();
	    rc.erase(rcs);
	    if (s->rebuild_commits.find(rcs) == s->rebuild_commits.end()) {
		std::cout << "found reset commit: " << rcs << "\n";
		s->reset_commits.insert(rcs);
	    } else {
		if (s->children.find(rcs) != s->children.end()) {
		    rc.insert(s->children[rcs].begin(), s->children[rcs].end());
		}
	    }
	}
    }

    // Now that we know what the reset commits are, generate the trees that will
    // achieve this.
    std::set<std::string>::iterator s_it;
    for (s_it = s->reset_commits.begin(); s_it != s->reset_commits.end(); s_it++) {
	std::string sha1 = *s_it;
	std::string git_ls_tree_cmd = std::string("cd ") + repo_path + std::string(" && git ls-tree --full-tree -r ") + sha1 + std::string(" > ../tree.txt && cd ..");
	if (std::system(git_ls_tree_cmd.c_str())) {
	    std::cout << "git_ls_tree_cmd \"" << git_ls_tree_cmd << "\" failed\n";
	    exit(-1);
	}
	process_ls_tree(sha1);
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
