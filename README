BasKet Note Pads
================
Gleb Baryshev <gleb.baryshev@gmail.com>

Kelvie Wong <kelvie@ieee.org>

////
Run this document through asciidoc to read it nicely-formatted in HTML.
////

Purpose
-------
(From the original README by Sébastien)

This application provides as many baskets as you wish, and you can drag and drop
various objects (text, URLs, images, sounds...)  into its.

Objects can be edited, copied, dragged... So, you can arrange them as you want !

It's a DropDrawers clone (http://www.sigsoftware.com/dropdrawers/index.html) for
KDE 4.

Project Status
--------------
In the previous years, porting from KDE 3 to KDE 4 was generally finished.
However, some features remained not ported or became broken. Currently bug
fixing is under way.

Developers
-----------
As you may or may not have noticed, there isn't a user's section currently.
If you are reading this, chances are, you are a developer (if I'm wrong email me
;), so most of the developers documentation will go here until we can finalize a
user README after we're done porting.

Git
~~~
At present BasKet project uses Git.  The main reason for this is that right now, we
don't have much contact with the main developers, and a bunch of us do not have
SVN access.  There are other people who wish to contribute and are in a similar
situation, and so the easiest way to coordinate these efforts is through a
distributed source management system -- Git fills this void nicely, is under
very active development, and is quickly gaining popularity.

Using Git
~~~~~~~~~
I'm sure not everyone is familiar with Git.  There's plenty of resources about
what commands do what, so here's a just a quick rundown of some Git conventions.

Don't work on the master branch
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Your published `master` branch is the branch other people will be pulling, so
only merge what you are confident with sharing.  I generally keep a branch
called `work` in my repository for the main work, and `next` as a branch that's
waiting to be merged into master.

Basically, keep your `master` clean.  Don't try to rewrite history on master
(with git rebase) after it has been published -- if anyone else has pulled from
you, they are going to be in heaps of trouble the next time they try to.
Instead, just add things like revert commits (via `git revert`), or other
amending commits on top of your current `master`.

Commit messages
^^^^^^^^^^^^^^^
Feel free to write whatever you want in your own branches, but try to have the
messages that you merge into master follow a specific convention

It is important to follow this convention because many git tools (such as Gitorious
and the gitweb web interfaces, as well as `git format-patch`, and even `git
log`!)  rely on this convention when parsing commit messages.

In short, the first line of your commit message should be a _very_ short (no
longer than 80 characters) summary of your commit.  When creating a patch, it
becomes the subject of the email (in Git, patches are emails).

Separate it then with two lines (this part's important!), and a detailed
description of the commit.  Do not be afraid to write really detailed commit
message (in fact, I'd encourage it! People who want the short version can just
read the first line).

For example (this is commit `688ab72c`):
----
    Removed all of the whitespace changes from kde4port-svn. (<-- subject line)

    The process to get rid of the whitespace changes was this: (full descrip.)
    1. First ask git to create a diff without whitespace changes
    2. Run a script to go in and reject hunks with non-trivial whitespace
       changes (such as multiline whitespace changes and adding whitespace
       where there were none before).
    3. Apply the new patch.
    4. Apply the changes between this new HEAD and kde4port-svn onto kde4port-svn

    ...
----

Also, good commit messages means we don't have to write changelogs (try it! type
`git shortlog`)

////
TODO: More to come..
////

Contributing
~~~~~~~~~~~~
The first step to contribute is to clone a source repository:

----
git clone https://github.com/gl-bars/basket.git
cd ./basket
git checkout remotes/origin/basket-gleb
----

You should add a new branch and make changes there.  When you have a series of
patches ready to be merged with upstream, first make sure your master is up to
date:

  git pull https://github.com/gl-bars/basket.git basket-gleb

Then push your changes to your repository on GitHub and then submit a merge
request against the main repository.

Contact
-------
If you have any questions, or would like to contribute (always welcome!) please
send me an email to the  development mailing list at
basket-devel@lists.sourceforge.net.

Developers are usually idle on #basket-devel @ freenode on IRC, and it's quite
likely you'll catch one of us there Due to timezone differences, however, it's
generally better to email the list.

The BasKet web site (again, unmaintained right now) is at:
http://basket.kde.org/


Building/Installation
----------------------
To build and install BasKet, follow these steps (this assumes you have the relevant
kde4 development libraries and CMake):

----
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=`kde4-config --prefix` ..
make
# make install
----

Or you can try your luck with the installer script:

  ./installer