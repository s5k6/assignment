Assigning students to tutorials
===============================

I've successfully used this rather ad-hoc tool for many years to
assign students to tutorials.

**Note:** This code is unmaintained, undocumented, and un-everything.
  Use at your own risk.

The students are asked to cast a vote via Email, which consists of an
ordering of the available tutorials with descending precedence.  If
they have a good reason, they may even blacklist a tutorial.

I collect these votes in a plaintext file, and feed it into the tool
together with the tutorial's capacities.  The [provided
example](./votes.dat) uses 6 tutorials identified by upper-case
letters, and 63 students with greek names (which is anonymised
real-life data).  Tutorials `B` and `C` are for 12 students each, the
other tutorials can accommodate 10 students each.

    $ ./assi B=12,C,A=10,D,E,F <votes.dat >assignment.dat

The result is a file containing a pain-minimizing assignment of
students to tutorials.

More information is in the [builtin help](./help.txt).


Build instructions
------------------

    $ make
