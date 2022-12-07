This a proposal for an XML format of modular data as found in YUM repositories.


Reasons
=======

Remove YAML from YUM repositories. All other data in the repository is in XML.
This will remove a YAML parser from a list of dependencies of a package
manager. It makes a minimal installation smaller and hopefully safer.

DNF5 package manager is departing from glib library and libmodulemd,
a reference implementation of the YAML format, is built on glib and exposes
glib in its API. The fulfill DNF5's goal, a replacement for libmodulemd is
required as glib cannot be removed without breaking libmodulemd's interface.
An incompatible implementation gives a perfect opportunity for new format.

Remove unnecessary features from the format. YAML format implemented features
which were never adopted (intents, checksums, service levels, disjunctive
dependencies). Remove for-build stanzas irrelevant for YUM repository
(filters, components, buildopts, xmd).

Optimize the format for package managers. YAML format was created as
an input/output format for both packager managers and module builders.
However, with the advent of modulemd-packager-v3 format, the input/output
format symmetry has gone. Next, querying a repository for a list of module
names, list of streams, latest module version meant processing all data as the
YAML format was a linear list of module builds. XML will optimize the format
by placing module builds of the same stream or module name together. Proper
nesting will naturally deduplicate identifiers that were defined and parsed in
YAML again and again.

Well define the format. YAML specification was based on examples and many
details were left open for implementations. As a result, DNF made assumptions
or incomplete or no implementation. XML format will specify an alphabet and
a maximal length for various data types (e.g. stream names). It will prevent
from occurring duplicate or contradicting subdocuments (e.g. multiple default
streams). It will clarify corner cases previously undocumented (e.g. end of
life time pointy). XML language provides well-established tools for a formal
specification (e.g. XML Schema).


Plan
====

Create an XML format in discussion with package manager developers (DNF,
Satellite, libmodulemd) and wide community (e.g. Fedora and Red Hat Enterprise
Linux distribution developers). Have a set of examples, tests, formal
specification.

Create a convertor from YAML modulemd-v2 format to the XML format. That will
help validate the new format on real existing YAML data.

Plug that convertor into a compose process (i.e. enhance createrepo_c to
convert YAML modular data to XML and have them both in the repository).

Create a parser library and merging library. While libmodulemd could be
extended, the required glib remove mandates a new library.

Plug the parser into a package manager (i.e. enhance DNF5 and Satellite to
support and prefer the XML format).

(Once all that works and it demonstrates its benefits over the YAML format, we
can start deprecating the YAML format on the repository side (i.e. change MBS
to produce XML format directly and Bodhi, rpminspect and other tools to handle
XML modules natively).)


Status
======

There are two variant of the specification: "complete" and "reduced". Complete
implements all features of modulemd-v2 YAML format, while reduced is stripped
down of the unwanted features. Now there are both published for your
reference, but the reduced variant is where to go and what to develop. See the
corresponding subdirectories.

Each of the subdirectories contain overview.xml demonstrating a quick
glimpse on format, example.xml demonstrating all the feature on real data,
schema.xsd with a formal specification, tests with positive and negative
tests, and doc documenting decisions made in the design.

As large development in the XML parsing library is expected, the XML specification
will be moved to a dedicated git repository. That's not to disturb libmodulemd
history and release cycle and to clearly separate build-time dependencies.
