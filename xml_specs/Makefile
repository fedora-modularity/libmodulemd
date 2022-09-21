SCHEMA = concise.xsd
GOOD = $(wildcard good/*.xml)
BAD = $(wildcard bad/*.xml)
GOOD_OUT = $(patsubst %.xml,%.out,$(GOOD))
BAD_OUT = $(patsubst %.xml,%.out,$(BAD))
.PHONY: bad check example good

check: bad good example

example: $(SCHEMA) example.xml
	xmllint --noout --schema "$(SCHEMA)" example.xml

good: $(GOOD_OUT)

bad: $(BAD_OUT)

good/%.out: $(SCHEMA) good/%.xml
	xmllint --noout --schema "$(SCHEMA)" good/$*.xml

.ONESHELL:
bad/%.out: $(SCHEMA) bad/%.xml
	@xmllint --noout --schema "$(SCHEMA)" bad/$*.xml 2>/dev/null
	if [ "$$?" == 3 ]; then
		echo "bad/$*.xml fails to validate as expected"
		exit 0;
	else
		echo "bad/$*.xml unexpectedly validated"
		exit 1
	fi
