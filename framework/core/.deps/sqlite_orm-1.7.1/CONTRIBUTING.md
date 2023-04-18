# How to Contribute #

Thank you for your interest in contributing to the sqlite_orm project!

## GitHub pull requests ##

This is the preferred method of submitting changes.  When you submit a pull request through github,
it activates the continuous integration (CI) build systems at Appveyor and Travis to build your changes
on a variety of Linux, Windows and MacOS configurations and run all the test suites.  Follow these requirements 
for a successful pull request:

 1. All significant changes require a [github issue](https://github.com/fnc12/sqlite_orm/issues).  Trivial changes such as fixing a typo or a compiler warning do not.

 1. The pull request title must begin with the github issue identifier if it has an associated issue, for example:

        #9999 : an example pull request title
        
 1. Commit messages must be understandable in future by different developers and must be written in english language only:
     
Instructions:

 1. Create a fork in your GitHub account of http://github.com/fnc12/sqlite_orm
 1. Clone the fork to your development system.
 1. Create a branch for your changes (best practice is following git flow pattern with issue number as branch name, e.g. feature/9999-some-feature or bugfix/9999-some-bug).
 1. Modify the source to include the improvement/bugfix, and:

    * Remember to provide *tests* for all submitted changes!
    * Use test-driven development (TDD): add a test that will isolate the bug *before* applying the change that fixes it.
    * Verify that you follow current code style on sqlite_orm.
    * [*optional*] Verify that your change works on other platforms by adding a GitHub service hook to [Travis CI](http://docs.travis-ci.com/user/getting-started/#Step-one%3A-Sign-in) and [AppVeyor](http://www.appveyor.com/docs).  You can use this technique to run the sqlite_orm CI jobs in your account to check your changes before they are made public.  Every GitHub pull request into sqlite_orm will run the full CI build and test suite on your changes.

 1. Commit and push changes to your branch (please use issue name and description as commit title, e.g. "make it perfect. (fixes #9999)").
 1. Use GitHub to create a pull request going from your branch to sqlite_orm:dev.  Ensure that the github issue number is at the beginning of the title of your pull request.
 1. Wait for other contributors or committers to review your new addition, and for a CI build to complete.
 1. Wait for a owner or collaborators to commit your patch.

## If you want to build the project locally ##

See our detailed instructions on the [CMake README](https://github.com/fnc12/sqlite_orm#usage).

## If you want to review open issues... ##

 1. Review the [GitHub Pull Request Backlog](https://github.com/fnc12/sqlite_orm/pulls).  Code reviews are opened to all.

## If you discovered a defect... ##

 1. Check to see if the issue is already in the [github issues](https://github.com/fnc12/sqlite_orm/issues).
 1. If not please create an issue describing the change you're proposing in the github issues page.
 1. Contribute your code changes using the GitHub pull request method:

## GitHub recipes for Pull Requests ##

Sometimes commmitters may ask you to take actions in your pull requests.  Here are some recipes that will help you accomplish those requests.  These examples assume you are working on github issue 9999.  You should also be familiar with the [upstream](https://help.github.com/articles/syncing-a-fork/) repository concept.

### Squash your changes ###

If you have commits with adding code which is removed in a different commit within the same PR then please squash all commits to remove unnecessary add commits.

1. Use the command ``git log`` to identify how many commits you made since you began.
2. Use the command ``git rebase -i HEAD~N`` where N is the number of commits.
3. Leave "pull" in the first line.
4. Change all other lines from "pull" to "fixup".
5. All your changes are now in a single commit.

If you already have a pull request outstanding, you will need to do a "force push" to overwrite it since you changed your commit history:

    git push -u origin feature/9999-make-perfect --force

A more detailed walkthrough of a squash can be found at [Git Ready](http://gitready.com/advanced/2009/02/10/squashing-commits-with-rebase.html).

### Rebase your pull request ###

If your pull request has a conflict with dev, it needs to be rebased:

    git checkout feature/9999-make-perfect
    git rebase upstream dev
      (resolve any conflicts, make sure it builds)
    git push -u origin feature/9999-make-perfect --force

### Fix a bad merge ###

If your pull request contains commits that are not yours, then you should use the following technique to fix the bad merge in your branch:

    git checkout dev
    git pull upstream dev
    git checkout -b feature/9999-make-perfect-take-2
    git cherry-pick ...
        (pick only your commits from your original pull request in ascending chronological order)
    squash your changes to a single commit if there is more than one (see above)
    git push -u origin feature/9999-make-perfect-take-2:feature/9999-make-perfect

This procedure will apply only your commits in order to the current dev, then you will squash them to a single commit, and then you force push your local feature/9999-make-perfect-take-2 into remote feature/9999-make-perfect which represents your pull request, replacing all the commits with the new one.

 
