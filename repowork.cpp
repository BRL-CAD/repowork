/*                    R E P O W O R K . C P P
 * BRL-CAD
 *
 * Published in 2020 by the United States Government.
 * This work is in the public domain.
 *
 */
/** @file repowork.cpp
 *
 * Main processing loop
 *
 */

#include <filesystem>
#include <iostream>
#include <sstream>
#include <locale>

#include "cxxopts.hpp"
#include "repowork.h"


typedef int (*gitcmd_t)(git_fi_data *, std::ifstream &);

gitcmd_t
gitit_find_cmd(std::string &line, std::map<std::string, gitcmd_t> &cmdmap)
{
    gitcmd_t gc = NULL;
    std::map<std::string, gitcmd_t>::iterator c_it;
    for (c_it = cmdmap.begin(); c_it != cmdmap.end(); c_it++) {
	if (!ficmp(line, c_it->first)) {
	    gc = c_it->second;
	    break;
	}
    }
    return gc;
}

int
parse_fi_file(git_fi_data *fi_data, std::ifstream &infile)
{
    std::map<std::string, gitcmd_t> cmdmap;
    cmdmap[std::string("alias")] = parse_alias;
    cmdmap[std::string("blob")] = parse_blob;
    cmdmap[std::string("cat-blob")] = parse_cat_blob;
    cmdmap[std::string("checkpoint")] = parse_checkpoint;
    cmdmap[std::string("commit ")] = parse_commit;
    cmdmap[std::string("done")] = parse_done;
    cmdmap[std::string("feature")] = parse_feature;
    cmdmap[std::string("get-mark")] = parse_get_mark;
    cmdmap[std::string("ls")] = parse_ls;
    cmdmap[std::string("option")] = parse_option;
    cmdmap[std::string("progress")] = parse_progress;
    cmdmap[std::string("reset")] = parse_reset;
    cmdmap[std::string("tag")] = parse_tag;

    size_t offset = infile.tellg();
    std::string line;
    std::map<std::string, gitcmd_t>::iterator c_it;
    while (std::getline(infile, line)) {
	// Skip empty lines
	if (!line.length()) {
	    offset = infile.tellg();
	    continue;
	}

	gitcmd_t gc = gitit_find_cmd(line, cmdmap);
	if (!gc) {
	    //std::cerr << "Unsupported command!\n";
	    offset = infile.tellg();
	    continue;
	}

	// If we found a command, process it
	//std::cout << "line: " << line << "\n";
	// some commands have data on the command line - reset seek so the
	// callback can process it
	infile.seekg(offset);
	(*gc)(fi_data, infile);
	offset = infile.tellg();
    }


    return 0;
}

int
parse_splice_fi_file(git_fi_data *fi_data, std::ifstream &infile)
{
    std::map<std::string, gitcmd_t> cmdmap;
    cmdmap[std::string("alias")] = parse_alias;
    cmdmap[std::string("blob")] = parse_blob;
    cmdmap[std::string("cat-blob")] = parse_cat_blob;
    cmdmap[std::string("checkpoint")] = parse_checkpoint;
    cmdmap[std::string("commit ")] = parse_splice_commit;
    cmdmap[std::string("done")] = parse_done;
    cmdmap[std::string("feature")] = parse_feature;
    cmdmap[std::string("get-mark")] = parse_get_mark;
    cmdmap[std::string("ls")] = parse_ls;
    cmdmap[std::string("option")] = parse_option;
    cmdmap[std::string("progress")] = parse_progress;
    cmdmap[std::string("reset")] = parse_reset;
    cmdmap[std::string("tag")] = parse_tag;

    size_t offset = infile.tellg();
    std::string line;
    std::map<std::string, gitcmd_t>::iterator c_it;
    while (std::getline(infile, line)) {
	// Skip empty lines
	if (!line.length()) {
	    offset = infile.tellg();
	    continue;
	}

	gitcmd_t gc = gitit_find_cmd(line, cmdmap);
	if (!gc) {
	    //std::cerr << "Unsupported command!\n";
	    offset = infile.tellg();
	    continue;
	}

	// If we found a command, process it
	//std::cout << "line: " << line << "\n";
	// some commands have data on the command line - reset seek so the
	// callback can process it
	infile.seekg(offset);
	(*gc)(fi_data, infile);
	offset = infile.tellg();
    }


    return 0;
}

