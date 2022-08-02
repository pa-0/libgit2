#include "clar_libgit2.h"
#include "futils.h"
#include "repository.h"

void test_clone_shallow__initialize(void)
{
    cl_git_pass(git_libgit2_opts(GIT_OPT_ENABLE_SHALLOW, 1));
}

void test_clone_shallow__cleanup(void)
{
	git_libgit2_opts(GIT_OPT_ENABLE_SHALLOW, 0);
	cl_git_sandbox_cleanup();
}

static int remote_single_branch(git_remote **out, git_repository *repo, const char *name, const char *url, void *payload)
{
	GIT_UNUSED(payload);

	cl_git_pass(git_remote_create_with_fetchspec(out, repo, name, url, "+refs/heads/master:refs/remotes/origin/master"));

	return 0;
}

void test_clone_shallow__clone_depth_zero(void)
{
	git_str path = GIT_STR_INIT;
	git_repository *repo;
	git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
	git_array_oid_t roots = GIT_ARRAY_INIT;

	clone_opts.fetch_opts.depth = 0;
	clone_opts.remote_cb = remote_single_branch;

	git_str_joinpath(&path, clar_sandbox_path(), "shallowclone_0");

	cl_git_pass(git_clone(&repo, "https://github.com/libgit2/TestGitRepository", git_str_cstr(&path), &clone_opts));

	/* cloning with depth 0 results in a full clone. */
	cl_assert_equal_b(false, git_repository_is_shallow(repo));

	/* full clones do not have shallow roots. */
	cl_git_pass(git_repository__shallow_roots(&roots, repo));
	cl_assert_equal_i(0, roots.size);

	git_array_clear(roots);
	git_str_dispose(&path);
	git_repository_free(repo);
}

void test_clone_shallow__clone_depth_one(void)
{
	git_str path = GIT_STR_INIT;
	git_repository *repo;
	git_revwalk *walk;
	git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
	git_oid oid;
	git_array_oid_t roots = GIT_ARRAY_INIT;
	size_t num_commits = 0;
	int error = 0;

	clone_opts.fetch_opts.depth = 1;
	clone_opts.remote_cb = remote_single_branch;

	git_str_joinpath(&path, clar_sandbox_path(), "shallowclone_1");

	cl_git_pass(git_clone(&repo, "https://github.com/libgit2/TestGitRepository", git_str_cstr(&path), &clone_opts));

	cl_assert_equal_b(true, git_repository_is_shallow(repo));

	cl_git_pass(git_repository__shallow_roots(&roots, repo));
	cl_assert_equal_i(1, roots.size);
	cl_assert_equal_s("49322bb17d3acc9146f98c97d078513228bbf3c0", git_oid_tostr_s(&roots.ptr[0]));

	git_revwalk_new(&walk, repo);

	git_revwalk_push_head(walk);

	while ((error = git_revwalk_next(&oid, walk)) == GIT_OK) {
		num_commits++;
	}

	cl_assert_equal_i(num_commits, 1);
	cl_assert_equal_i(error, GIT_ITEROVER);

	git_array_clear(roots);
	git_str_dispose(&path);
	git_revwalk_free(walk);
	git_repository_free(repo);
}

void test_clone_shallow__clone_depth_five(void)
{
	git_str path = GIT_STR_INIT;
	git_repository *repo;
	git_revwalk *walk;
	git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
	git_oid oid;
	git_array_oid_t roots = GIT_ARRAY_INIT;
	size_t num_commits = 0;
	int error = 0;

	clone_opts.fetch_opts.depth = 5;
	clone_opts.remote_cb = remote_single_branch;

	git_str_joinpath(&path, clar_sandbox_path(), "shallowclone_5");

	cl_git_pass(git_clone(&repo, "https://github.com/libgit2/TestGitRepository", git_str_cstr(&path), &clone_opts));

	cl_assert_equal_b(true, git_repository_is_shallow(repo));

	cl_git_pass(git_repository__shallow_roots(&roots, repo));
	cl_assert_equal_i(3, roots.size);
	cl_assert_equal_s("c070ad8c08840c8116da865b2d65593a6bb9cd2a", git_oid_tostr_s(&roots.ptr[0]));
	cl_assert_equal_s("0966a434eb1a025db6b71485ab63a3bfbea520b6", git_oid_tostr_s(&roots.ptr[1]));
	cl_assert_equal_s("83834a7afdaa1a1260568567f6ad90020389f664", git_oid_tostr_s(&roots.ptr[2]));

	git_revwalk_new(&walk, repo);

	git_revwalk_push_head(walk);

	while ((error = git_revwalk_next(&oid, walk)) == GIT_OK) {
		num_commits++;
	}

	cl_assert_equal_i(num_commits, 13);
	cl_assert_equal_i(error, GIT_ITEROVER);

	git_array_clear(roots);
	git_str_dispose(&path);
	git_revwalk_free(walk);
	git_repository_free(repo);
}
