SCHEMA = ../schema.xsd
GOOD = $(wildcard good/*.xml)
BAD = $(wildcard bad/*.xml)
GOOD_OUT = $(patsubst %.xml,%.out,$(GOOD))
BAD_OUT = $(patsubst %.xml,%.out,$(BAD))
.PHONY: bad check example good

check: bad good example

example: $(SCHEMA) ../example.xml
	xmllint --noout --schema "$(SCHEMA)" ../example.xml

good: $(GOOD_OUT)

bad: $(BAD_OUT)

good/%.out: $(SCHEMA) good/%.xml
	xmllint --noout --schema "$(SCHEMA)" good/$*.xml

.ONESHELL:
bad/%.out: $(SCHEMA) bad/%.xml
	@xmllint --noout --schema "$(SCHEMA)" bad/$*.xml 2>/dev/null
	RETVAL=$$?
	if [ "$$RETVAL" == 3 ]; then
		echo "bad/$*.xml fails to validate as expected"
		exit 0
	fi
	if [ "$$RETVAL" == 0 ]; then
		echo "bad/$*.xml unexpectedly validated"
	else
		echo "bad/$*.xml failed to be processed"
	fi
	exit 1