int
parse_replace_fi_file(git_fi_data *fi_data, std::ifstream &infile)
{
    std::map<std::string, gitcmd_t> cmdmap;
    cmdmap[std::string("alias")] = parse_alias;
    cmdmap[std::string("blob")] = parse_blob;
    cmdmap[std::string("cat-blob")] = parse_cat_blob;
    cmdmap[std::string("checkpoint")] = parse_checkpoint;
    cmdmap[std::string("commit ")] = parse_replace_commit;
    cmdmap[std::string("done")] = parse_done;
    cmdmap[std::string("feature")] = parse_feature;
    cmdmap[std::string("get-mark")] = parse_get_mark;
    cmdmap[std::string("ls")] = parse_ls;
    cmdmap[std::string("option")] = parse_option;
    cmdmap[std::string("progress")] = parse_progress;
    cmdmap[std::string("reset")] = parse_reset;
    cmdmap[std::string("tag")] = parse_tag;

    size_t offset = infile.tellg();
    std::string line;
    std::map<std::string, gitcmd_t>::iterator c_it;
    while (std::getline(infile, line)) {
	// Skip empty lines
	if (!line.length()) {
	    offset = infile.tellg();
	    continue;
	}

	gitcmd_t gc = gitit_find_cmd(line, cmdmap);
	if (!gc) {
	    //std::cerr << "Unsupported command!\n";
	    offset = infile.tellg();
	    continue;
	}

	// If we found a command, process it
	//std::cout << "line: " << line << "\n";
	// some commands have data on the command line - reset seek so the
	// callback can process it
	infile.seekg(offset);
	(*gc)(fi_data, infile);
	offset = infile.tellg();
    }

    return 0;
}

int
parse_add_fi_file(git_fi_data *fi_data, std::ifstream &infile)
{
    std::map<std::string, gitcmd_t> cmdmap;
    cmdmap[std::string("alias")] = parse_alias;
    cmdmap[std::string("blob")] = parse_blob;
    cmdmap[std::string("cat-blob")] = parse_cat_blob;
    cmdmap[std::string("checkpoint")] = parse_checkpoint;
    cmdmap[std::string("commit ")] = parse_add_commit;
    cmdmap[std::string("done")] = parse_done;
    cmdmap[std::string("feature")] = parse_feature;
    cmdmap[std::string("get-mark")] = parse_get_mark;
    cmdmap[std::string("ls")] = parse_ls;
    cmdmap[std::string("option")] = parse_option;
    cmdmap[std::string("progress")] = parse_progress;
    cmdmap[std::string("reset")] = parse_reset;
    cmdmap[std::string("tag")] = parse_tag;

    size_t offset = infile.tellg();
    std::string line;
    std::map<std::string, gitcmd_t>::iterator c_it;
    while (std::getline(infile, line)) {
	// Skip empty lines
	if (!line.length()) {
	    offset = infile.tellg();
	    continue;
	}

	gitcmd_t gc = gitit_find_cmd(line, cmdmap);
	if (!gc) {
	    //std::cerr << "Unsupported command!\n";
	    offset = infile.tellg();
	    continue;
	}

	// If we found a command, process it
	//std::cout << "line: " << line << "\n";
	// some commands have data on the command line - reset seek so the
	// callback can process it
	infile.seekg(offset);
	(*gc)(fi_data, infile);
	offset = infile.tellg();
    }


    return 0;
}



