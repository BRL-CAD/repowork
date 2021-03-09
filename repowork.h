/*                      R E P O W O R K . H
 * BRL-CAD
 *
 * Published in 2020 by the United States Government.
 * This work is in the public domain.
 *
 */
/** @file repowork.h
 *
 * Brief description
 *
 */

#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <stdlib.h>

#ifndef REPOWORK_H
#define REPOWORK_H

/* Parser of Git fast-export information, based on the format described by:
 * https://git-scm.com/docs/git-fast-import */


/* Convenience macro for comparing a substring to the first part of a longer
 * string - used primarily to find commands */
#define ficmp(_s1, _s2) _s1.compare(0, _s2.size(), _s2) && _s1.size() >= _s2.size()

class git_commitish {
    public:
	long index = -1;  // all commits must have an index into the master commit vector
	long mark  = -1;  // globally unique numerical identifier
	std::string sha1 = std::string(); // Original sha1 or part thereof, if available.  If the commit is modified this string should be invalidated.
	std::string ref = std::string(); //  branch reference or other Git reference
};


/* Types of git file actions */
enum git_action_t { filemodify, filedelete, filecopy, filerename, filedeleteall, notemodify };
class git_op {
    public:
	git_action_t type;
	std::string mode;
	git_commitish dataref;
	std::string path;
	std::string dest_path;
};

struct git_fi_data;

class git_commit_data {
    public:
	struct git_fi_data *s;

	// Basic commit data
	git_commitish id;
	std::string commit_msg;
	std::string branch;
	std::vector<git_op> fileops;  // ordered set of file operations

	// Authorship
	std::string author;
	std::string author_timestamp;
	std::string committer;
	std::string committer_timestamp;

	// Relationships with other commits
	git_commitish from;
	std::vector<git_commitish> merges;

	// Notes present a problem - they're stored using a separate
	// structure commits, and data but are associated with commits
	// using the SHA1 hash *only* - they aren't tied in to the mark
	// ids in fast export.  Therefore, we need a --show-original-ids
	// fast export to be able to make the association during parsing.
	//
	// A note commit will have one file op, whose path will be the
	// SHA1 of the commit it notates and its dataref will be the
	// note contents.
	int notes_commit = 0;

	// It's not maximally efficient space wise, but rather than trying
	// to keep track of where the note content associated with this commit
	// is whenever we need it, just provide a convenient place to stash it.
	// Then we only have to untangle things once.
	std::string notes_string;

	// Resets are order dependent - treat them as pseudo-commits for
	// storage purpose, but they are written differently
	int reset_commit = 0;

	// Special purpose entries for assigning an additional line
	// to the existing notes-based info to id SVN usernames
	std::string svn_id;
	std::string svn_author;
};

class git_tag_data {
    public:
	struct git_fi_data *s;
	std::string tag;
	git_commitish id;
	git_commitish from;
	std::string tag_msg;
	std::string tagger;
	std::string tagger_timestamp;
};

class git_blob_data {
    public:
	struct git_fi_data *s;
	size_t offset;
	size_t length;
	git_commitish id;

	/* If a blob is needed that is not in the original fi file,
	 * we need a local buffer to hold the data */
	char *cbuffer = NULL;
};

class git_fi_data {

    public:
	bool write_notes = true;
	bool wrap_commit_lines = false;
	bool trim_whitespace = false;

	std::vector<git_blob_data> blobs;
	std::vector<git_tag_data> tags;
	std::vector<git_commit_data> commits;

	// SHA1s are static in this environment, since it is too
	// difficult to calculate the SHA1 after changing commit
	// data - therefore, we need to be able to map from old
	// SHA1 references to mark values.
	std::map<std::string, long> sha1_to_mark;

	// Marks are unique, and context will make it clear which vector
	// is being referenced.
	std::map<long, long> mark_to_index;
	std::map<long, long> mark_old_to_new;

	// As long as the proposed mark m is above the range of assigned marks,
	// go with it.  Otherwise, generate a new mark
	long next_mark(int m) {
	    if (m == -1) {
	       	mark++;
	    } else {
		mark = m;
	    }
	    if (m != -1) {
		mark_old_to_new[m] = mark;
	    }
	    return mark;
	};

    private:
	long mark = -1;
};

extern int git_parse_commitish(git_commitish &gc, git_fi_data *s, std::string line);
extern int parse_blob(git_fi_data *fi_data, std::ifstream &infile);
extern int parse_commit(git_fi_data *fi_data, std::ifstream &infile);
extern int parse_reset(git_fi_data *fi_data, std::ifstream &infile);
extern int parse_tag(git_fi_data *fi_data, std::ifstream &infile);

/* Misc commands */
extern int parse_alias(git_fi_data *fi_data, std::ifstream &infile);
extern int parse_cat_blob(git_fi_data *fi_data, std::ifstream &infile);
extern int parse_checkpoint(git_fi_data *fi_data, std::ifstream &infile);
extern int parse_done(git_fi_data *fi_data, std::ifstream &infile);
extern int parse_feature(git_fi_data *fi_data, std::ifstream &infile);
extern int parse_get_mark(git_fi_data *fi_data, std::ifstream &infile);
extern int parse_progress(git_fi_data *fi_data, std::ifstream &infile);
extern int parse_ls(git_fi_data *fi_data, std::ifstream &infile);
extern int parse_option(git_fi_data *fi_data, std::ifstream &infile);


/* Output */
extern int write_blob(std::ofstream &outfile, git_blob_data *b, std::ifstream &infile);
extern int write_commit(std::ofstream &outfile, git_commit_data *c, std::ifstream &infile);
extern int write_tag(std::ofstream &outfile, git_tag_data *t, std::ifstream &infile);

#endif /* REPOWORK_H */

/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */