#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
	char var;
	char* result;
} Rule;

typedef struct {
	Rule* rules;
	size_t count;
	size_t capacity;
} RuleSet;

typedef struct {
	RuleSet ruleSet;
	char* start;
} Grammar;

char* produceFromRule(Rule rule, char* str) {
	size_t strLen = strlen(str);
	size_t ruleLen = strlen(rule.result);

	size_t occurrences = 0;
	for(size_t i = 0; i < strLen; ++i) {
		if(str[i] == rule.var) occurrences++;
	}

	size_t size = strLen + occurrences * ruleLen + 1;

	char* buffer = calloc(size, sizeof(char));

	size_t bufferIndex = 0;

	for(size_t i = 0; i < strLen; ++i) {
		if(str[i] == rule.var) {
			memcpy(buffer + bufferIndex, rule.result, ruleLen);
			bufferIndex += ruleLen;
		} else {
			buffer[bufferIndex++] = str[i];
		}
	}

	return buffer;
}

typedef struct {
	atomic_bool* isFound;
	Grammar grammar;
	char* currentStr;
	const char* str;
} validationArgs;

void* validateGrammar(void* data) {
	validationArgs args = *(validationArgs*)data;
	if(atomic_load(args.isFound))
		return false;

	const char* currentStr = args.currentStr;
	const char* str = args.str;
	Grammar grammar = args.grammar;

	if(strlen(currentStr) >= (strlen(str) * strlen(currentStr)))
		return false;
	if(strcmp(currentStr, str) == 0)
		return true;

	bool res = false;
	for(int i = 0; i < grammar.ruleSet.count; ++i) {
		Rule rule = grammar.ruleSet.rules[i];

		for(char c = 0; c < strlen(currentStr); ++c)  {
			if(currentStr[c] == rule.var) {
				char* produced = produceFromRule(rule, (char*)currentStr);
				if(strcmp(produced, currentStr) == 0) {
					free(produced);
					return false;
				}

				validationArgs* newArgs = malloc(sizeof(validationArgs));
				newArgs->currentStr = produced;
				newArgs->isFound    = args.isFound;
				newArgs->str		= str;
				validateGrammar(newArgs);
				free(produced);

			}
		}

	}

	return (void*)res;
}

bool isValidGrammar(Grammar grammar, const char* str) {
	atomic_bool found = false;
	return validateGrammar(grammar, grammar.start, str);
}

int main(void) {
	Grammar grammar;
	Rule rules[] = {
		{ 'S', "A" },
		{ 'A', "0A1" },
		{ 'A', "" },
	};
	grammar.ruleSet.rules = rules;
	grammar.ruleSet.count = 3;
	grammar.start = "S";

#define CHECK(g, str) \
	printf("\"%s\" -> %s\n", str, isValidGrammar((g), str) ? "VALID" : "INVALID")

	CHECK(grammar, "");
	CHECK(grammar, "01");
	CHECK(grammar, "001");
	CHECK(grammar, "011");
	CHECK(grammar, "0011");

}