int
main(int argc, char *argv[])
{
    git_fi_data fi_data;
    bool splice_commits = false;
    bool replace_commits = false;
    bool add_commits = false;
    bool no_blobs = false;
    bool no_commits = false;
    bool no_tags = false;
    bool collapse_notes = false;
    bool wrap_commit_lines = false;
    bool trim_whitespace = false;
    bool list_empty = false;
    std::string repo_path;
    std::string email_map;
    std::string svn_accounts;
    std::string svn_rev_map;
    std::string svn_branch_map;
    std::string svn_tags;
    std::string remove_commits;
    std::string correct_branches;
    std::string key_account_map;
    std::string key_branch_map;
    std::string key_sha1_map;
    std::string children_file;
    std::string id_file;
    int cwidth = 72;

    // TODO - might be good do have a "validate" option that does the fast import and then
    // checks every commit saved from the old repo in the new one...
    try
    {
	cxxopts::Options options(argv[0], " - process git fast-import files");

	options.add_options()
	    ("email-map", "Specify replacement username+email mappings (one map per line, format is commit-id-1;commit-id-2)", cxxopts::value<std::vector<std::string>>(), "map file")

	    ("t,trim-whitespace", "Trim extra spaces and end-of-line characters from the end of commit messages", cxxopts::value<bool>(trim_whitespace))
	    ("w,wrap-commit-lines", "Wrap long commit lines to 72 cols (won't wrap messages already having multiple non-empty lines)", cxxopts::value<bool>(wrap_commit_lines))
	    ("width", "Column wrapping width (if enabled)", cxxopts::value<int>(), "N")

	    ("r,repo", "Original git repository path (must support running git log)", cxxopts::value<std::vector<std::string>>(), "path")
	    ("n,collapse-notes", "Take any git-notes contents and append them to regular commit messages.  Requires --repo", cxxopts::value<bool>(collapse_notes))

	    ("add-commits", "Look for git fast-import files in an 'add' directory and add to history.  Unlike splice commits, these are not being inserted into existing commit streams.", cxxopts::value<bool>(add_commits))
	    ("remove-commits", "Specify sha1 list of commits to remove from history", cxxopts::value<std::vector<std::string>>(), "list_file")
	    ("splice-commits", "Look for git fast-import files in a 'splices' directory and insert them into the history.", cxxopts::value<bool>(splice_commits))
	    ("replace-commits", "Look for git fast-import files in a 'replace' directory and overwrite.  File of fast import file should be sha1 of target commit to replace.", cxxopts::value<bool>(replace_commits))

	    ("no-blobs", "Write only commits in output .fi file.", cxxopts::value<bool>(no_blobs))
	    ("no-commits", "Write only commits in output .fi file.", cxxopts::value<bool>(no_commits))
	    ("no-tags", "Write only commits in output .fi file.", cxxopts::value<bool>(no_tags))

	    ("list-empty", "Print out information about empty commits.", cxxopts::value<bool>(list_empty))


	    ("svn-accounts", "Specify svn rev -> committer map (one mapping per line, format is commit-rev name)", cxxopts::value<std::vector<std::string>>(), "map file")
	    ("svn-revs", "Specify git sha1 -> svn rev map (one mapping per line, format is sha1;[commit-rev])", cxxopts::value<std::vector<std::string>>(), "map file")
	    ("set-branches", "Specify [git sha1|rev] -> svn branch (one mapping per line, format is key:[branch;branch], commits without an entry are cleared)", cxxopts::value<std::vector<std::string>>(), "map file")
	    ("correct-branches", "Specify rev -> branch sets (one mapping per line, format is key;[branch;branch], entries set here will override values in set-branches assignments, commits without an entry are NOT cleared.)", cxxopts::value<std::vector<std::string>>(), "map")
	    ("svn-tags", "Specify git sha1 list that was committed to tags, not branches", cxxopts::value<std::vector<std::string>>(), "sha1 list")
	    ("key-sha1-map", "sha1 -> msg&time map (needs original-oid tags)", cxxopts::value<std::vector<std::string>>(), "file")
	    ("key-account-map", "msg&time -> author map (needs sha1->key map)", cxxopts::value<std::vector<std::string>>(), "file")
	    ("key-branch-map", "msg&time -> branch map (needs sha1->key map)", cxxopts::value<std::vector<std::string>>(), "file")

	    ("rebuild-ids", "Specify commits (revision number or SHA1) to rebuild.  Requires git-repo be set as well.  Needs --show-original-ids information in fast import file", cxxopts::value<std::vector<std::string>>(), "file")
	    ("rebuild-ids-children", "File with output of \"git rev-list --children --all\" - needed for processing rebuild-ids", cxxopts::value<std::vector<std::string>>(), "file")

	    ("h,help", "Print help")
	    ;

	auto result = options.parse(argc, argv);

	if (result.count("help"))
	{
	    options.custom_help(std::string("[OPTION...] <input_file> <output_file>"));
	    std::cout << options.help({""}) << std::endl;
	    return 0;
	}

	if (result.count("r"))
	{
	    auto& ff = result["r"].as<std::vector<std::string>>();
	    repo_path = ff[0];
	}

	if (result.count("email-map"))
	{
	    auto& ff = result["email-map"].as<std::vector<std::string>>();
	    email_map = ff[0];
	}

	if (result.count("svn-accounts"))
	{
	    auto& ff = result["svn-accounts"].as<std::vector<std::string>>();
	    svn_accounts = ff[0];
	}

	if (result.count("rebuild-ids"))
	{
	    auto& ff = result["rebuild-ids"].as<std::vector<std::string>>();
	    id_file = ff[0];
	}

	if (result.count("rebuild-ids-children"))
	{
	    auto& ff = result["rebuild-ids-children"].as<std::vector<std::string>>();
	    children_file = ff[0];
	}

	if (result.count("key-sha1-map"))
	{
	    auto& ff = result["key-sha1-map"].as<std::vector<std::string>>();
	    key_sha1_map = ff[0];
	}

	if (result.count("key-account-map"))
	{
	    auto& ff = result["key-account-map"].as<std::vector<std::string>>();
	    key_account_map = ff[0];
	}

	if (result.count("key-branch-map"))
	{
	    auto& ff = result["key-branch-map"].as<std::vector<std::string>>();
	    key_branch_map = ff[0];
	}

	if (result.count("svn-revs"))
	{
	    auto& ff = result["svn-revs"].as<std::vector<std::string>>();
	    svn_rev_map = ff[0];
	}

	if (result.count("set-branches"))
	{
	    auto& ff = result["set-branches"].as<std::vector<std::string>>();
	    svn_branch_map = ff[0];
	}

	if (result.count("correct-branches"))
	{
	    auto& ff = result["correct-branches"].as<std::vector<std::string>>();
	    correct_branches = ff[0];
	}

	if (result.count("remove-commits"))
	{
	    auto& ff = result["remove-commits"].as<std::vector<std::string>>();
	    remove_commits = ff[0];
	}

	if (result.count("svn-tags"))
	{
	    auto& ff = result["svn-tags"].as<std::vector<std::string>>();
	    svn_tags = ff[0];
	}

	if (result.count("width"))
	{
	    cwidth = result["width"].as<int>();
	}

    }
    catch (const cxxopts::OptionException& e)
    {
	std::cerr << "error parsing options: " << e.what() << std::endl;
	return -1;
    }

    if (collapse_notes && !repo_path.length()) {
	std::cerr << "Cannot collapse notes into commit messages without knowing the path\nto the repository - aborting.  (It is necessary to run git log to\ncapture the message information, and for that we need the original\nrepository in addition to the fast-import file.)\n\nTo specify a repo folder, use the -r option.  Currently the folder must be in the working directory.\n";
	return -1;
    }

    if (id_file.length() && !repo_path.length()) {
	std::cerr << "Need Git repository path for CVS id list processing!\n";
	return -1;
    }

    if (argc != 3) {
	std::cout << "repowork [OPTION...] <input_file> <output_file>\n";
	return -1;
    }
    std::ifstream infile(argv[1], std::ifstream::binary);
    if (!infile.good()) {
	return -1;
    }

    int ret = parse_fi_file(&fi_data, infile);

    // The subsequent steps, if invoked, may need svn_id set.
    for (size_t i = 0; i < fi_data.commits.size(); i++) {
	parse_cvs_svn_info(&fi_data.commits[i], fi_data.commits[i].commit_msg);
    }

    if ((replace_commits || splice_commits || add_commits) && !fi_data.have_sha1s) {
	std::cerr << "Fatal - sha1 SVN rev updating requested, but don't have original sha1 ids - redo fast-export with the --show-original-ids option.\n";
	exit(1);
    }


    if (collapse_notes) {
	// Let the output routines know not to write notes commits.
	// (blobs will have to be taken care of later by git gc).
	fi_data.write_notes = false;

	// Reset the input stream
	infile.clear();
	infile.seekg(0, std::ios::beg);

	// Handle the notes
	git_unpack_notes(&fi_data, repo_path);
	git_parse_notes(&fi_data);
    }

    if (key_sha1_map.length()) {
	read_key_sha1_map(&fi_data, key_sha1_map);
    }

    if (key_account_map.length()) {
	if (!key_sha1_map.length()) {
	    std::cerr << "CVS author map specified without key map\n";
	    return -1;
	}
	read_key_cvsauthor_map(&fi_data, key_account_map);
    }

    if (key_branch_map.length()) {
	if (!key_sha1_map.length()) {
	    std::cerr << "CVS branch map specified without key map\n";
	    return -1;
	}
	read_key_cvsbranch_map(&fi_data, key_branch_map);
    }

    if (email_map.length()) {
	// Reset the input stream
	infile.clear();
	infile.seekg(0, std::ios::beg);

	// Handle the notes
	git_map_emails(&fi_data, email_map);
    }

    if (svn_accounts.length()) {
	// Handle the svn committers
	git_map_svn_committers(&fi_data, svn_accounts);
    }

    if (list_empty) {
	for (size_t i = 0; i < fi_data.commits.size(); i++) {
	    if (fi_data.commits[i].commit_msg.length() && !fi_data.commits[i].fileops.size()) {
		if (fi_data.commits[i].id.sha1.length()) {
		    std::cout << "Empty commit(" << fi_data.commits[i].id.sha1 << "): " << fi_data.commits[i].commit_msg << "\n";
		} else {
		    std::cout << "Empty commit: " << fi_data.commits[i].commit_msg << "\n";
		}
	    }
	}
    }

    if (id_file.length()) {
	// Handle rebuild info
	git_id_rebuild_commits(&fi_data, id_file, repo_path, children_file);
    }

    if (remove_commits.length()) {
	git_remove_commits(&fi_data, remove_commits);
    }

    ////////////////////////////////////////////////////
    // Various note correction routines
    if (svn_rev_map.length()) {
	git_update_svn_revs(&fi_data, svn_rev_map);
    }

    if (svn_branch_map.length()) {
	git_assign_branch_labels(&fi_data, svn_branch_map, 0);
    }
    if (correct_branches.length()) {
	git_assign_branch_labels(&fi_data, correct_branches, 1);
    }
    if (svn_tags.length()) {
	git_set_tag_labels(&fi_data, svn_tags);
    }

    fi_data.wrap_width = cwidth;
    fi_data.wrap_commit_lines = wrap_commit_lines;
    fi_data.trim_whitespace = trim_whitespace;

    infile.close();


    // If we have any replace commits, parse and overwrite.
    if (replace_commits) {

	std::filesystem::path ip = std::string(argv[1]);
	std::filesystem::path aip = std::filesystem::absolute(ip);
	std::filesystem::path pip = aip.parent_path();
	pip /= "replace";
	if (!std::filesystem::exists(pip)) {
	    std::cerr << "Warning - splices enabled but " << pip << " is not present on the filesystem.\n";
	} else {
	    for (const auto& de : std::filesystem::recursive_directory_iterator(pip)) {
		std::cout << "Processing " << de.path().string() << "\n";
		std::ifstream sfile(de.path(), std::ifstream::binary);
		fi_data.replace_sha1 = de.path().filename().string();
		int ret = parse_replace_fi_file(&fi_data, sfile);
		sfile.close();
	    }
	}
    }

    // If we have any additional commits, parse and insert them.
    if (add_commits) {

	std::filesystem::path ip = std::string(argv[1]);
	std::filesystem::path aip = std::filesystem::absolute(ip);
	std::filesystem::path pip = aip.parent_path();
	pip /= "add";
	if (!std::filesystem::exists(pip)) {
	    std::cerr << "Warning - adds enabled but " << pip << " is not present on the filesystem.\n";
	} else {
	    for (const auto& de : std::filesystem::recursive_directory_iterator(pip)) {
		std::cout << "Processing " << de.path().string() << "\n";
		std::ifstream sfile(de.path(), std::ifstream::binary);
		int ret = parse_add_fi_file(&fi_data, sfile);
		sfile.close();
	    }
	}
    }

    // If we have any splice commits, parse and insert them.  (Note - this comes last, for
    // bookkeeping reasons.)
    if (splice_commits) {

	std::filesystem::path ip = std::string(argv[1]);
	std::filesystem::path aip = std::filesystem::absolute(ip);
	std::filesystem::path pip = aip.parent_path();
	pip /= "splices";
	if (!std::filesystem::exists(pip)) {
	    std::cerr << "Warning - splices enabled but " << pip << " is not present on the filesystem.\n";
	} else {
	    for (const auto& de : std::filesystem::recursive_directory_iterator(pip)) {
		std::cout << "Processing " << de.path().string() << "\n";
		std::ifstream sfile(de.path(), std::ifstream::binary);
		int ret = parse_splice_fi_file(&fi_data, sfile);
		sfile.close();
	    }
	}
    }

    std::ifstream ifile(argv[1], std::ifstream::binary);
    std::ofstream ofile(argv[2], std::ios::out | std::ios::binary);
    if (!no_blobs) {
	ofile << "progress Writing blobs...\n";
	for (size_t i = 0; i < fi_data.blobs.size(); i++) {
	    write_blob(ofile, &fi_data.blobs[i], ifile);
	    if ( !(i % 1000) ) {
		ofile << "progress blob " << i << " of " << fi_data.blobs.size() << "\n";
	    }
	}
    }
    if (!no_commits) {
	ofile << "progress Writing commits...\n";
	for (size_t i = 0; i < fi_data.commits.size(); i++) {
	    write_commit(ofile, &fi_data.commits[i], &fi_data, ifile);
	    if ( !(i % 1000) ) {
		ofile << "progress commit " << i << " of " << fi_data.commits.size() << "\n";
	    }
	}
    }
    if (!no_tags) {
	ofile << "progress Writing tags...\n";
	for (size_t i = 0; i < fi_data.tags.size(); i++) {
	    write_tag(ofile, &fi_data.tags[i], ifile);
	}
    }
    ofile << "progress Done.\n";

    ifile.close();
    ofile.close();

    std::cout << "Git fast-import file is generated:  " << argv[2] << "\n\n";
    std::cout << "Note that when imported, compression and packing will be suboptimal by default.\n";
    std::cout << "Some possible steps to take:\n";
    std::cout << "  mkdir git_repo && cd git_repo && git init\n";
    std::cout << "  cat ../" << argv[2] << " | git fast-import\n";
    std::cout << "  git gc --aggressive\n";
    std::cout << "  git reflog expire --expire-unreachable=now --all\n";
    std::cout << "  git gc --prune=now\n";

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
