/*                    N O T E S . C P P
 * BRL-CAD
 *
 * Published in 2020 by the United States Government.
 * This work is in the public domain.
 *
 */
/** @file notes.cpp
 *
 * Utility functions for handling git notes
 *
 */

#include <string>
#include <regex>

#include "repowork.h"

int
git_unpack_notes(git_fi_data *s, std::string &repo_path)
{
    // Iterate over the commits looking for note commits.  If we find one,
    // find its associated blob with data, read it, find the associated
    // commit, and stash it in a string in that container.
    for (size_t i = 0; i < s->commits.size(); i++) {
	if (s->commits[i].notes_commit) {
	    continue;
	}

	if (!s->have_sha1s) {
	    std::cerr << "Fatal - notes unpacking requested, but don't have original sha1 ids - redo fast-export with the --show-original-ids option.\n";
	    exit(1);
	}

	if (!s->commits[i].id.sha1.length()) {
	    std::cerr << "Warning - commit " << s->commits[i].id.mark << " has no sha1 info, skipping notes lookup\n";
	    continue;
	}

	// This is cheap and clunky, but I've not yet found a document
	// describing how to reliably unpack git notes...
	std::string git_notes_cmd = std::string("cd ") + repo_path + std::string(" && git log -1 ") + s->commits[i].id.sha1 + std::string(" --pretty=format:\"%N\" > ../sha1.txt && cd ..");
        if (std::system(git_notes_cmd.c_str())) {
            std::cout << "git_sha1_cmd failed\n";
	    exit(-1);
        }

	std::ifstream n("sha1.txt");
	if (!n.good()) {
	    std::cout << "sha1.txt read failed\n";
	    exit(-1);
	}
	std::string note((std::istreambuf_iterator<char>(n)), std::istreambuf_iterator<char>());

	// Write the message to the commit's note string storage;
	s->commits[i].notes_string = note;

	n.close();
    }

    return 0;
}

// Parse notes data looking for commit information
int
git_parse_notes(git_fi_data *s)
{
    // Iterate over the commits looking for note commits.  If we find one,
    // find its associated blob with data, read it, find the associated
    // commit, and stash it in a string in that container.
    for (size_t i = 0; i < s->commits.size(); i++) {
	if (!s->commits[i].notes_string.length()) {
	    continue;
	}
	parse_cvs_svn_info(&s->commits[i], s->commits[i].notes_string);
	update_commit_msg(&s->commits[i]);

	// SPECIAL PURPOSE CODE - should go away eventually.
	// We wrote the wrong SVN branch name for older dmtogl branches -
	// names were deliberately collapsed in git conversion, but we
	// should reflect the original SVN history in the metadata.  Undo
	// the mapping for the label based on revision number.
	if (s->commits[i].svn_id.length()) {
	    long revnum = std::stol(s->commits[i].svn_id);
	    if (revnum < 36472) {
		if (s->commits[i].svn_branches.find(std::string("dmtogl")) != s->commits[i].svn_branches.end()) {
		    s->commits[i].svn_branches.erase(std::string("dmtogl"));
		    s->commits[i].svn_branches.insert(std::string("dmtogl-branch"));
		}
		update_commit_msg(&s->commits[i]);
	    }
	}
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
